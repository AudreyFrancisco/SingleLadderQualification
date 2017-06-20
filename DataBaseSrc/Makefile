# -----------------  Slight TS DB make file make 
# Auth : A.Franco
# Ver 1.0
#
#---------------------------------------------------------
#
# define the C compiler to use
CC = g++


# define header files dir other than /usr/include
INCLUDES = -I/usr/include/libxml2

# define any libraries to link into executable:
LIBS = -lxml2 -lcurl

# define any compile-time flags
CFLAGS = -O2 -pipe -fPIC -g -std=c++11 -mcmodel=medium $(INCLUDES)

# define the C source files
SRCS = AlpideDB.cpp AlpideDBEndPoints.cpp AlpideDBManager.cpp CernSsoCookiesJar.cpp utilities.cpp main.cpp

# define the C object files 
OBJS = $(SRCS:.c=.o)

# define the executable file 
MAIN = testDB

#
.PHONY: depend clean

all:	$(MAIN)
	@echo  Test Alpide DB is compiled. Enjoy!

$(MAIN):	$(OBJS) 
		$(CC) $(CFLAGS) -o $(MAIN) $(OBJS) $(LIBS)

# this is a suffix replacement rule for building .o's from .c's
.c.o:
	$(CC) $(CFLAGS) -c $<  -o $@

clean:
	$(RM) *.o *~ $(MAIN)