CC=g++
INCLUDE=/usr/local/include
LIBPATH=/usr/local/lib
CFLAGS= -pipe -fPIC -g -std=c++0x -I $(INCLUDE)
LINKFLAGS=-lusb-1.0 -ltinyxml -L $(LIBPATH)
#LINKFLAGS=
OBJECT= runTest
#LIBRARY=libpalpidefs.so
CLASS= TReadoutBoard.cpp TAlpide.cpp AlpideConfig.cpp AlpideDecoder.cpp USB.cpp USBHelpers.cpp TReadoutBoardDAQ.cpp \
 TReadoutBoardMOSAIC.cpp TChipConfig.cpp TBoardConfigDAQ.cpp TBoardConfigMOSAIC.cpp TConfig.cpp BoardDecoder.cpp SetupHelpers.cpp \
 MosaicSrc/alpide3rcv.cpp MosaicSrc/controlinterface.cpp MosaicSrc/i2cbus.cpp MosaicSrc/i2cslave.cpp MosaicSrc/i2csyspll.cpp \
 MosaicSrc/ipbus.cpp MosaicSrc/ipbusudp.cpp MosaicSrc/mdatagenerator.cpp MosaicSrc/mdatareceiver.cpp MosaicSrc/mdatasave.cpp \
 MosaicSrc/mexception.cpp MosaicSrc/mruncontrol.cpp MosaicSrc/mtriggercontrol.cpp MosaicSrc/mwbbslave.cpp \
 MosaicSrc/pexception.cpp MosaicSrc/pulser.cpp 
#CLASS=  USB.cpp TDaqboard.cpp TPalpidefs.cpp TDut.cpp TTestsetup.cpp chiptests.cpp TConfig.cpp TModuleSetup.cpp
OBJS = $(CLASS:.cpp=.o)
$(info OBJS="$(OBJS)")

all:    test test_mosaic test_noiseocc test_threshold test_digital test_fifo test_dacscan test_pulselength test_threshold_pb

#ThresholdScan:  $(OBJS) ThresholdScan.cpp
#	$(CC) -o ThresholdScan $(OBJS) $(CFLAGS) ThresholdScan.cpp $(LINKFLAGS)

test:   $(OBJS) main.cpp
	$(CC) -o $(OBJECT) $(OBJS) $(CFLAGS) main.cpp $(LINKFLAGS)

test_mosaic:   $(OBJS) main_mosaic.cpp
	$(CC) -o test_mosaic $(OBJS) $(CFLAGS) main_mosaic.cpp $(LINKFLAGS)

test_noiseocc:   $(OBJS) main_noiseocc.cpp
	$(CC) -o test_noiseocc $(OBJS) $(CFLAGS) main_noiseocc.cpp $(LINKFLAGS)

test_threshold:   $(OBJS) main_threshold.cpp
	$(CC) -o test_threshold $(OBJS) $(CFLAGS) main_threshold.cpp $(LINKFLAGS)

test_threshold_pb:   $(OBJS) main_threshold_pb.cpp
	$(CC) -o test_threshold_pb $(OBJS) $(CFLAGS) main_threshold_pb.cpp $(LINKFLAGS)

test_digital:   $(OBJS) main_digitalscan.cpp
	$(CC) -o test_digital $(OBJS) $(CFLAGS) main_digitalscan.cpp $(LINKFLAGS)

test_fifo:   $(OBJS) main_fifo.cpp
	$(CC) -o test_fifo $(OBJS) $(CFLAGS) main_fifo.cpp $(LINKFLAGS)

test_dacscan:   $(OBJS) main_dacscan.cpp
	$(CC) -o test_dacscan $(OBJS) $(CFLAGS) main_dacscan.cpp $(LINKFLAGS)

test_pulselength:   $(OBJS) main_pulselength.cpp
	$(CC) -o test_pulselength $(OBJS) $(CFLAGS) main_pulselength.cpp $(LINKFLAGS)

test_tid: $(OBJS) main_tid.cpp
	$(CC) -o test_tid $(OBJS) $(CFLAGS) main_tid.cpp $(LINKFLAGS)
  

%.o: 	%.cpp %.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o $(OBJECT)
	rm -rf MosaicSrc/*.o

