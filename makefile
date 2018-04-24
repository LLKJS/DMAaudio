##################
# Compiler
CXX = gcc
# Compiler flags
CFLAGS += -Wall
# Libraries
LDLIBS_ASOUND ?= -lasound
LDLIBS += $(LDLIBS_ASOUND)

##################
.PHONY: all clean

all: rec play

###### Record
rec: rec.o 
	$(CXX) -o rec rec.o
rec.o: rec.c
	$(CXX) -c $(LDLIBS) rec.c

###### Play
play: play.o
	$(CXX) -o play play.o
play.o: play.c
	$(CXX) -c $(LDLIBS) play.c

clean:
	rm -f *.o *.d rec play
