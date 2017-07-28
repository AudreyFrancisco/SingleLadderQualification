CC=g++
INCLUDE=-I/usr/include/libxml2
LIBPATH=
CFLAGS= -O2 -pipe -fPIC -g -std=c++11 $(INCLUDE)
LINKFLAGS=-lxml2 -lcurl $(LIBPATH)
LIBRARY=libalucms.so

CLASSES= AlpideDB.cpp AlpideDBEndPoints.cpp AlpideDBManager.cpp CernSsoCookiesJar.cpp utilities.cpp
OBJS = $(CLASSES:.cpp=.o)
DEPS = $(OBJS)

all: lib

lib: $(LIBRARY)

$(LIBRARY): $(DEPS)
	$(CC) -shared $(OBJS) $(CFLAGS) $(LINKFLAGS) -o $(LIBRARY)

%.o:    %.cpp %.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o

clean-all: clean
	rm -rf $(LIBRARY)

.PHONY: all lib clean clean-all
