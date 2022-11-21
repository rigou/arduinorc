
CC = g++
#CFLAGS = -g -DDEBUG
CFLAGS = -O -Wall

VERSION = 1.4

PAGER = less

FILES = Makefile libserial.h libserial.cpp txupload.cpp

txupload:	libserial.o txupload.o
	$(CC) $(CFLAGS) libserial.o txupload.o -o txupload

clean:
	rm -f *.o
