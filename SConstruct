import os

# build basic environment
baseenv = Environment(
    CFLAGS = ['-std=c99', '-Wall', '-Wextra'],
    CXXFLAGS = ['-std=c++11', '-Wall', '-Wextra'],
    ENV = {
        'PATH' : os.environ['PATH'],
        # required for colored outputs
        'HOME' : os.environ['HOME'],
        'TERM' : os.environ['TERM'],
    }
)

# arch
arch = os.environ.get('LX_ARCH')

baseenv.Append(CXXFLAGS = ['-O2', '-DNDEBUG', '-g'])
baseenv.Append(CFLAGS = ['-O2', '-DNDEBUG', '-g'])
builddir = 'build/' + arch

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
    FSDIR = Dir('rootfs/bench'),
    ARCH = arch
)

# create envs for xtensa-linux and host
env = baseenv.Clone()
ccprefix = builddir + '/buildroot/host/usr/bin/' + arch + '-linux'
env.Append(
    CPPPATH = ['#include'],
    LIBPATH = [env['BINDIR']],
    LINKFLAGS = ['-static']
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

def LxLibrary(env, target, source):
    lib = env.StaticLibrary(target, source)
    env.Install(env['BINDIR'], lib)
    return lib

env.LxProgram = LxProgram
env.LxLibrary = LxLibrary

# build everything
env.SConscript('apps/SConscript', 'env', variant_dir = builddir + '/apps', duplicate = 0)
env.SConscript('libs/SConscript', 'env', variant_dir = builddir + '/libs', duplicate = 0)
