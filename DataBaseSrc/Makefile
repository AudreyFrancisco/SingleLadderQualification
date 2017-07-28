CC=g++
INCLUDE=-I/usr/include/libxml2
LIBPATH=
CFLAGS= -O2 -pipe -fPIC -g -std=c++11 $(INCLUDE)
LINKFLAGS=-lxml2 -lcurl $(LIBPATH)




CLASSES= AlpideDB.cpp AlpideDBEndPoints.cpp AlpideDBManager.cpp CernSsoCookiesJar.cpp utilities.cpp
OBJS = $(CLASSES:.cpp=.o)
DEPS = $(OBJS)

test_db: $(DEPS)
	$(CC) -o test_db $(OBJS) $(CFLAGS) main.cpp $(LINKFLAGS)


%.o:    %.cpp %.h
	$(CC) $(CFLAGS) -c -o $@ $<


# Other Targets


clean:

.PHONY: all clean
