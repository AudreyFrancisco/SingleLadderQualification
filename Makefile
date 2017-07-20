CC=g++
LIBMOSAIC_DIR=./MosaicSrc/libmosaic
LIBPOWERBOARD_DIR=./MosaicSrc/libpowerboard
INCLUDE=-I/usr/local/include -I./MosaicSrc -I$(LIBMOSAIC_DIR)/include -I$(LIBPOWERBOARD_DIR)/include
LIB=-L/usr/local/lib -L$(LIBPOWERBOARD_DIR) -lpowerboard -L$(LIBMOSAIC_DIR) -lmosaic
LIBPATH=/usr/local/lib
CFLAGS= -O2 -pipe -fPIC -g -std=c++11 -mcmodel=medium -I $(INCLUDE)
LINKFLAGS=-lusb-1.0 -ltinyxml -lpthread -L $(LIBPATH) $(LIB)
#OBJECT= runTest
LIBRARY=libalpide.so
ANALYSIS_LIBRARY=libalpide_analysis.so

ROOTCONFIG   := $(shell which root-config)
ROOTCFLAGS   := $(shell $(ROOTCONFIG) --cflags)
ROOTLDFLAGS  := $(shell $(ROOTCONFIG) --ldflags)
ROOTLIBS     := $(shell $(ROOTCONFIG) --glibs)

RU_SOURCES = ReadoutUnitSrc/TRuWishboneModule.cpp ReadoutUnitSrc/TRuTransceiverModule.cpp ReadoutUnitSrc/TRuDctrlModule.cpp \
 TReadoutBoardRU.cpp TBoardConfigRU.cpp

MOSAIC_SOURCES = MosaicSrc/alpidercv.cpp MosaicSrc/controlinterface.cpp MosaicSrc/pexception.cpp MosaicSrc/TAlpideDataParser.cpp

CLASSES= TReadoutBoard.cpp TAlpide.cpp AlpideConfig.cpp AlpideDecoder.cpp AlpideDebug.cpp THIC.cpp USB.cpp USBHelpers.cpp TReadoutBoardDAQ.cpp \
 TReadoutBoardMOSAIC.cpp TChipConfig.cpp TBoardConfig.cpp TBoardConfigDAQ.cpp TBoardConfigMOSAIC.cpp TConfig.cpp TPowerBoard.cpp TPowerBoardConfig.cpp\
 BoardDecoder.cpp SetupHelpers.cpp THisto.cpp TScanAnalysis.cpp TDigitalAnalysis.cpp TFifoAnalysis.cpp TNoiseAnalysis.cpp\
 TScan.cpp TFifoTest.cpp TThresholdScan.cpp TDigitalScan.cpp TNoiseOccupancy.cpp TLocalBusTest.cpp TScanConfig.cpp TestBeamTools.cpp Common.cpp $(RU_SOURCES) $(MOSAIC_SOURCES)

OBJS = $(CLASSES:.cpp=.o)
$(info OBJS="$(OBJS)")

CLASSES_ROOT= TThresholdAnalysis.cpp

OBJS_ROOT  = $(CLASSES_ROOT:.cpp=.o)
OBJS_ROOT += $(OBJS)
$(info OBJS_ROOT="$(OBJS_ROOT)")

DEPS = $(OBJS) $(LIBMOSAIC_DIR) $(LIBPOWERBOARD_DIR)

all:   $(LIBMOSAIC_DIR) $(LIBPOWERBOARD_DIR) stopclk startclk test_mosaic test_noiseocc test_threshold test_digital test_fifo test_dacscan test_pulselength test_source test_poweron test_noiseocc_ext test_temperature test_readoutunit test_localbus test_roottest test_scantest test_threshold_v1

$(LIBMOSAIC_DIR):
	$(MAKE) -C $@

$(LIBPOWERBOARD_DIR):
	$(MAKE) -C $@

lib: $(DEPS)
	$(CC) -shared $(OBJS) $(CFLAGS) $(LINKFLAGS) -o $(LIBRARY)

lib_analysis: $(DEPS) $(OBJS_ROOT)
	$(CC) -shared $(OBJS_ROOT) $(CFLAGS) $(ROOTCFLAGS) $(LINKFLAGS) $(ROOTLDFLAGS) $(ROOTLIBS) -o $(ANALYSIS_LIBRARY)

test_mosaic: $(DEPS) main_mosaic.cpp
	$(CC) -o test_mosaic $(OBJS) $(CFLAGS) main_mosaic.cpp $(LINKFLAGS)

test_noiseocc:   $(DEPS) main_noiseocc.cpp
	$(CC) -o test_noiseocc $(OBJS) $(CFLAGS) main_noiseocc.cpp $(LINKFLAGS)

test_source:   $(DEPS) main_source.cpp
	$(CC) -o test_source $(OBJS) $(CFLAGS) main_source.cpp $(LINKFLAGS)

test_threshold:   $(DEPS) main_threshold.cpp
	$(CC) -o test_threshold $(OBJS) $(CFLAGS) main_threshold.cpp $(LINKFLAGS)

test_threshold_pb:   $(DEPS) main_threshold_pb.cpp
	$(CC) -o test_threshold_pb $(OBJS) $(CFLAGS) main_threshold_pb.cpp $(LINKFLAGS)

test_digital:   $(DEPS) main_digitalscan.cpp
	$(CC) -o test_digital $(OBJS) $(CFLAGS) main_digitalscan.cpp $(LINKFLAGS)

test_fifo:   $(DEPS) main_fifo.cpp
	$(CC) -o test_fifo $(OBJS) $(CFLAGS) main_fifo.cpp $(LINKFLAGS)

test_dacscan:   $(DEPS) main_dacscan.cpp
	$(CC) -o test_dacscan $(OBJS) $(CFLAGS) main_dacscan.cpp $(LINKFLAGS)

test_pulselength:   $(DEPS) main_pulselength.cpp
	$(CC) -o test_pulselength $(OBJS) $(CFLAGS) main_pulselength.cpp $(LINKFLAGS)

test_poweron:   $(DEPS) main_poweron.cpp
	$(CC) -o test_poweron $(OBJS) $(CFLAGS) main_poweron.cpp $(LINKFLAGS)

test_powerboard:   $(DEPS) main_testpowerboard.cpp
	$(CC) -o test_powerboard $(OBJS) $(CFLAGS) main_testpowerboard.cpp $(LINKFLAGS)

test_noiseocc_ext:   $(DEPS) main_noiseocc_ext.cpp
	$(CC) -o test_noiseocc_ext $(OBJS) $(CFLAGS) main_noiseocc_ext.cpp $(LINKFLAGS)

test_temperature:   $(DEPS) main_temperature.cpp
	$(CC) -o test_temperature $(OBJS) $(CFLAGS) main_temperature.cpp $(LINKFLAGS)

test_readoutunit: $(DEPS) main_readoutunit.cpp
	$(CC) -o test_readoutunit $(OBJS) $(CFLAGS) main_readoutunit.cpp $(LINKFLAGS)

test_localbus: $(DEPS) main_localbus.cpp
	$(CC) -o test_localbus $(OBJS) $(CFLAGS) main_localbus.cpp $(LINKFLAGS)

# Example ROOT executable: add $(ROOTCFLAGS) $(ROOTLDFLAGS) $(ROOTLIBS)
test_roottest: $(DEPS) main_roottest.cpp
	$(CC) -o test_roottest $(OBJS) $(CFLAGS) $(ROOTCFLAGS) main_roottest.cpp $(LINKFLAGS) $(ROOTLDFLAGS) $(ROOTLIBS)

# Executables with classes using ROOT.
test_scantest: $(DEPS) $(OBJS_ROOT) main_scantest.cpp
	$(CC) -o test_scantest $(OBJS_ROOT) $(CFLAGS)$(CFLAGS) $(ROOTCFLAGS) main_scantest.cpp $(LINKFLAGS) $(ROOTLDFLAGS) $(ROOTLIBS)

test_threshold_v1: $(DEPS)  $(OBJS_ROOT) main_threshold_v1.cpp
	$(CC) -o test_threshold_v1 $(OBJS_ROOT) $(CFLAGS) $(ROOTCFLAGS) main_threshold_v1.cpp $(LINKFLAGS) $(ROOTLDFLAGS) $(ROOTLIBS)

#added for calibration
test_tuneITHR: $(DEPS)  $(OBJS_ROOT) main_tuneITHR.cpp
	$(CC) -o test_tuneITHR $(OBJS_ROOT) $(CFLAGS) $(ROOTCFLAGS) main_tuneITHR.cpp $(LINKFLAGS) $(ROOTLDFLAGS) $(ROOTLIBS)

test_ITHRthreshold: $(DEPS)  $(OBJS_ROOT) main_ITHRthreshold.cpp
	$(CC) -o test_ITHRthreshold $(OBJS_ROOT) $(CFLAGS) $(ROOTCFLAGS) main_ITHRthreshold.cpp $(LINKFLAGS) $(ROOTLDFLAGS) $(ROOTLIBS)

test_tuneVCASN: $(DEPS)  $(OBJS_ROOT) main_tuneVCASN.cpp
	$(CC) -o test_tuneVCASN $(OBJS_ROOT) $(CFLAGS) $(ROOTCFLAGS) main_tuneVCASN.cpp $(LINKFLAGS) $(ROOTLDFLAGS) $(ROOTLIBS)

test_VCASNthreshold: $(DEPS)  $(OBJS_ROOT) main_VCASNthreshold.cpp
	$(CC) -o test_VCASNthreshold $(OBJS_ROOT) $(CFLAGS) $(ROOTCFLAGS) main_VCASNthreshold.cpp $(LINKFLAGS) $(ROOTLDFLAGS) $(ROOTLIBS)

# Classes using ROOT.
TThresholdAnalysis.o: TThresholdAnalysis.cpp TThresholdAnalysis.h
	$(CC) $(CFLAGS) $(ROOTCFLAGS) -c -o $@ $<

stopclk: $(DEPS) main_stopclk.cpp
	$(CC) -o stopclk $(OBJS) $(CFLAGS) main_stopclk.cpp $(LINKFLAGS)

startclk: $(DEPS) main_startclk.cpp
	$(CC) -o startclk $(OBJS) $(CFLAGS) main_startclk.cpp $(LINKFLAGS)


%.o:    %.cpp %.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o
	rm -rf MosaicSrc/*.o
	rm -rf ReadoutUnitSrc/*.o

clean-all:	clean
	rm -rf test_*
	rm -rf $(LIBRARY)
	rm -rf $(ANALYSIS_LIBRARY)
	$(MAKE) -C $(LIBMOSAIC_DIR) cleanall
	$(MAKE) -C $(LIBPOWERBOARD_DIR) cleanall

.PHONY:	all clean clean-all $(LIBMOSAIC_DIR) $(LIBPOWERBOARD_DIR) lib lib_analysis
