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

build=$LX_BUILD
if [ "$build" != "release" ] && [ "$build" != "debug" ]; then
	build=release
fi
builddir="build/$build"
mkdir -p $builddir

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
			( cd linux && make O=../$builddir ARCH=xtensa -j$cpus $* )
		fi
		;;

	mkapps)
		make -C apps $*
		;;

	run|runbench)
		cmds=`mktemp`
		if [ $cmd = "runbench" ]; then
			echo "target sim --mem_model --memlimit=128" > $cmds
		else
			echo "target sim --turbo --memlimit=128" > $cmds
		fi
		echo "symbol-file $builddir/vmlinux" >> $cmds
		echo "display/i \$pc" >> $cmds

		xt-gdb --xtensa-core=DC_D_233L $builddir/arch/xtensa/boot/Image.elf --command=$cmds
		rm $cmds
		;;

	*)
		echo "Usage: $0 (mkbr|mklx|mkapps|run|runbench)" >&2
		echo "  Use LX_BUILD to set the build-type (debug|release)." >&2
		;;
esac

