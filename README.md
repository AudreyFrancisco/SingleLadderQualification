# Framework for ALPIDE testing

[![pipeline status](https://gitlab.cern.ch/alice-its-alpide-software/new-alpide-software/badges/master/pipeline.svg)](https://gitlab.cern.ch/alice-its-alpide-software/new-alpide-software/commits/master)

## Build instructions

### Initial build

First, clone the project:
```
git clone https://gitlab.cern.ch/alice-its-alpide-software/new-alpide-software
```
Then, create and change to a build directory (it can but doesn't have
to be a sub-directory of the project source directory):
```
mkdir build
cd build
```
Prepare the build by running cmake (at least version 3.2, i.e. you
need to install the package cmake3 on CC7) in the build directory:
```
cmake <source directory>
```
and build the project with:
```
make -j $(nproc)
```
Now you can run the executables from the build directory.

### Updating the code
You can update the code with the latest version from the repository with:
```
git pull --rebase
```

On updates (from the repository or by your own changes) it suffices to
rerun `make` in the build directory:
```
cd build
make -j $(nproc)
```

## Advanced build options

### Installation

If you don't want to install to the system-wide default location
(/usr/local) you should (once) set a prefix directory for the
installation:
```
cd build
cmake -DCMAKE_INSTALL_PREFIX=<prefix>
```
Then, you can install the software by running `make install` in the
build directory:
```
make -j $(nproc) install
```

### Packaging

In order to build an rpm package, run `cpack` in the build directory:
```
cd build
cpack
```
