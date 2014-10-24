#
# makefile for the myftp client program
#
# Author: Joshua Shoebridge
#

myftp: myftp.o service.o
	gcc myftp.o service.o -o myftp

myftp.o: myftp.c
	gcc -c myftp.c

service.o: service.c service.h
	gcc -c service.c

clean:
	rm -rf *.o
