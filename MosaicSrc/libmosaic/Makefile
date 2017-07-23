CC = gcc
CXX = g++
AR = ar rcs
RM = rm -f
CPPFLAGS = -O2 -g -Wall -std=gnu++11 -fPIC -I./include
LDFLAGS = ""
LDLIBS = ""

LIB_SRC = 		src/genconsumer.cpp \
				src/i2cbus.cpp \
				src/i2cslave.cpp \
				src/i2csyspll.cpp \
				src/ipbus.cpp \
				src/ipbusudp.cpp \
				src/mboard.cpp \
				src/mdatagenerator.cpp \
				src/mdatareceiver.cpp \
				src/mdatasave.cpp \
				src/mexception.cpp \
				src/mruncontrol.cpp \
				src/mservice.cpp \
				src/mtriggercontrol.cpp \
				src/mwbbslave.cpp \
				src/pulser.cpp \
				src/trgrecorder.cpp


LIB_OBJS = $(LIB_SRC:.cpp=.o)

TARGET_LIB = libmosaic.a

all: $(TARGET_LIB)

$(TARGET_LIB): $(LIB_OBJS)
	$(RM) $@
	$(AR) $@ $(LIB_OBJS)
	
	
depend: .depend

.depend: $(SRC)
	rm -f ./.depend
	$(CXX) $(CPPFLAGS) -MM $^>>./.depend;

clean:
	$(RM) $(LIB_OBJS)
	$(RM) *~

cleanall:
	@$(MAKE) clean
	$(RM) $(TARGET_LIB)

dist-clean: clean
	$(RM) *~ .depend

include .depend
