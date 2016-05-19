#!/bin/bash

if [ "$LX_ARCH" = "" ] || [ "$LX_ARCH" != "x86_64" ]; then
	echo "Defaulting to LX_ARCH=xtensa"
	LX_ARCH=xtensa
fi
export LX_ARCH

if [ "$LX_ARCH" = "xtensa" ]; then
	if [ ! -d "$XTENSA_DIR" ]; then
		echo "Please set XTENSA_DIR to the path of the xtensa directory" >&2
		exit 1
	fi
else
	if [ ! -d "$GEM5_DIR" ]; then
		echo "Please set GEM5_DIR to the path of the gem5 directory" >&2
		exit 1
	fi

    export M5_PATH=gem5
fi

if [ -f /proc/cpuinfo ]; then
	cpus=`cat /proc/cpuinfo | grep '^processor[[:space:]]*:' | wc -l`
else
	cpus=1
fi

build=$LX_BUILD
if [ "$build" != "release" ] && [ "$build" != "debug" ]; then
	build=release
fi
builddir="build/$LX_ARCH-$build"
mkdir -p $builddir $builddir/buildroot

if [ "$LX_ARCH" = "xtensa" ]; then
	libgcc=$builddir/buildroot/host/usr/lib/gcc/xtensa-buildroot-linux-uclibc/4.8.4/libgcc.a
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
fi

cmd=$1
shift

# add PATH to xtensa tools and buildroot-toolchain
if [ "$LX_ARCH" = "xtensa" ]; then
	PATH=$XTENSA_DIR/XtDevTools/install/tools/RE-2014.5-linux/XtensaTools/bin/:$PATH
fi
PATH=$(pwd)/$builddir/buildroot/host/usr/bin:$PATH
export PATH

export CC=`readlink -f $builddir/host/usr/bin/$LX_ARCH-linux-gcc`

extract_results() {
	awk '/>===.*/ {
		capture = 1
	}
	/<===.*/ {
		capture = 0
	}
	/^[^<>].*/ {
	    if(capture == 1)
	        print $0
	}'
}

run_gem5() {
	ln -sf `readlink -f $builddir/vmlinux` $M5_PATH/binaries/x86_64-vmlinux-2.6.22.9

	GEM5_CPU=${GEM5_CPU:-detailed}

	params=`mktemp`
	echo -n "--outdir=$M5_PATH --debug-file=gem5.log" >> $params
	if [ "$GEM5_FLAGS" != "" ]; then
		echo -n " --debug-flags=$GEM5_FLAGS" >> $params
	fi
	echo -n " $GEM5_DIR/configs/example/fs.py --cpu-type $GEM5_CPU" >> $params
	echo -n " --cpu-clock=1GHz --sys-clock=1GHz" >> $params
	echo -n " --caches --l2cache" >> $params
	echo -n " --command-line=\"ttyS0 noapictimer console=ttyS0 lpj=7999923 root=/dev/sda1\" " >> $params
	if [ "$GEM5_CP" != "" ]; then
		echo -n " --restore-with-cpu $GEM5_CPU --checkpoint-restore=$GEM5_CP" >> $params
	fi
	if [ "$cmd" = "warmupgem5" ]; then
		echo -n " --checkpoint-at-end" >> $params
	fi

	if [ "$cmd" = "rungem5" ]; then
        xargs -a $params $GEM5_DIR/build/X86/gem5.opt | tee $M5_PATH/log.txt
	else
		# start gem5 in background
		xargs -a $params $GEM5_DIR/build/X86/gem5.opt > $M5_PATH/log.txt 2>/dev/null &

		# wait until com1 port is open
		while [ "`lsof -i :3456`" == "" ]; do
			echo "Waiting for GEM5 to start..." 1>&2
			sleep 1
		done

		echo -n > $M5_PATH/res.tmp

		# now send the command via telnet to gem5
		(
			if [ "$cmd" = "warmupgem5" ]; then
				# login via bruteforce
				while true; do
					echo "root"
					sleep 1
				done
			elif [ "$cmd" = "fsbenchgem5" ]; then
				echo -e "\n/fsbench.sh '$FSBENCH_CMD'"
			else
				echo -e "\n/bench.sh"
			fi
			# sleep "for ever"
			sleep 100000
		) | telnet 127.0.0.1 3456 2>/dev/null > $M5_PATH/res.tmp &

		# wait until we see the "benchmarks done" in the log file
		while [ "`grep $1 $M5_PATH/res.tmp`" == "" ]; do
			sleep 1
		done

		killall -INT gem5.opt
		# kill all in our process group to stop subshells, too
		trap "kill -INT 0" TERM

		if [ "$cmd" = "warmupgem5" ]; then
			# wait until the checkpoint is written
			path=$M5_PATH/cpt.*/m5.cpt
			while [ "`grep 'isa=x86' $path`" = "" ]; do
				echo "Waiting for checkpoint..." 1>&2
				sleep 1
				path=$M5_PATH/cpt.*/m5.cpt
			done
		fi
	fi
}

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

		if [ "$LX_ARCH" = "xtensa" ] && [ "$build" = "debug" ]; then
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

	mkdisk)
		if [ "$LX_ARCH" = "x86_64" ]; then
			# create swap disk
			dd if=/dev/zero of=$M5_PATH/disks/linux-bigswap2.img count=1024

			# create disk for root fs
			rm -f $M5_PATH/disks/x86root.img
			$GEM5_DIR/util/gem5img.py init $M5_PATH/disks/x86root.img 64
			tmp=`mktemp -d`
			$GEM5_DIR/util/gem5img.py mount $M5_PATH/disks/x86root.img $tmp
			cpioimg=`readlink -f $builddir/buildroot/images/rootfs.cpio`
			( cd $tmp && sudo cpio -id < $cpioimg )
			$GEM5_DIR/util/gem5img.py umount $tmp
			rmdir $tmp
		else
			echo "Unsupported"
		fi
		;;

	run)
		cd $builddir && xt-run --memlimit=128 --mem_model $simflags \
			arch/xtensa/boot/Image.elf
		;;

	runturbo)
		cd $builddir && xt-run --memlimit=128 --mem_model $simflags --turbo \
			arch/xtensa/boot/Image.elf
		;;

	rungem5)
		if [ "$LX_ARCH" = "x86_64" ]; then
			run_gem5
		else
			echo "Unsupported"
		fi
		;;

	runqemu)
		if [ "$LX_ARCH" = "x86_64" ]; then
			qemu-system-x86_64 -enable-kvm -serial stdio -hda $M5_PATH/disks/x86root.img \
			    -kernel $builddir/arch/x86/boot/bzImage \
			    -append "console=ttyS0 root=/dev/sda1"
		else
			echo "Unsupported"
		fi
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

		extract_results < $res
		rm $res $tmp
		;;

	benchgem5|fsbenchgem5)
		run_gem5 '^<===Benchmarks_done==='
        grep -v '^#' $M5_PATH/res.tmp | extract_results
        rm $M5_PATH/res.tmp
		;;

	warmupgem5)
		run_gem5 '^#'
        rm $M5_PATH/res.tmp
		;;

	trace)
		args="--mem_model $simflags"
		cd $builddir && xt-run $args --memlimit=128 \
			--client_commands="trace --level 0 trace.txt" \
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

	dbgqemu)
		if [ "$LX_ARCH" = "x86_64" ]; then
			qemu-system-x86_64 -serial stdio \
			    -hda $M5_PATH/disks/x86root.img \
			    -kernel $builddir/arch/x86/boot/bzImage \
			    -append "console=ttyS0 root=/dev/sda1" -S -s &

			cmd=`mktemp`
			echo "target remote localhost:1234" > $cmd
			echo 'display/i $pc' >> $cmd
			gdb --tui $builddir/vmlinux -command=$cmd
			rm $cmd
		else
			echo "Unsupported"
		fi
		;;

	gem5serial)
		while true; do
			echo "Waiting for port 3456.. to be open..."
			while true; do
				popen=0
				port=3456
				while [ $port -lt 3458 ]; do
					if [ "`lsof -i :$port`" != "" ]; then
						popen=1
						break
					fi
					port=$((port + 1))
				done
				if [ $popen -ne 0 ]; then
					break
				fi
				sleep 1
			done

			echo "Connecting to port $port..."
			telnet localhost $port
		done
		;;

	*)
		echo -n "Usage: $0 (mkbr|mklx|mkapps|elf|dis|disp|mkdisk|run|runturbo|rungem5|" >&2
		echo -n "runqemu|bench|fsbench|benchgem5|fsbenchgem5|warmupgem5|trace|dbg|dbgturbo|" >&2
		echo "dbgqemu|gem5serial)" >&2
		echo "  Use LX_ARCH to set the architecture to build for (xtensa|x86_64)." >&2
		echo "  Use LX_BUILD to set the build-type (debug|release)." >&2
		echo "  Set LX_THCMP=1 to configure cache misses comparably." >&2
		;;
esac
