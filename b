#!/bin/bash
if [ ! -d "$XTENSA_DIR" ]; then
	echo "Please set XTENSA_DIR to the path of the xtensa directory" >&2
	exit 1
fi

if [ -f /proc/cpuinfo ]; then
	cpus=`cat /proc/cpuinfo | grep '^processor[[:space:]]*:' | wc -l`
else
	cpus=1
fi

if [ "$LX_ARCH" = "" ]; then
	LX_ARCH=xtensa
fi
export LX_ARCH

build=$LX_BUILD
if [ "$build" != "release" ] && [ "$build" != "debug" ]; then
	build=release
fi
builddir="build/$LX_ARCH-$build"
mkdir -p $builddir $builddir/buildroot

libgcc=$builddir/host/usr/lib/gcc/xtensa-buildroot-linux-uclibc/4.8.4/libgcc.a
export XTENSA_CORE=DE_233L

simflags=""
# ensure that a cache miss costs 30 cycles, as a global memory read of 32bytes on T3
if [ "$LX_THCMP" = 1 ]; then
	simflags=" --write_delay=17 --read_delay=17"
	echo "Configuring core to require 30 cycles per cache-miss." 1>&2
else
	echo "Configuring core to require 13 cycles per cache-miss." 1>&2
fi
simflags="$simflags --dcsize=$((64*1024)) --dcline=32 --icsize=$((64*1024)) --icline=32"

cmd=$1
shift

# add PATH to xtensa tools and buildroot-toolchain
if [ "$LX_ARCH" = "xtensa" ]; then
	PATH=$XTENSA_DIR/XtDevTools/install/tools/RE-2014.5-linux/XtensaTools/bin/:$PATH
fi
PATH=$(pwd)/$builddir/buildroot/host/usr/bin:$PATH
export PATH

export CC=`readlink -f $builddir/host/usr/bin/$LX_ARCH-linux-gcc`

case $cmd in
	mkbr)
		if [ ! -f $builddir/buildroot/.config ]; then
			cp configs/config-buildroot-$LX_ARCH $builddir/buildroot/.config
		fi

		( cd buildroot && make O=../$builddir/buildroot -j$cpus $* )

		# we have to strip the debugging info here, because it's in gwarf-4 format but the xtensa gdb only
		# supports 2 (and is too stupid to just ignore them)
		if [ "$LX_ARCH" = "xtensa" ] && [ -f $libgcc ]; then
			xtensa-linux-objcopy -g $libgcc
		fi
		;;

	mklx)
		if [ ! -f $builddir/.config ]; then
			cp configs/config-linux-$LX_ARCH $builddir/.config
		fi

		# tell linux our cross-compiler prefix
		export CROSS_COMPILE=$LX_ARCH-linux-

		if [ "$build" = "debug" ]; then
			( cd linux && make O=../$builddir ARCH=$LX_ARCH KBUILD_CFLAGS="-O1 -gdwarf-2 -g" -j$cpus $* )
		else
			( cd linux && make O=../$builddir ARCH=$LX_ARCH -j$cpus $* )
		fi
		;;

	mkapps)
		scons -j$cpus
		;;

	elf=*)
		readelf -aW $builddir/bin/${cmd#elf=} | less
		;;

	dis=*)
		if [ "$LX_ARCH" = "xtensa" ]; then
			xt-objdump -SC $builddir/bin/${cmd#dis=} | less
		else
			objdump -SC $builddir/bin/${cmd#dis=} | less
		fi
		;;

	disp=*)
		xt-objdump -dC $builddir/bin/${cmd#disp=} | \
			awk -v EXEC=$builddir/bin/${cmd#disp=} -f ./tools/pimpdisasm.awk | less
		;;

	run)
		cd $builddir && xt-run --memlimit=128 --mem_model $simflags \
			arch/xtensa/boot/Image.elf
		;;

	runturbo)
		cd $builddir && xt-run --memlimit=128 --mem_model $simflags --turbo \
			arch/xtensa/boot/Image.elf
		;;

	bench|fsbench)
		tmp=`mktemp`
		if [ "$cmd" = "bench" ]; then
			echo RUN_BENCH > $tmp
		else
			echo -e -n "CMD=$FSBENCH_CMD\0" > $tmp
		fi

		# we can't run xt-run in background for some reason, which is why we run the loop that
		# waits for the benchmark to finish in background.
		res=`mktemp`
		(
			while [ "`grep '<=== Benchmarks done ===' $res 2>/dev/null`" = "" ]; do
				# if the simulator exited for some reason and thus deleted the file, stop here
				if [ ! -f $res ]; then
					exit 1
				fi
				sleep 1
			done

			# kill the process that uses the temp-file. should be just the iss started here
			# not killing all 'iss' instances allows us to run multiple benchmarks in parallel
			kill `lsof $res | grep '^iss' -m 1 | awk -e '{ print $2 }'`
		) &

		(
			cd $builddir && xt-run --memlimit=128 --mem_model $simflags \
			--loadbin=$tmp@0x3000000 arch/xtensa/boot/Image.elf > $res
		)

		# extract benchmark result
		awk '/>===.*/ {
			capture = 1
		}
		/<===.*/ {
			capture = 0
		}
		/^[^<>].*/ {
		    if(capture == 1)
		        print $0
		}' $res

		rm $res $tmp
		;;

	trace)
		args="--mem_model $simflags"
		cd $builddir && xt-run $args --memlimit=128 \
			--client_commands="trace --level 6 trace.txt" \
			arch/xtensa/boot/Image.elf
		;;

	dbg|dbgturbo)
		cmds=`mktemp`
		if [ $cmd = "dbgturbo" ]; then
			echo "target sim --mem_model $simflags --memlimit=128 --turbo" > $cmds
		else
			echo "target sim --mem_model $simflags --memlimit=128" > $cmds
		fi
		echo "symbol-file vmlinux" >> $cmds
		echo "display/i \$pc" >> $cmds

		cd $builddir && xt-gdb --tui arch/xtensa/boot/Image.elf --command=$cmds
		rm $cmds
		;;

	*)
		echo "Usage: $0 (mkbr|mklx|mkapps|run|runturbo|bench|fsbench|trace|dbg|dbgturbo)" >&2
		echo "  Use LX_BUILD to set the build-type (debug|release)." >&2
		echo "  Set LX_THCMP=1 to configure cache misses comparably." >&2
		;;
esac
