CC=g++
INCLUDE=/usr/local/include
LIBPATH=/usr/local/lib
CFLAGS= -pipe -fPIC -g -std=c++0x -I $(INCLUDE)
LINKFLAGS=-lusb-1.0 -L $(LIBPATH)
#LINKFLAGS=
OBJECT= runTest
#LIBRARY=libpalpidefs.so
CLASS= TReadoutBoard.cpp TAlpide.cpp AlpideConfig.cpp AlpideDecoder.cpp USB.cpp USBHelpers.cpp TReadoutBoardDAQ.cpp TChipConfig.cpp TBoardConfigDAQ.cpp TConfig.cpp
#CLASS=  USB.cpp TDaqboard.cpp TPalpidefs.cpp TDut.cpp TTestsetup.cpp chiptests.cpp TConfig.cpp TModuleSetup.cpp
OBJS = $(CLASS:.cpp=.o)

all:    test

test:   $(OBJS) main.cpp
	$(CC) -o $(OBJECT) $(OBJS) $(CFLAGS) main.cpp $(LINKFLAGS)

%.o: 	%.cpp %.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o $(OBJECT)

