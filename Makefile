CC=g++
LIBMOSAIC_DIR=./MosaicSrc/libmosaic
LIBPOWERBOARD_DIR=./MosaicSrc/libpowerboard
INCLUDE=-I/usr/local/include -I./MosaicSrc -I$(LIBMOSAIC_DIR)/include -I$(LIBPOWERBOARD_DIR)/include
LIB=-L/usr/local/lib -L$(LIBPOWERBOARD_DIR) -lpowerboard -L$(LIBMOSAIC_DIR) -lmosaic
LIBPATH=/usr/local/lib
CFLAGS= -O2 -pipe -fPIC -g -std=c++11 -mcmodel=medium -I $(INCLUDE)
LINKFLAGS=-lusb-1.0 -ltinyxml -lpthread -L $(LIBPATH) -L $(LIB)
#OBJECT= runTest
LIBRARY=libalpide.so

ROOTCONFIG   := $(shell which root-config)
ROOTCFLAGS   := $(shell $(ROOTCONFIG) --cflags)
ROOTLDFLAGS  := $(shell $(ROOTCONFIG) --ldflags)
ROOTLIBS     := $(shell $(ROOTCONFIG) --glibs)

RU_SOURCES = ReadoutUnitSrc/TRuWishboneModule.cpp ReadoutUnitSrc/TRuTransceiverModule.cpp ReadoutUnitSrc/TRuDctrlModule.cpp \
 TReadoutBoardRU.cpp TBoardConfigRU.cpp

MOSAIC_SOURCES = MosaicSrc/alpidercv.cpp MosaicSrc/controlinterface.cpp MosaicSrc/pexception.cpp MosaicSrc/TAlpideDataParser.cpp

CLASSES= TReadoutBoard.cpp TAlpide.cpp AlpideConfig.cpp AlpideDecoder.cpp AlpideDebug.cpp USB.cpp USBHelpers.cpp TReadoutBoardDAQ.cpp \
 TReadoutBoardMOSAIC.cpp TChipConfig.cpp TBoardConfig.cpp TBoardConfigDAQ.cpp TBoardConfigMOSAIC.cpp TConfig.cpp \
 BoardDecoder.cpp SetupHelpers.cpp THisto.cpp TScanAnalysis.cpp TDigitalAnalysis.cpp TFifoAnalysis.cpp\
 TScan.cpp TFifoTest.cpp TThresholdScan.cpp TDigitalScan.cpp TLocalBusTest.cpp TScanConfig.cpp TestBeamTools.cpp Common.cpp $(RU_SOURCES) $(MOSAIC_SOURCES)

OBJS = $(CLASSES:.cpp=.o)
$(info OBJS="$(OBJS)")

CLASSES_ROOT= TThresholdAnalysis.cpp

OBJS_ROOT  = $(CLASSES_ROOT:.cpp=.o)
OBJS_ROOT += $(OBJS)
$(info OBJS_ROOT="$(OBJS_ROOT)")

all:   $(LIBMOSAIC_DIR) $(LIBPOWERBOARD_DIR) stopclk startclk test_mosaic test_noiseocc test_threshold test_digital test_fifo test_dacscan test_pulselength test_source test_poweron test_noiseocc_ext test_temperature test_readoutunit test_localbus test_roottest test_scantest test_threshold_v1

$(LIBMOSAIC_DIR):
	$(MAKE) -C $@

$(LIBPOWERBOARD_DIR):
	$(MAKE) -C $@

lib: $(OBJS)
	$(CC) -shared $(OBJS) $(CFLAGS) $(LINKFLAGS) -o $(LIBRARY)

test_mosaic:   $(OBJS) main_mosaic.cpp
	$(CC) -o test_mosaic $(OBJS) $(CFLAGS) main_mosaic.cpp $(LINKFLAGS)

test_noiseocc:   $(OBJS) main_noiseocc.cpp
	$(CC) -o test_noiseocc $(OBJS) $(CFLAGS) main_noiseocc.cpp $(LINKFLAGS)

test_source:   $(OBJS) main_source.cpp
	$(CC) -o test_source $(OBJS) $(CFLAGS) main_source.cpp $(LINKFLAGS)

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

test_poweron:   $(OBJS) main_poweron.cpp
	$(CC) -o test_poweron $(OBJS) $(CFLAGS) main_poweron.cpp $(LINKFLAGS)

test_noiseocc_ext:   $(OBJS) main_noiseocc_ext.cpp
	$(CC) -o test_noiseocc_ext $(OBJS) $(CFLAGS) main_noiseocc_ext.cpp $(LINKFLAGS)

test_temperature:   $(OBJS) main_temperature.cpp
	$(CC) -o test_temperature $(OBJS) $(CFLAGS) main_temperature.cpp $(LINKFLAGS)

test_readoutunit: $(OBJS) main_readoutunit.cpp
	$(CC) -o test_readoutunit $(OBJS) $(CFLAGS) main_readoutunit.cpp $(LINKFLAGS)

test_localbus: $(OBJS) main_localbus.cpp
	$(CC) -o test_localbus $(OBJS) $(CFLAGS) main_localbus.cpp $(LINKFLAGS)

# Example ROOT executable: add $(ROOTCFLAGS) $(ROOTLDFLAGS) $(ROOTLIBS)
test_roottest: $(OBJS) main_roottest.cpp
	$(CC) -o test_roottest $(OBJS) $(CFLAGS) $(ROOTCFLAGS) main_roottest.cpp $(LINKFLAGS) $(ROOTLDFLAGS) $(ROOTLIBS)

# Executables with classes using ROOT. 
test_scantest: $(OBJS_ROOT) main_scantest.cpp
	$(CC) -o test_scantest $(OBJS_ROOT) $(CFLAGS)$(CFLAGS) $(ROOTCFLAGS) main_scantest.cpp $(LINKFLAGS) $(ROOTLDFLAGS) $(ROOTLIBS)

test_threshold_v1:   $(OBJS_ROOT) main_threshold_v1.cpp
	$(CC) -o test_threshold_v1 $(OBJS_ROOT) $(CFLAGS) $(ROOTCFLAGS) main_threshold_v1.cpp $(LINKFLAGS) $(ROOTLDFLAGS) $(ROOTLIBS)

# Classes using ROOT.
TThresholdAnalysis.o: TThresholdAnalysis.cpp TThresholdAnalysis.h 
	$(CC) $(CFLAGS) $(ROOTCFLAGS) -c -o $@ $<

stopclk:   $(OBJS) main_stopclk.cpp
	$(CC) -o stopclk $(OBJS) $(CFLAGS) main_stopclk.cpp $(LINKFLAGS)

startclk:   $(OBJS) main_startclk.cpp
	$(CC) -o startclk $(OBJS) $(CFLAGS) main_startclk.cpp $(LINKFLAGS)


%.o:    %.cpp %.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o
	rm -rf MosaicSrc/*.o
	rm -rf ReadoutUnitSrc/*.o

clean-all:	clean
	rm -rf test_*
	rm -rf libalpide.so

	$(MAKE) -C $(LIBMOSAIC_DIR) clean
	$(MAKE) -C $(LIBPOWERBOARD_DIR) clean

.PHONY:	all clean clean-all $(LIBMOSAIC_DIR) $(LIBPOWERBOARD_DIR)

