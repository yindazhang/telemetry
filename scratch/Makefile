CC = g++
LINKERFLAG = -lm

SRCS := sketch.cc
BINS := sketch

all: sketch

sketch: sketch.o
	g++ -lm sketch.o -o sketch

sketch.o: sketch.cc
	g++ -c sketch.cc

clean:
	rm -rvf sketch.o sketch