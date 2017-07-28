CC=g++
LIBMOSAIC_DIR=./MosaicSrc/libmosaic
LIBPOWERBOARD_DIR=./MosaicSrc/libpowerboard
LIBALUCMS_DIR=./DataBaseSrc
INCLUDE=-I/usr/local/include -I./MosaicSrc -I$(LIBMOSAIC_DIR)/include -I$(LIBPOWERBOARD_DIR)/include -I$(LIBALUCMS_DIR) -I/usr/include/libxml2
LIB=-L/usr/local/lib -L$(LIBPOWERBOARD_DIR) -lpowerboard -L$(LIBMOSAIC_DIR) -lmosaic -L$(LIBALUCMS_DIR) -lalucms -lxml2 -lcurl
CFLAGS= -O2 -pipe -fPIC -g -std=c++11 -mcmodel=medium $(INCLUDE)
LINKFLAGS=-lusb-1.0 -ltinyxml -lpthread $(LIB)


### Libraries
LIBRARY=libalpide.so
ANALYSIS_LIBRARY=libalpide_analysis.so

### ROOT specific variables
ROOTCONFIG   := $(shell which root-config)
ROOTCFLAGS   := $(shell $(ROOTCONFIG) --cflags)
ROOTLDFLAGS  := $(shell $(ROOTCONFIG) --ldflags)
ROOTLIBS     := $(shell $(ROOTCONFIG) --glibs)


### Source files
RU_SOURCES = ReadoutUnitSrc/TRuWishboneModule.cpp ReadoutUnitSrc/TRuTransceiverModule.cpp \
  ReadoutUnitSrc/TRuDctrlModule.cpp TReadoutBoardRU.cpp TBoardConfigRU.cpp

MOSAIC_SOURCES = MosaicSrc/alpidercv.cpp MosaicSrc/controlinterface.cpp MosaicSrc/pexception.cpp \
  MosaicSrc/TAlpideDataParser.cpp

CLASSES= TReadoutBoard.cpp TAlpide.cpp AlpideConfig.cpp AlpideDecoder.cpp AlpideDebug.cpp THIC.cpp \
  USB.cpp USBHelpers.cpp TReadoutBoardDAQ.cpp TReadoutBoardMOSAIC.cpp TChipConfig.cpp \
  TBoardConfig.cpp TBoardConfigDAQ.cpp TBoardConfigMOSAIC.cpp TConfig.cpp TPowerBoard.cpp \
  TPowerBoardConfig.cpp BoardDecoder.cpp SetupHelpers.cpp THisto.cpp TScanAnalysis.cpp \
  TDigitalAnalysis.cpp TFifoAnalysis.cpp TNoiseAnalysis.cpp TScan.cpp TFifoTest.cpp \
  TThresholdScan.cpp TDigitalScan.cpp TNoiseOccupancy.cpp TLocalBusTest.cpp TScanConfig.cpp \
  TestBeamTools.cpp Common.cpp $(RU_SOURCES) $(MOSAIC_SOURCES)
OBJS = $(CLASSES:.cpp=.o)

### Source files using ROOT classes
CLASSES_ROOT= TThresholdAnalysis.cpp
OBJS_ROOT  = $(CLASSES_ROOT:.cpp=.o)

### Dependencies
DEPS = $(OBJS) $(LIBMOSAIC_DIR) $(LIBPOWERBOARD_DIR) $(LIBALUCMS_DIR)

### Executables
# Definition of the executables
EXE = startclk stopclk

# test_* executables without ROOT
TEST_EXE = test_mosaic test_noiseocc test_threshold test_digitalscan test_fifo test_dacscan \
  test_pulselength test_source test_poweron test_noiseocc_ext test_temperature test_readoutunit \
  test_localbus test_chip_count test_alucms
EXE += $(TEST_EXE)

# test_* executables with ROOT
TEST_EXE_ROOT =  test_roottest test_scantest test_threshold_v1 test_tuneITHR test_ITHRthreshold \
  test_tuneVCASN test_VCASNthreshold
EXE+= $(TEST_EXE_ROOT)


#### TARGETS ####
all:   $(LIBMOSAIC_DIR) $(LIBPOWERBOARD_DIR) $(LIBALUCMS_DIR) $(EXE)

### EXECUTABLES
# test_* executables without ROOT using Pattern Rules
$(TEST_EXE) : test_% : main_%.cpp $(DEPS)
	$(CC) -o $@ $(OBJS) $(CFLAGS) $< $(LINKFLAGS)

# test_* executables with ROOT using Pattern Rules
$(TEST_EXE_ROOT): test_% : main_%.cpp $(DEPS) $(OBJS_ROOT)
	$(CC) -o $@ $(OBJS) $(OBJS_ROOT) $(CFLAGS) $(ROOTCFLAGS) $< $(LINKFLAGS) $(ROOTLDFLAGS) $(ROOTLIBS)

# executables with special rules
stopclk: $(DEPS) main_stopclk.cpp
	$(CC) -o stopclk $(OBJS) $(CFLAGS) main_stopclk.cpp $(LINKFLAGS)

startclk: $(DEPS) main_startclk.cpp
	$(CC) -o startclk $(OBJS) $(CFLAGS) main_startclk.cpp $(LINKFLAGS)

### DYNAMIC LIBRARIES
lib: $(DEPS)
	$(CC) -shared $(OBJS) $(CFLAGS) $(LINKFLAGS) -o $(LIBRARY)

lib_analysis: $(DEPS) $(OBJS_ROOT)
	$(CC) -shared $(OBJS_ROOT) $(CFLAGS) $(ROOTCFLAGS) $(LINKFLAGS) $(ROOTLDFLAGS) $(ROOTLIBS) -o $(ANALYSIS_LIBRARY)


### STATIC LIBRARIES (in subfolders used by the executables and dynamic libraries)
$(LIBMOSAIC_DIR):
	$(MAKE) -C $@

$(LIBPOWERBOARD_DIR):
	$(MAKE) -C $@

$(LIBALUCMS_DIR):
	$(MAKE) -C $@

### OBJECTS
# Classes
$(OBJS): %.o:    %.cpp %.h
	$(CC) $(CFLAGS) -c -o $@ $<

# Classes using ROOT.
$(OBJS_ROOT): %.o: %.cpp %.h
	$(CC) $(CFLAGS) $(ROOTCFLAGS) -c -o $@ $<

### CLEANING
clean:
	rm -rf *.o
	rm -rf MosaicSrc/*.o
	rm -rf ReadoutUnitSrc/*.o
	$(MAKE) -C $(LIBALUCMS_DIR) clean

clean-all:	clean
	rm -rf test_*
	rm -rf startclk stopclk
	rm -rf $(LIBRARY)
	rm -rf $(ANALYSIS_LIBRARY)
	$(MAKE) -C $(LIBMOSAIC_DIR) cleanall
	$(MAKE) -C $(LIBPOWERBOARD_DIR) cleanall
	$(MAKE) -C $(LIBALUCMS_DIR) clean-all

.PHONY:	all clean clean-all $(LIBMOSAIC_DIR) $(LIBPOWERBOARD_DIR) $(LIBALUCMS_DIR) lib lib_analysis
