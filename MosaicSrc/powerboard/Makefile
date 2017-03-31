
CC = gcc
CXX = g++
RM = rm -f
CPPFLAGS = -O2 -g -Wall -std=gnu++11 $(INC_DIR)
LDFLAGS = -O2 -g -Wall -std=gnu++11
OBJS = $(SRC:.cpp=.o)

LDLIBS = -L../libpowerboard -lpowerboard -L../libmosaic -lmosaic 
INC_DIR = -I../libmosaic/include -I../libpowerboard/include
LIBS = ../libmosaic ../libpowerboard


# list of source files
SRC = 	pbif.cpp \
		pbctrl.cpp

# Targets
ALL_EXE = pbctrl

all: $(LIBS) $(ALL_EXE) 

$(LIBS):
	$(MAKE) -C $@

pbctrl: $(OBJS) $(LIBS)
	$(CXX) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@   

depend: .depend

.depend: $(SRC)
	rm -f ./.depend
	$(CXX) $(CPPFLAGS) -MM $^>>./.depend;

clean:
	$(RM) $(OBJS)
	$(RM) *~

cleanall:
	@$(MAKE) clean
	$(RM) $(ALL_EXE)

dist-clean: clean
	$(RM) *~ .depend

.PHONY: all $(LIBS) 

include .depend
