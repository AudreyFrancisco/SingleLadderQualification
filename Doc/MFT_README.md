# MFT branch of new-alpide-software

## Getting started

Clone the repository (master branch called mft-master) :

```
$ git clone https://gitlab.cern.ch/alice-MFT-ladder-test/new-alpide-software.git

```

## Prerequisites ##

### Tested Setups ###

Make sure you have a recent versions of compiler, qmake (for GUI users), and ROOT ($ROOTSYS shell env variable pointing to your working ROOT distribution).
For Linux distributions and detailed informations about setup and installation, you can refer to the Doc/UserManual.pdf document.

#### MacOS (High Sierra 10.13.3)

- Apple LLVM version 9.0.0 (clang-900.0.39.2)
- Xcode 9.2
- ROOT 6.12/04
- QMake version 3.1 using Qt 5.10.0 (GUI)

#### Ubuntu (16.04 LTS)

- gcc version 5.4.0
- ROOT 6.12/04
- QMake version 3.0 using Qt 5.5.1 (GUI)

--> The software will be tested on CentOS 7 (Cern CentOS 7).

## Installation ##

### Software ##

In this section again, see Doc/UserManual.pdf for detailed informations. To install the software, run the following commands :

```
$ cd new-alpide-software
$ make

```
When the compilation is successfully done, you can follow the instructions reported in the next section to run the tests.

### Running the tests ###

The tests that are now available (tested) for MFT HICs are (this list is not exhaustive):
- FIFO scan (main_fifo.cpp)
- Digital scan (main_digitalscan.cpp)
- Threshold scan (main_threshold.cpp)
- Noise occupancy (main_noiseocc.cpp)
- ...

For now, each test has a corresponding configuration file specifying the type of the tested device and some configurable parameters for the Readout Board and the chips. To run a test, the structure of the command (for MFT HICs) is the following :

```
$ ./test_* -c Config/Config_MFTLadder_*.cfg

```
for example, to run a threshold scan, the command line is :

```
$ ./test_threshold -c Config/Config_MFTLadder_ThresholdScan.cfg

```
### Analysis macros ###

Each test also has a corresponding analysis macro to analyze and visualize the test results stored in .dat files in the Data/. The macros and the command
lines to launch them are documented in the UserManual.pdf. In the case of a threshold scan, run FitThresholds.C to fit the obtained S-curves and visualize the threshold distribution and associated noise :

```
$ cd analysis/
$ root -l
root[0] .x FitThresholds.C+ ("../Data/ThresholdScan_xxxx_xxxx_ChipX_0.dat")

```

To get the hit map for a Digital scan, the macro is called Hitmap.C and in the case of a noise occupancy scan, the hit maps and the fake hit rate histos for each chip are generated by NoiseOccupancyRawToHisto.C. Concerning the NoiseOccupancyRawToHisto macro, the number of triggers and the DAC parameters have to be manually tuned at the beginning of the macro). This will be adjusted in a future version.

### GUI ###

The Graphical (User) Interface aims to communicate with the database to store the results of the tests for each HIC. This part of the soft is being modified and developed to make it compatible with the future MFT database. For now, it is linked to the ITS database and a specific access is required. Despite of this, the GUI can be compiled and launched if you just want to see the GUI window and his test menu.

#### Ubuntu

Under Ubuntu distribution, makefile modifications are not needed to compile and launch the GUI following these steps :

```
$ cd new-alpide-software/
$ make lib && make lib_analysis
(or  $ make && make lib && make lib_analysis if the software is not already compiled)

$ cd GUI/
$ qmake -makefile
$ make

$ source ./env.sh
$ ./GUI

```
This should be able to launch the GUI if your setup is at least the one described in the section "Tested Setups".

#### MacOS (10.13.3)

To make the GUI compatible, some modifications are needed, you can try to run the GUI following hereafter steps :

```
$ cd new-alpide-software/
$ make lib && make lib_analysis
(or  $ make && make lib && make lib_analysis if the software is not already compiled)

$ cd GUI/
$ qmake -makefile
```
At this stage, if you run a "make" command, you would get some errors. To fix them, you can start modifying the created Makefile this way :

- In the CFLAGS and CXXFLAGS, add the argument "-Wno-error-length-array". This is maybe not necessary to add to both flag lists but it works this way...
- Then, in the file env.sh, replace the following line :

  export LD_LIBRARY_PATH=$(readlink -f ${ROOT_DIR}/../):$LD_LIBRARY_PATH

  by the following one :

  export LD_LIBRARY_PATH="$(greadlink -f ../):"$LD_LIBRARY_PATH && DYLD_LIBRARY_PATH="$(greadlink -f ../):"$DYLD_LIBRARY_PATH

(These lines are already written in the file env.sh, to switch from one to another setup, you just have to comment/uncomment the corresponding lines).

Then compile and launch the GUI :

```
$ make

$ source env.sh
$ open GUI.app

```

## Versioning ##

- We use Cern Gitlab for versioning, you can find the repository at https://gitlab.cern.ch/alice-MFT-ladder-test/new-alpide-software

## Documentation ##

- The previously mentioned UserManual.pdf is an important document if you want more detailed informations about the software installation and using.
- You can also find other needed informations about ladders, test bench, readout boards, MOSAIC firmware in the twiki at :
  https://twiki.cern.ch/twiki/bin/view/ALICE/SchematicsAndManualsForLadderTestBench