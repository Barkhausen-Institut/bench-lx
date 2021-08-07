#!/bin/bash

if [ -z $LX_ARCH ]; then
	LX_ARCH=x86_64
	echo "Defaulting to LX_ARCH=$LX_ARCH"
fi

if [ -z $LX_PLATFORM ]; then
	LX_PLATFORM=qemu
	echo "Defaulting to LX_PLATFORM=$LX_PLATFORM"
fi

if [ "$LX_PLATFORM" = "gem5" ] && [ ! -d "$GEM5_DIR" ]; then
	echo "Please set GEM5_DIR to the path of the gem5 directory" >&2
	exit 1
fi

LX_CORES=${LX_CORES:-1}
LX_BUILDDIR="build/$LX_ARCH"
export LX_ARCH LX_PLATFORM LX_BUILDDIR LX_CORES

mkdir -p $LX_BUILDDIR/{linux,buildroot,riscv-pk,disks}

cmd=$1
shift

if [ ! -f build/ignoreint ]; then
	g++ -Wall -Wextra -O2 -o build/ignoreint tools/ignoreint.cc
fi

if [ "$cmd" != "mkqemu" ]; then
	export PATH=$(pwd)/$LX_BUILDDIR/buildroot/host/usr/bin:$PATH
fi

export CC=`readlink -f $LX_BUILDDIR/host/usr/bin/$LX_ARCH-linux-gcc`
export CROSS_COMPILE=$LX_ARCH-linux-

case $cmd in
	warmup|run|dbg|bench|fsbench|serverbench|servertrace)
		./platforms/$LX_PLATFORM $cmd
		;;

	mkbr)
		if [ ! -f $LX_BUILDDIR/buildroot/.config ]; then
			cp configs/config-buildroot-$LX_ARCH $LX_BUILDDIR/buildroot/.config
		fi

		( cd buildroot && make O=../$LX_BUILDDIR/buildroot -j$(nproc) $* )
		if [ $? -ne 0 ]; then
			exit 1
		fi

		if [ "$LX_PLATFORM" = "gem5" ]; then
			# create disk for root fs
			rm -f $LX_BUILDDIR/disks/root.img
			$GEM5_DIR/util/gem5img.py init $LX_BUILDDIR/disks/root.img 128
			tmp=`mktemp -d`
			$GEM5_DIR/util/gem5img.py mount $LX_BUILDDIR/disks/root.img $tmp
			cpioimg=`readlink -f $LX_BUILDDIR/buildroot/images/rootfs.cpio`
			( cd $tmp && sudo cpio -id < $cpioimg )
			$GEM5_DIR/util/gem5img.py umount $tmp
			rmdir $tmp
		fi
		;&	# fall through; build benchfs as well

	mkbenchfs)
		if [ "$LX_PLATFORM" = "gem5" ]; then
			# create disk for bench fs
			rm -f $LX_BUILDDIR/disks/bench.img
			$GEM5_DIR/util/gem5img.py init $LX_BUILDDIR/disks/bench.img 128
			tmp=`mktemp -d`
			$GEM5_DIR/util/gem5img.py mount $LX_BUILDDIR/disks/bench.img $tmp
			sudo cp -r rootfs/bench/* $tmp
			$GEM5_DIR/util/gem5img.py umount $tmp
			rmdir $tmp
		fi
		;;

	mklx)
		if [ ! -f $LX_BUILDDIR/linux/.config ]; then
			cp configs/config-linux-$LX_ARCH $LX_BUILDDIR/linux/.config
		fi

		if [ "$LX_ARCH" = "riscv64" ]; then
			export ARCH=riscv
		else
			export ARCH=$LX_ARCH
		fi

		( cd linux && make O=../$LX_BUILDDIR/linux -j$(nproc) $* )
		if [ $? -ne 0 ]; then
			exit 1
		fi
		;&	# fall through; bbl includes Linux

	mkbbl)
		if [ "$LX_ARCH" = "riscv64" ]; then
			export RISCV=$(pwd)/$LX_BUILDDIR/buildroot/host
			case $LX_PLATFORM in
				qemu) args="--with-mem-start=0x80000000" ;;
				gem5) args="--with-mem-start=0x80000000" ;;
				hw)   args="--with-mem-start=0x10000000 --with-dts=../../../../configs/hw.dts" ;;
			esac
			mkdir -p $LX_BUILDDIR/riscv-pk/$LX_PLATFORM
			(
				cd $LX_BUILDDIR/riscv-pk/$LX_PLATFORM \
					&& ../../../../riscv-pk/configure \
						--host=${CROSS_COMPILE::-1} \
						--with-payload=../../linux/vmlinux \
						$args \
					&& CFLAGS=" -D__riscv_compressed=1" make -j$(nproc)
			)
		fi
		;;

	mkqemu)
		mkdir -p build/qemu
		cd build/qemu
		../../qemu/configure \
			--target-list=riscv64-softmmu,x86_64-softmmu \
			--enable-trace-backends=simple \
			&& make -j$(nproc)
		;;

	mkapps)
		scons -j$(nproc) || exit 1

		# build ycsbclient for host
		export CARGO_TARGET_DIR=`readlink -f build/rust`
		export XBUILD_SYSROOT_PATH=$CARGO_TARGET_DIR/sysroot
		cargo build --release --color=always || exit 1

		# in case we want to build it for riscv:
		# export RUST_TARGET=riscv64gc-unknown-linux-gnu
		# export RUST_TARGET_PATH=`readlink -f tools`
		# export CARGO_TARGET_DIR=`readlink -f build/rust`
		# export XBUILD_SYSROOT_PATH=$CARGO_TARGET_DIR/sysroot
		# cargo build --target $RUST_TARGET --release --color=always || exit 1
		;;

	elf=*)
		${CROSS_COMPILE}readelf -aW $LX_BUILDDIR/bin/${cmd#elf=} | less
		;;

	dis=*)
		${CROSS_COMPILE}objdump -dC $LX_BUILDDIR/bin/${cmd#dis=} | less
		;;

	*)
		echo "Usage: $0 <cmd>" >&2
		echo ""
		echo "The following commands are supported:" >&2
		echo "  mkbr [<arg>..]:     make buildroot (and disk for x86_64)" >&2
		echo "  mklx [<arg>..]:     make linux" >&2
		echo "  mkbbl:              make RISC-V bootloader" >&2
		echo "  mkqemu:             make QEMU" >&2
		echo "  mkapps:             make applications" >&2
		echo "  elf:                show ELF information" >&2
		echo "  dis:                disassembly" >&2
		echo "  warmup:             boot linux and checkpoint it" >&2
		echo "  run:                run it" >&2
		echo "  dbg:                debug linux" >&2
		echo "  bench:              run benchmarks" >&2
		echo "  fsbench:            run FS benchmarks" >&2
		echo ""
		echo "Use LX_ARCH to set the architecture to build for (riscv64|x86_64)." >&2
		echo "Use LX_PLATFORM to set the platform to run linux on (gem5|qemu|hw)." >&2
		echo "Use LX_FLAGS to specify additional flags to the simulator" >&2
		;;
esac
