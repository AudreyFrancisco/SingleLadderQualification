# Migration to cmake build

The following instructions assume that you have a setup working with
the (now deprecated) Makefile-based build and lists the steps
recommended for the migration to cmake. For an initial installation or
further documentation refer to the general [README](README.md),
please.

## preparations

You need to install cmake (at least in version 3.2). In order to have
full support for C++11, you have to install a sufficiently modern C++
compiler (gcc 4.8.5 as default in CC7 is insufficient). For format
checking you should have the proper version of clang-format installed:
```
yum install -y centos-release-scl && yum update -y && yum install -y cmake3 llvm-toolset-7 devtoolset-7
```
and add the following lines to `~/.bashrc`:
```
source scl_source enable llvm-toolset-7
source scl_source enable devtoolset-7
```
For the following instructions, either open a new shell or re-source
`.bashrc`:
```
source ~/.bashrc
```

Update your version of the software by executing (in the project
directory):
```
git pull
```

Remove any remainders from the old build (again in the project dir):
```
make clean-all
```

## build

For the first cmake-based build do:
```
mkdir build
cd build
cmake ..
```

For the actual build (and all subsequent rebuilds upon updates) it
suffices to run make in the build directory:
```
make -j $(nproc)
```

## using the new build

To use this build you do not need to source any environment script but
you can directly execute the binaries from the build directory. In
order to continue using the same Data directory as before create a
symlink to the previous Data directory, i.e. in the build directory:
```
ln -s ../Data .
```
and execute all the binaries from the build directory, e.g. for the
GUI:
```
GUI/GUI
```