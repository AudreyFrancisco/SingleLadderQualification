# Framework for ALPIDE testing

Here, you find the general instructions for building and using the
software. For instructions on the migration to cmake refer to
[CMakeMigration.md](CMakeMigration.md), instead.

## Build instructions

### Prerequisites
In order to build and use the software the following requirements must be installed
(CC7 packages names):
- make
- cmake3
- tar
- zlib
- wget
- subversion
- git
- libusb1-devel
- tinyxml-devel
- qt5-qtbase-devel
- krb5-workstation
- cern-get-sso-cookie

In order to use clang-format and a modern gcc compiler, please also install:
```
yum install -y centos-release-scl && yum update -y && yum install -y llvm-toolset-7 devtoolset-7
```
Add the following lines to the ``.bashrc``:
```
source scl_source enable llvm-toolset-7
source scl_source enable devtoolset-7
```

In addition you should have working installations of
- ROOT (for ROOT-based analyses)
- Qt5 (for GUIs)

While the software can be built without these components, this results in
limited functionality and is discouraged.

Please do not install ROOT using the package manager (``yum``), but rather
download the binaries from https://root.cern.ch/ eg. like
```
wget -O /opt/root.tar.gz \
  https://root.cern.ch/download/root_v6.14.00.Linux-centos7-x86_64-gcc4.8.tar.gz
cd /opt/ ; tar xzfv root.tar.bz
```
Please make sure that you pick the binaries which match your operating system.
ROOT has to be loaded using ``source /opt/root/bin/thisroot.sh``.

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

## Using the software
It is recommended (at least for now) to run the software from the main build
directory. You do not need to source any environment script (i.e. no more
`source env.sh` for the GUI). Unless configured differently by environment
variables (see below), the executables expect to find the configuration
files in the current working directory and will write the output to Data/
in this directory.

### Environment variables (experimental)
You can configure the software by setting the following environment variables:
- `ALPIDE_TEST_CONFIG`: If set config files will be looked for in this directory. If
unset config files will be searched for in the current working directory (i.e.
the directory from which the executable is started).
- `ALPIDE_TEST_DATA`: If set the data files will be written to this directory. If
unset data files will be written to Data/ in the current working directory
instead.

This might not yet work for all tools, please report unintended behaviour.

## Advanced build options (not for general use)

### Configure the build
Even though not recommended, you can disable parts of the software. To do so
switch on the respective option using cmake, i.e.:
```
cmake -D<option>="ON"
```
The available options are:
- DISABLE\_QT: Do not build Qt-based components (i.e. no GUI).
- DISABLE\_ROOT: Do not build ROOT-based analyses.

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
