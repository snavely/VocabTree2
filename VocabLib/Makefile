# Makefile for VocabLib

MACHTYPE=$(shell uname -m)

GCC			= g++

CC=gcc
# OPTFLAGS=-g2
OPTFLAGS=-O3 -ffast-math -Wall -mfpmath=sse -msse2 -funroll-loops -march=core2
OTHERFLAGS=-Wall -fopenmp -pthread

INCLUDE_PATH=-I../lib/ann_1.1/include/ANN -I../lib/ann_1.1_char/include/ANN \
	-I../lib/imagelib -I../lib/zlib/include

OBJS=keys2.o kmeans.o kmeans_kd.o VocabTreeBuild.o VocabTreeIO.o \
	VocabTreeUtil.o VocabTree.o VocabFlatNode.o

CPPFLAGS=$(INCLUDE_PATH) $(OTHERFLAGS) $(OPTFLAGS)

LIB=libvocab.a

all: $(LIB)

$(LIB): $(OBJS)
	ar r $(LIB) $(OBJS)
	cp $(LIB) ..

clean:
	rm -f *.o *~ $(LIB)
