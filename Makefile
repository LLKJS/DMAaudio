##################
# Compiler
CXX = gcc
# Compiler flags
CFLAGS += -Wall -g
# Libraries
LDLIBS_ASOUND ?= -lasound
LDLIBS += $(LDLIBS_ASOUND)

##################
.PHONY: all clean

all: rec play

###### Record
rec: rec.o 
	$(CXX)  $(LDLIBS) -o rec rec.o
rec.o: rec.c rec.h Makefile
	$(CXX) $(CFLAGS) -c rec.c

###### Play
play: play.o
	$(CXX) $(LDLIBS) -o play play.o
play.o: play.c Makefile
	$(CXX) $(CFLAGS) -c play.c

clean:
	rm -f *.o *.d rec play
