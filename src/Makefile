# Makefile for src

MACHTYPE=$(shell uname -m)

GCC=g++

CC=gcc
# OPTFLAGS=-g2
OPTFLAGS=-O3 -fopenmp
OTHERFLAGS=-Wall

INCLUDE_PATH=-I../lib/ann_1.1/include/ANN -I../lib/ann_1.1_char/include/ANN \
	-I../lib/imagelib -I../VocabLib -I../lib/zlib/include
LIB_PATH=-L../lib -L../VocabLib -L../lib/zlib/lib

OBJS=VocabCompare.o

LIBS=-lvocab -lANN -lANN_char -limage -lz

CPPFLAGS=$(INCLUDE_PATH) $(LIB_PATH) $(OTHERFLAGS) $(OPTFLAGS)

VOCABCOMPARE=VocabCompare
VOCABCOMBINE=VocabCombine

all: $(VOCABCOMPARE) $(VOCABCOMBINE)

$(VOCABCOMPARE): VocabCompare.o
	g++ -o $(CPPFLAGS) -o $@ $^ $(LIBS)

$(VOCABCOMBINE): VocabCombine.o
	g++ -o $(CPPFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f *.o *~ $(LIB)
