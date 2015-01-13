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

# build type
btype = os.environ.get('LX_BUILD')
if btype == 'debug':
    baseenv.Append(CXXFLAGS = ' -O0 -g')
    baseenv.Append(CFLAGS = ' -O0 -g')
else:
    baseenv.Append(CXXFLAGS = ' -O3 -DNDEBUG')
    baseenv.Append(CFLAGS = ' -O3 -DNDEBUG')
    btype = 'release'
builddir = 'build/' + btype

# print executed commands?
verbose = os.environ.get('LX_VERBOSE', 0)
if int(verbose) == 0:
    baseenv['CCCOMSTR'] = "CC $TARGET"
    baseenv['CXXCOMSTR'] = "CXX $TARGET"
    baseenv['LINKCOMSTR'] = "LD $TARGET"
    baseenv['ARCOMSTR'] = "AR $TARGET"

baseenv.Append(
    BINDIR = Dir(builddir + '/bin'),
    FSDIR = Dir('rootfs')
)

# create envs for xtensa-linux and host
env = baseenv.Clone()
ccprefix = 'buildroot/output/host/usr/bin/xtensa-linux'
env.Append(
    CPPPATH = ['#include'],
    LINKFLAGS = ' -static'
)
env.Replace(
    CXX = ccprefix + '-g++',
    CC = ccprefix + '-gcc'
)

# convenience functions
def LxProgram(env, target, source, libs = []):
    prog = env.Program(
        target, source,
        LIBS = libs
    )
    env.Install(env['BINDIR'], prog)
    env.Install(env['FSDIR'].abspath + '/bench', prog)
    return prog

env.LxProgram = LxProgram

# build everything
env.SConscript('apps/SConscript', 'env', variant_dir = builddir + '/apps', duplicate = 0)
