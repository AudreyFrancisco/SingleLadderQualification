CC=gcc
CXX=g++
AR = ar rcs
RM = rm -f
INCLUDE=-I./serial/include/ -I.
LIBPATH=
CPPFLAGS=-O2 -fPIC -g  -Wall -Werror -pedantic -std=c++11 $(INCLUDE)
CFLAGS=$(CPPFLAGS)
LIBRARY=libscopecontrol.a

CLASSES = scope_control.cpp \
          serial/src/serial.cc \
          serial/src/impl/unix.cc \
          serial/src/impl/list_ports/list_ports_linux.cc
OBJS1 = $(CLASSES:.cpp=.o)
OBJS  = $(OBJS1:.cc=.o)

all: lib

asd:
	echo $(CFLAGS)

lib: $(LIBRARY)

$(LIBRARY): $(OBJS)
	$(RM) $@
	$(AR) $@ $(OBJS)

%.o:    %.cpp %.h %.cc
	$(CXX) $(CPPFLAGS) -c -o $@ $<

test_scope_control: ./test/tester.cpp $(LIBRARY)
	$(CXX) -o test_scope_control ./test/tester.cpp $(CPPFLAGS) -lscopecontrol -lrt -L. $(LIBRARY)

clean:
	$(RM) $(OBJS)
	$(RM) test_scope_control

clean-all: clean
	$(RM) $(LIBRARY)

.PHONY: all lib clean clean-all

dist-clean: clean

.PHONY: clean cleanall dist-clean