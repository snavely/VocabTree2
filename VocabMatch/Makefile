# Makefile for VocabMatch

MACHTYPE=$(shell uname -m)

GCC			= g++

CC=gcc
# OPTFLAGS=-g2
OPTFLAGS=-O3 -fopenmp
OTHERFLAGS=-Wall

INCLUDE_PATH=-I../lib/ann_1.1/include/ANN -I../lib/ann_1.1_char/include/ANN \
	-I../lib/imagelib -I../VocabLib -I../lib/zlib/include
LIB_PATH=-L../lib -L../VocabLib -L../lib/zlib/lib

OBJS=VocabMatch.o
OBJS_DESC=VocabMatch_desc.o

LIBS=-lvocab -lANN -lANN_char -limage -lz

CPPFLAGS=$(INCLUDE_PATH) $(LIB_PATH) $(OTHERFLAGS) $(OPTFLAGS)

BIN=VocabMatch
BIN_DESC=VocabMatch_desc

all: $(BIN) $(BIN_DESC) VocabMatchScript VocabMatchScript_desc

$(BIN): $(OBJS)
	g++ -o $(CPPFLAGS) -o $(BIN) $(OBJS) $(LIBS)

$(BIN_DESC): $(OBJS_DESC)
	g++ -o $(CPPFLAGS) -o $@ $^ $(LIBS)

VocabMatchScript: VocabMatchScript.o
	g++ -o $(CPPFLAGS) -o $@ $^ $(LIBS)

VocabMatchScript_desc: VocabMatchScript_desc.o
	g++ -o $(CPPFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f *.o *~ $(LIB)
