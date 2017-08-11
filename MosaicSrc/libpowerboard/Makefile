CC = gcc
CXX = g++
AR = ar rcs
RM = rm -f
CPPFLAGS = -O2 -g -Wall -std=gnu++11 -pedantic -fPIC -I../libmosaic/include -I./include
LDFLAGS = ""
LDLIBS = ""

LIB_SRC = 		src/powerboard.cpp \
				src/ltc2635.cpp \
				src/pcf8574.cpp \
				src/ad5254.cpp \
				src/ad7997.cpp \
				src/sc18is602.cpp \
				src/max31865.cpp \
				src/max5419.cpp \
				src/ADC128D818.cpp


LIB_OBJS = $(LIB_SRC:.cpp=.o)

TARGET_LIB = libpowerboard.a

all: $(TARGET_LIB)

$(TARGET_LIB): $(LIB_OBJS)
	$(RM) $@
	$(AR) -o $@ $(LIB_OBJS)

clean:
	$(RM) $(LIB_OBJS)
	$(RM) *~

cleanall:
	@$(MAKE) clean
	$(RM) $(TARGET_LIB)

dist-clean: clean
	$(RM) *~

.PHONY: clean cleanall dist-clean
