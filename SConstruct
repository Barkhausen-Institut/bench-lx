import os

# build basic environment
baseenv = Environment(
    CFLAGS = ' -std=c99 -Wall -Wextra',
    CXXFLAGS = ' -std=c++11 -Wall -Wextra',
    ENV = {
        'PATH' : os.environ['PATH'],
        # required for colored outputs
        'HOME' : os.environ['HOME'],
        'TERM' : os.environ['TERM'],
    }
)

# arch
arch = os.environ.get('LX_ARCH')

# build type
btype = os.environ.get('LX_BUILD')
if btype == 'debug':
    baseenv.Append(CXXFLAGS = ' -O0 -g')
    baseenv.Append(CFLAGS = ' -O0 -g')
else:
    baseenv.Append(CXXFLAGS = ' -O2 -DNDEBUG')
    baseenv.Append(CFLAGS = ' -O2 -DNDEBUG')
    btype = 'release'
builddir = 'build/' + arch + '-' + btype

# print executed commands?
verbose = os.environ.get('LX_VERBOSE', 0)
if int(verbose) == 0:
    baseenv['CCCOMSTR'] = "CC $TARGET"
    baseenv['CXXCOMSTR'] = "CXX $TARGET"
    baseenv['LINKCOMSTR'] = "LD $TARGET"
    baseenv['ARCOMSTR'] = "AR $TARGET"

baseenv.Append(
    BUILDDIR = Dir(builddir),
    BINDIR = Dir(builddir + '/bin'),
    FSDIR = Dir('benchfs'),
    ARCH = arch
)

# create envs for xtensa-linux and host
env = baseenv.Clone()
ccprefix = builddir + '/buildroot/host/usr/bin/' + arch + '-linux'
env.Append(
    CPPPATH = ['#include'],
    LINKFLAGS = ' -static'
)

if arch == 'xtensa':
    xtdir = os.environ.get('XTENSA_DIR', '')
    env.Append(
        CPPPATH = [
            xtdir + '/XtDevTools/install/tools/RE-2014.5-linux/XtensaTools/xtensa-elf/include',
        ],
        LIBPATH = [
            xtdir + '/XtDevTools/install/builds/RE-2014.5-linux/DE_233L/xtensa-elf/arch/lib/'
        ],
    )

env.Replace(
    CXX = ccprefix + '-g++',
    CC = ccprefix + '-gcc'
)

# convenience functions
def LxProgram(env, target, source, libs = []):
    prog = env.Program(
        target, source,
        LIBS = libs + ['m']
    )
    env.Install(env['BINDIR'], prog)
    env.Install(env['FSDIR'].abspath + '/bin', prog)
    return prog

env.LxProgram = LxProgram

# build everything
env.SConscript('apps/SConscript', 'env', variant_dir = builddir + '/apps', duplicate = 0)
