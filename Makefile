CC=g++
LIBMOSAIC_DIR=./MosaicSrc/libmosaic
LIBPOWERBOARD_DIR=./MosaicSrc/libpowerboard
INCLUDE=-I/usr/local/include -I./MosaicSrc -I$(LIBMOSAIC_DIR)/include -I$(LIBPOWERBOARD_DIR)/include
LIB=-L/usr/local/lib -L$(LIBPOWERBOARD_DIR) -lpowerboard -L$(LIBMOSAIC_DIR) -lmosaic
CFLAGS= -O2 -pipe -fPIC -g -std=c++0x -mcmodel=large $(INCLUDE)
LINKFLAGS=-lusb-1.0 -lpthread $(LIB)
#LINKFLAGS=
OBJECT= runTest
LIBRARY=libalpide.so

CLASS= TReadoutBoard.cpp TAlpide.cpp AlpideConfig.cpp AlpideDecoder.cpp USB.cpp USBHelpers.cpp TReadoutBoardDAQ.cpp \
 TReadoutBoardMOSAIC.cpp TChipConfig.cpp TBoardConfig.cpp TBoardConfigDAQ.cpp TBoardConfigMOSAIC.cpp TConfig.cpp \
 BoardDecoder.cpp SetupHelpers.cpp THisto.cpp\
 MosaicSrc/alpidercv.cpp MosaicSrc/controlinterface.cpp MosaicSrc/pexception.cpp MosaicSrc/TAlpideDataParser.cpp \
 TScan.cpp TThresholdScan.cpp TScanConfig.cpp
#CLASS=  USB.cpp TDaqboard.cpp TPalpidefs.cpp TDut.cpp TTestsetup.cpp chiptests.cpp TConfig.cpp TModuleSetup.cpp
OBJS = $(CLASS:.cpp=.o)
$(info OBJS="$(OBJS)")

all: $(LIBMOSAIC_DIR) $(LIBPOWERBOARD_DIR) test_stream test_mosaic test_noiseocc test_threshold test_digital test_fifo test_dacscan test_pulselength test_source test_poweron test_noiseocc_ext test_scantest test_temperature stopclk startclk

$(OBJECT):   $(OBJS) main.cpp
	$(CC) -o $(OBJECT) $(OBJS) $(CFLAGS) main.cpp $(LINKFLAGS)

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

test_scantest:   $(OBJS) main_scantest.cpp
	$(CC) -o test_scantest $(OBJS) $(CFLAGS) main_scantest.cpp $(LINKFLAGS)

test_temperature:   $(OBJS) main_temperature.cpp
	$(CC) -o test_temperature $(OBJS) $(CFLAGS) main_temperature.cpp $(LINKFLAGS)


test_stream:   $(OBJS) main_pipestream.cpp
	$(CC) -o test_stream $(OBJS) $(CFLAGS) main_pipestream.cpp $(LINKFLAGS)

stopclk:   $(OBJS) main_stopclk.cpp
	$(CC) -o stopclk $(OBJS) $(CFLAGS) main_stopclk.cpp $(LINKFLAGS)

startclk:   $(OBJS) main_startclk.cpp
	$(CC) -o startclk $(OBJS) $(CFLAGS) main_startclk.cpp $(LINKFLAGS)


%.o: 	%.cpp %.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o $(OBJECT)
	rm -rf MosaicSrc/*.o
	$(MAKE) -C $(LIBMOSAIC_DIR) clean
	$(MAKE) -C $(LIBPOWERBOARD_DIR) clean


.PHONY: all $(LIBMOSAIC_DIR) $(LIBPOWERBOARD_DIR)

