CC=g++
GIT_VERSION:=$(shell git describe --dirty --always)

### Static libraries which are located in subfolders
LIBMOSAIC_DIR=./MosaicSrc/libmosaic
LIBPOWERBOARD_DIR=./MosaicSrc/libpowerboard
LIBALUCMS_DIR=./DataBaseSrc
STATIC_LIBS=$(LIBMOSAIC_DIR) $(LIBPOWERBOARD_DIR) $(LIBALUCMS_DIR)

INCLUDE=-I. -Iinc -isystem/usr/local/include -I./MosaicSrc -I$(LIBMOSAIC_DIR)/include -I$(LIBPOWERBOARD_DIR)/include -I$(LIBALUCMS_DIR) -isystem/opt/local/include -isystem/usr/include/libxml2 -isystem/opt/local/include/libxml2
LIB=-L/usr/local/lib -L/opt/local/lib -L$(LIBPOWERBOARD_DIR) -lpowerboard -L$(LIBMOSAIC_DIR) -lmosaic -L$(LIBALUCMS_DIR) -lalucms -lxml2 -lcurl
CFLAGS= -O2 -pipe -fPIC -g -std=c++11 -Wall -Werror -pedantic $(INCLUDE) -DVERSION=\"$(GIT_VERSION)\"

LINUX_LINKFLAGS=
UNAME_S := $(shell uname -s)
ifneq ($(UNAME_S),Darwin)
	LINUX_LINKFLAGS=-lpthread
endif

LINKFLAGS=-lusb-1.0 -ltinyxml $(LIB) $(LINUX_LINKFLAGS)


### Libraries
LIBRARY=libalpide.so
ANALYSIS_LIBRARY=libalpide_analysis.so

### ROOT specific variables
ROOTCONFIG   := $(shell which root-config)
ROOTCFLAGS   := $(shell $(ROOTCONFIG) --cflags | sed -e 's/-pthread//g')
ROOTLDFLAGS  := $(shell $(ROOTCONFIG) --ldflags)
ROOTLIBS     := $(shell $(ROOTCONFIG) --glibs | sed -e 's/-lpthread//g')


### Source files
BASE_CLASSES= TReadoutBoard.cpp TAlpide.cpp AlpideConfig.cpp AlpideDecoder.cpp AlpideDebug.cpp THIC.cpp \
  USB.cpp USBHelpers.cpp TReadoutBoardDAQ.cpp TReadoutBoardMOSAIC.cpp TChipConfig.cpp \
  TBoardConfig.cpp TBoardConfigDAQ.cpp TBoardConfigMOSAIC.cpp TConfig.cpp TPowerBoard.cpp \
  TPowerBoardConfig.cpp BoardDecoder.cpp SetupHelpers.cpp THisto.cpp TScanAnalysis.cpp \
  TPowerAnalysis.cpp TDigitalAnalysis.cpp TDigitalWFAnalysis.cpp TFifoAnalysis.cpp TLocalBusAnalysis.cpp \
  TDACScan.cpp TDataTaking.cpp TReadoutTest.cpp TEnduranceCycle.cpp TCycleAnalysis.cpp TReadoutAnalysis.cpp \
  TNoiseAnalysis.cpp TScan.cpp TFifoTest.cpp TPowerTest.cpp TSCurveScan.cpp TDigitalScan.cpp \
  TNoiseOccupancy.cpp TLocalBusTest.cpp TScanConfig.cpp TestBeamTools.cpp Common.cpp \
  TReadoutBoardRU.cpp TBoardConfigRU.cpp TApplyMask.cpp THicConfig.cpp
BASE_OBJS = $(BASE_CLASSES:.cpp=.o)

RU_SOURCES = ReadoutUnitSrc/TRuWishboneModule.cpp ReadoutUnitSrc/TRuTransceiverModule.cpp \
  ReadoutUnitSrc/TRuDctrlModule.cpp
RU_OBJS = $(RU_SOURCES:.cpp=.o)

MOSAIC_SOURCES = MosaicSrc/alpidercv.cpp MosaicSrc/controlinterface.cpp MosaicSrc/pexception.cpp \
  MosaicSrc/TAlpideDataParser.cpp
MOSAIC_OBJS = $(MOSAIC_SOURCES:.cpp=.o)

OBJS=$(BASE_OBJS) $(RU_OBJS) $(MOSAIC_OBJS)

### Source files using ROOT classes
ROOT_CLASSES= TSCurveAnalysis.cpp TApplyTuning.cpp
ROOT_OBJS  = $(ROOT_CLASSES:.cpp=.o)

### Dependencies
DEPS = $(OBJS) $(STATIC_LIBS)

### Executables
# Definition of the executables
EXE = startclk stopclk

# test_* executables without ROOT
TEST_EXE = test_mosaic test_noiseocc test_threshold test_digitalscan test_fifo test_dacscan \
  test_pulselength test_source test_poweron test_noiseocc_ext test_temperature test_readoutunit \
  test_localbus test_chip_count test_alucms test_dacscan_voltage test_supply_voltage test_GRST
EXE += $(TEST_EXE)

# test_* executables with ROOT
TEST_EXE_ROOT =  test_roottest test_scantest test_threshold_v1 test_tuneITHR test_ITHRthreshold \
  test_tuneVCASN test_VCASNthreshold test_scantest_digital
EXE += $(TEST_EXE_ROOT)


#### TARGETS ####
all: $(EXE) Config.cfg

### Config.cfg
Config.cfg: ConfigTemplate.cfg
	bash -c 'if [[ ! -f Config.cfg ]]; then cp -v ConfigTemplate.cfg Config.cfg ; else echo "Cannot update the Config.cfg, local changes are present"; fi'

### EXECUTABLES
# test_* executables without ROOT using Pattern Rules
$(TEST_EXE) : test_% : exe/main_%.cpp $(DEPS)
	$(CC) -o $@ $(OBJS) $(CFLAGS) $< $(LINKFLAGS)

# test_* executables with ROOT using Pattern Rules
$(TEST_EXE_ROOT): test_% : exe/main_%.cpp $(DEPS) $(ROOT_OBJS)
	$(CC) -o $@ $(OBJS) $(ROOT_OBJS) $(CFLAGS) $(ROOTCFLAGS) $< $(LINKFLAGS) $(ROOTLDFLAGS) $(ROOTLIBS)

# executables with special rules
stopclk: exe/main_stopclk.cpp $(DEPS)
	$(CC) -o stopclk $(OBJS) $(CFLAGS) $< $(LINKFLAGS)

startclk: exe/main_startclk.cpp $(DEPS)
	$(CC) -o startclk $(OBJS) $(CFLAGS) $< $(LINKFLAGS)

### DYNAMIC LIBRARIES
lib: $(DEPS)
	$(CC) -shared $(OBJS) $(CFLAGS) $(LINKFLAGS) -o $(LIBRARY)

lib_analysis: $(DEPS) $(ROOT_OBJS)
	$(CC) -shared $(ROOT_OBJS)  $(CFLAGS) $(ROOTCFLAGS) $(LINKFLAGS) $(ROOTLDFLAGS) $(ROOTLIBS) -L. -lalpide -o $(ANALYSIS_LIBRARY)

### STATIC LIBRARIES (in subfolders used by the executables and dynamic libraries)
$(STATIC_LIBS):
	$(MAKE) -C $@ # execute the corresponding Makefile

### OBJECTS
# Classes
$(BASE_OBJS): %.o: src/%.cpp inc/%.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(RU_OBJS) $(MOSAIC_OBJS): %.o: %.cpp %.h
	$(CC) $(CFLAGS) -c -o $@ $<

# Classes using ROOT.
$(ROOT_OBJS): %.o: src/%.cpp inc/%.h
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

.PHONY:	all clean clean-all $(STATIC_LIBS) lib lib_analysis
