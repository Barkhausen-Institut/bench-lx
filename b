#!/bin/bash

if [ "$LX_ARCH" = "" ]; then
	echo "Defaulting to LX_ARCH=xtensa LX_PLATFORM=xtensa"
	LX_ARCH=xtensa
	LX_PLATFORM=xtensa
fi

if [ "$LX_PLATFORM" = "" ]; then
	echo "Please specify the platform to run on (gem5|qemu)" >&2
	exit 1
fi

if [ -f /proc/cpuinfo ]; then
	cpus=`cat /proc/cpuinfo | grep '^processor[[:space:]]*:' | wc -l`
else
	cpus=1
fi

LX_CORES=${LX_CORES:-1}

if [ "$LX_BUILD" != "release" ] && [ "$LX_BUILD" != "debug" ]; then
	LX_BUILD=release
fi
LX_BUILDDIR="build/$LX_ARCH-$LX_BUILD"
mkdir -p $LX_BUILDDIR $LX_BUILDDIR/buildroot

export LX_BUILD LX_ARCH LX_PLATFORM LX_BUILDDIR LX_CORES

cmd=$1
shift

if [ "$LX_ARCH" = "xtensa" ]; then
	if [ ! -d "$XTENSA_DIR" ]; then
		echo "Please set XTENSA_DIR to the path of the xtensa directory" >&2
		exit 1
	fi
	libgcc=$builddir/buildroot/host/usr/lib/gcc/xtensa-buildroot-linux-uclibc/4.8.4/libgcc.a
	# add PATH to xtensa tools and buildroot-toolchain
	PATH=$XTENSA_DIR/XtDevTools/install/tools/RE-2014.5-linux/XtensaTools/bin/:$PATH
else
	if [ ! -d "$GEM5_DIR" ]; then
		echo "Please set GEM5_DIR to the path of the gem5 directory" >&2
		exit 1
	fi
fi
PATH=$(pwd)/$LX_BUILDDIR/buildroot/host/usr/bin:$PATH
export PATH

export CC=`readlink -f $LX_BUILDDIR/host/usr/bin/$LX_ARCH-linux-gcc`

mkdir -p $LX_BUILDDIR/disks

case $cmd in
	warmup|run|dbg|bench|fsbench|trace|serverbench|servertrace)
		./platforms/$LX_PLATFORM $cmd
		;;

	mkbr)
		if [ ! -f $LX_BUILDDIR/buildroot/.config ]; then
			cp configs/config-buildroot-$LX_ARCH $LX_BUILDDIR/buildroot/.config
		fi

		( cd buildroot && make O=../$LX_BUILDDIR/buildroot -j$cpus $* )

		# we have to strip the debugging info here, because it's in gwarf-4 format but the xtensa gdb only
		# supports 2 (and is too stupid to just ignore them)
		if [ "$LX_ARCH" = "xtensa" ] && [ -f $libgcc ]; then
			xtensa-linux-objcopy -g $libgcc
		fi

		if [ "$LX_ARCH" = "x86_64" ]; then
			# create disk for root fs
			rm -f $LX_BUILDDIR/disks/x86root.img
			$GEM5_DIR/util/gem5img.py init $LX_BUILDDIR/disks/x86root.img 16
			tmp=`mktemp -d`
			$GEM5_DIR/util/gem5img.py mount $LX_BUILDDIR/disks/x86root.img $tmp
			cpioimg=`readlink -f $LX_BUILDDIR/buildroot/images/rootfs.cpio`
			( cd $tmp && sudo cpio -id < $cpioimg )
			$GEM5_DIR/util/gem5img.py umount $tmp
			rmdir $tmp

			# create disk for bench fs
			rm -f $LX_BUILDDIR/disks/linux-bigswap2.img
			$GEM5_DIR/util/gem5img.py init $LX_BUILDDIR/disks/linux-bigswap2.img 128
			tmp=`mktemp -d`
			$GEM5_DIR/util/gem5img.py mount $LX_BUILDDIR/disks/linux-bigswap2.img $tmp
			cpioimg=`readlink -f $LX_BUILDDIR/buildroot/images/rootfs.cpio`
			sudo cp -r benchfs/* $tmp
			$GEM5_DIR/util/gem5img.py umount $tmp
			rmdir $tmp
		fi
		;;

	mkbenchfs)
		# create disk for bench fs
		rm -f $LX_BUILDDIR/disks/linux-bigswap2.img
		$GEM5_DIR/util/gem5img.py init $LX_BUILDDIR/disks/linux-bigswap2.img 128
		tmp=`mktemp -d`
		$GEM5_DIR/util/gem5img.py mount $LX_BUILDDIR/disks/linux-bigswap2.img $tmp
		cpioimg=`readlink -f $LX_BUILDDIR/buildroot/images/rootfs.cpio`
		sudo cp -r benchfs/* $tmp
		$GEM5_DIR/util/gem5img.py umount $tmp
		rmdir $tmp
		;;

	mklx)
		if [ ! -f $LX_BUILDDIR/.config ]; then
			cp configs/config-linux-$LX_ARCH $LX_BUILDDIR/.config
		fi

		# tell linux our cross-compiler prefix
		export CROSS_COMPILE=$LX_ARCH-linux-

		if [ "$LX_ARCH" = "xtensa" ] && [ "$build" = "debug" ]; then
			( cd linux && make O=../$LX_BUILDDIR ARCH=$LX_ARCH KBUILD_CFLAGS="-O1 -gdwarf-2 -g" -j$cpus $* )
		else
			( cd linux && make O=../$LX_BUILDDIR ARCH=$LX_ARCH -j$cpus $* )
		fi
		;;

	mkapps)
		scons -j$cpus
		;;

	elf=*)
		readelf -aW $LX_BUILDDIR/bin/${cmd#elf=} | less
		;;

	dis=*)
		if [ "$LX_ARCH" = "xtensa" ]; then
			xt-objdump -SC $LX_BUILDDIR/bin/${cmd#dis=} | less
		else
			objdump -SC $LX_BUILDDIR/bin/${cmd#dis=} | less
		fi
		;;

	disp=*)
		xt-objdump -dC $LX_BUILDDIR/bin/${cmd#disp=} | \
			awk -v EXEC=$LX_BUILDDIR/bin/${cmd#disp=} -f ./tools/pimpdisasm.awk | less
		;;

	*)
		echo "Usage: $0 <cmd>" >&2
		echo ""
		echo "The following commands are supported:" >&2
		echo "  mklx:       make linux" >&2
		echo "  mkapps:     make applications" >&2
		echo "  mkbr:       make buildroot (and disk for x86_64)" >&2
		echo "  elf:        show ELF information" >&2
		echo "  dis:        disassembly" >&2
		echo "  disp:       enhanced disassembly" >&2
		echo "  warmup:     boot linux and checkpoint it" >&2
		echo "  run:        run it" >&2
		echo "  dbg:        debug linux" >&2
		echo "  bench:      run benchmarks" >&2
		echo "  fsbench:    run FS benchmarks" >&2
		echo "  trace:      create instruction trace" >&2
		echo ""
		echo "Use LX_ARCH to set the architecture to build for (xtensa|x86_64)." >&2
		echo "Use LX_PLATFORM to set the platform to run linux on (xtensa|gem5|qemu)." >&2
		echo "Use LX_BUILD to set the build-type (debug|release)." >&2
		echo "Use LX_FLAGS to specify additional flags to the simulator" >&2
		echo "Set LX_THCMP=1 to configure cache misses comparably." >&2
		;;
esac
