# Running tests with RUv1

## Basic format

Follow the build instructions listed in the root of this repo (mkdir build, cd build, cmake .., make).
From the build directory, all tests have the following format:

./test_test -c /path/to/config.cfg

NOTE: ALL RUv1 TESTS REQUIRE A CONFIG FILE, THERE IS CURRENTLY NO DEFAULT. AN EXAMPLE CONFIG FILE CAN BE FOUND IN Config/Config_RUv1_IB_HIC.cfg.

## Currently supported tests
The currently supported tests for RUv1 are:
    
    -test_digitalscan:  
    Digitally pulses each chip with configurable masking stages, stores hitmap in a histogram.
    
    -test_readoutunit
    Tests some basic RUv1 modules, pulses a given number of pixels a given number of times on each enabled chip, does basic test of event decoding.
    
    -test_gbtdebug
    Pulses some pixels and outputs the resulting GBT frames in a nice visual way in some text files, complains if any counters are not what they should be. 
    
    -test_fifo
    Tests read/write of some on-chip "fifos".
    
    -test_temperature
    Reads the temperature of the chip and the FPGA.
    
    -test_slowscan
    Like digital scan, but pulses only one pixel at a time.
    
    -test_threadedreadout
    Old test, now all readout of event data on RUv1 is handeled on a separate thread.

## Editing the tests
Changing the tests can be easily done in exe/main_test.cpp. I've tried to leave comments to give some idea what should and shouldn't be changed, but there's probably no harm in trial and error...

## Comments
- Lane assignment to ChipID depends on the transition board, so even if you have an IB HIC your config file may differ
- Currently only supported with firefly input 4 (IB module input) as the firmware doesn't currently allow for GTH and GPIO transceivers to be enabled simultaneously (or something along those lines)
- Want something added/changed? Email me at ryan.p.hannigan@utexas.edu 
