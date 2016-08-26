CC=g++
INCLUDE=/usr/local/include
LIBPATH=/usr/local/lib
CFLAGS= -pipe -fPIC -g -std=c++0x -I $(INCLUDE)
LINKFLAGS=-lusb-1.0 -L $(LIBPATH)
#LINKFLAGS=
OBJECT= runTest
#LIBRARY=libpalpidefs.so
CLASS= TReadoutBoard.cpp TAlpide.cpp AlpideConfig.cpp AlpideDecoder.cpp USB.cpp USBHelpers.cpp TReadoutBoardDAQ.cpp \
 TReadoutBoardMOSAIC.cpp TChipConfig.cpp TBoardConfigDAQ.cpp TBoardConfigMOSAIC.cpp TConfig.cpp BoardDecoder.cpp \
 MosaicSrc/alpide3rcv.cpp MosaicSrc/controlinterface.cpp MosaicSrc/i2cbus.cpp MosaicSrc/i2cslave.cpp MosaicSrc/i2csyspll.cpp \
 MosaicSrc/ipbus.cpp MosaicSrc/ipbusudp.cpp MosaicSrc/mdatagenerator.cpp MosaicSrc/mdatareceiver.cpp MosaicSrc/mdatasave.cpp \
 MosaicSrc/mexception.cpp MosaicSrc/mruncontrol.cpp MosaicSrc/mtriggercontrol.cpp MosaicSrc/mwbbslave.cpp \
 MosaicSrc/pexception.cpp MosaicSrc/pulser.cpp 
#CLASS=  USB.cpp TDaqboard.cpp TPalpidefs.cpp TDut.cpp TTestsetup.cpp chiptests.cpp TConfig.cpp TModuleSetup.cpp
OBJS = $(CLASS:.cpp=.o)
$(info OBJS="$(OBJS)")

all:    test ThresholdScan test_mosaic

ThresholdScan:  $(OBJS) ThresholdScan.cpp
	$(CC) -o ThresholdScan $(OBJS) $(CFLAGS) ThresholdScan.cpp $(LINKFLAGS)

test:   $(OBJS) main.cpp
	$(CC) -o $(OBJECT) $(OBJS) $(CFLAGS) main.cpp $(LINKFLAGS)

test_mosaic:   $(OBJS) main_mosaic.cpp
	$(CC) -o test_mosaic $(OBJS) $(CFLAGS) main_mosaic.cpp $(LINKFLAGS)

%.o: 	%.cpp %.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o $(OBJECT)
	rm -rf MosaicSrc/*.o

