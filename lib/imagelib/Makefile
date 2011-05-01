# Makefile for imagelib

MACHTYPE=$(shell uname -m)

GCC			= g++

CC=gcc
OPTFLAGS=-g2
# OPTFLAGS=-O3
OTHERFLAGS=-Wall

IMAGELIB_OBJS= qsort.o util.o

CFLAGS=$(OTHERFLAGS) $(OPTFLAGS)

IMAGELIB=libimage.a

%.$(P).o : %.c
	$(CC) -c -o $@ $(CFLAGS) $<

all: $(IMAGELIB)

$(IMAGELIB): $(IMAGELIB_OBJS)
	ar r $(IMAGELIB) $(IMAGELIB_OBJS)
	cp $(IMAGELIB) ..

clean:
	rm -f *.o *~ $(IMAGELIB)
