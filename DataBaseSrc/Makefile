CC=g++
AR = ar rcs
RM = rm -f
INCLUDE=-I/usr/include/libxml2 -isystem/opt/local/include/libxml2 -I. -I../inc
LIBPATH=
CFLAGS= -O2 -pipe -fPIC -g  -Wall -Werror -pedantic -std=c++11 $(INCLUDE)
LINKFLAGS=-lxml2 -lcurl $(LIBPATH)
LIBRARY=libalucms.a

CLASSES= AlpideDB.cpp AlpideDBEndPoints.cpp AlpideDBManager.cpp CernSsoCookiesJar.cpp utilities.cpp cJSON.c DBQueryQue.cpp DBHelpers.cpp
OBJS = $(CLASSES:.cpp=.o)

all: lib

lib: $(LIBRARY)

$(LIBRARY): $(OBJS)
	$(RM) $@
	$(AR) $@ $(OBJS)


%.o:    %.cpp %.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o

clean-all: clean
	rm -rf $(LIBRARY)

.PHONY: all lib clean clean-all
