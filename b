#!/bin/sh
if [ ! -d "$XTENSA_DIR" ]; then
	echo "Please set XTENSA_DIR to the path of the xtensa directory" >&2
	exit 1
fi

if [ -f /proc/cpuinfo ]; then
	cpus=`cat /proc/cpuinfo | grep '^processor[[:space:]]*:' | wc -l`
else
	cpus=1
fi

libgcc=buildroot/output/host/usr/lib/gcc/xtensa-buildroot-linux-uclibc/4.8.3/libgcc.a
core=DC_D_233L

build=$LX_BUILD
if [ "$build" != "release" ] && [ "$build" != "debug" ]; then
	build=release
fi
builddir="build/$build"
mkdir -p $builddir

simflags=""
# ensure that a cache miss costs 30 cycles, as a global memory read of 32bytes on T3
if [ "$LX_THCMP" = 1 ]; then
	simflags=" --write_delay=17 --read_delay=17"
	echo "Configuring core to require 30 cycles per cache-miss." 1>&2
else
	echo "Configuring core to require 13 cycles per cache-miss." 1>&2
fi

cmd=$1
shift

# add PATH to xtensa tools and buildroot-toolchain
PATH=$XTENSA_DIR/XtDevTools/install/tools/RD-2011.2-linux/XtensaTools/bin/:$PATH
PATH=$(pwd)/buildroot/output/host/usr/bin:$PATH
export PATH

export CC=`readlink -f buildroot/output/host/usr/bin/xtensa-linux-gcc`

case $cmd in
	mkbr)
		if [ ! -f buildroot/.config ]; then
			cp configs/config-buildroot-iss buildroot/.config
		fi

		( cd buildroot && make -j$cpus $* )

		# we have to strip the debugging info here, because it's in gwarf-4 format but the xtensa gdb only
		# supports 2 (and is too stupid to just ignore them)
		if [ -f $libgcc ]; then
			xtensa-linux-objcopy -g $libgcc
		fi
		;;

	mklx)
		if [ ! -f $builddir/.config ]; then
			cp configs/config-linux-iss $builddir/.config
		fi

		# tell linux our cross-compiler prefix
		export CROSS_COMPILE=xtensa-linux-

		if [ "$build" = "debug" ]; then
			( cd linux && make O=../$builddir ARCH=xtensa KBUILD_CFLAGS="-O1 -gdwarf-2 -g" -j$cpus $* )
		else
			flags="-Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common"
			flags="$flags -Werror-implicit-function-declaration -Wno-format-security -fno-delete-null-pointer-checks"
			flags="$flags -O3 -ffreestanding -D__linux__ -pipe -mlongcalls -mforce-no-pic -Wframe-larger-than=1024"
			flags="$flags -fno-stack-protector -Wno-unused-but-set-variable -fomit-frame-pointer -g "
			flags="$flags -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fconserve-stack"
			flags="$flags -Werror=implicit-int -Werror=strict-prototypes  -DCC_HAVE_ASM_GOTO -gdwarf-2"
			echo $flags
			( cd linux && make O=../$builddir KBUILD_CFLAGS="$flags" ARCH=xtensa -j$cpus $* )
		fi
		;;

	mkapps)
		scons -j$cpus
		;;

	elf=*)
		xt-readelf -aW $builddir/bin/${cmd#elf=} | less
		;;

	dis=*)
		xt-objdump --xtensa-core=$core -SC $builddir/bin/${cmd#dis=} | less
		;;

	disp=*)
		export XTENSA_CORE=$core
		xt-objdump -dC $builddir/bin/${cmd#disp=} | \
			awk -v EXEC=$builddir/bin/${cmd#disp=} -f ./tools/pimpdisasm.awk | less
		;;

	run)
		xt-run --xtensa-core=$core --memlimit=128 --mem_model $simflags \
			$builddir/arch/xtensa/boot/Image.elf
		;;

	runturbo)
		xt-run --xtensa-core=$core --memlimit=128 --turbo \
			$builddir/arch/xtensa/boot/Image.elf
		;;

	bench)
		tmp=`mktemp`
		echo RUN_BENCHMARKS > $tmp

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

		xt-run --xtensa-core=$core --memlimit=128 --mem_model $simflags \
			--loadbin=$tmp@0x3000000 $builddir/arch/xtensa/boot/Image.elf > $res

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
		xt-run --xtensa-core=$core $args --memlimit=128 \
			--client_commands="trace --level 6 trace.txt" \
			$builddir/arch/xtensa/boot/Image.elf
		;;

	dbg|dbgturbo)
		cmds=`mktemp`
		if [ $cmd = "dbgturbo" ]; then
			echo "target sim --turbo $simflags --memlimit=128" > $cmds
		else
			echo "target sim --mem_model $simflags --memlimit=128" > $cmds
		fi
		echo "symbol-file $builddir/vmlinux" >> $cmds
		echo "display/i \$pc" >> $cmds

		xt-gdb --xtensa-core=$core $builddir/arch/xtensa/boot/Image.elf --command=$cmds
		rm $cmds
		;;

	*)
		echo "Usage: $0 (mkbr|mklx|mkapps|run|runturbo|bench|trace|dbg|dbgturbo)" >&2
		echo "  Use LX_BUILD to set the build-type (debug|release)." >&2
		echo "  Set LX_THCMP=1 to configure cache misses comparably." >&2
		;;
esac
