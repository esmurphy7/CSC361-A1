CC=gcc
CFLAGS=-c -Wall

all: sws

sws: sws.o responseManager.o slre.o
	$(CC) sws.o responseManager.o slre.o -o sws
	
objects: sws.c responseManager.c slre.c
	$(CC) $(CFLAGS) sws.c responseManager.c slre.c
	
clean:
	rm -rf *o sws