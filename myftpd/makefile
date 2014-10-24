#
# makefile for the myftpd server program
#
# Author: Joshua Shoebridge
#

myftpd: myftpd.o daemon.o service.o
	gcc myftpd.o daemon.o service.o -o myftpd

myftpd.o: myftpd.c
	gcc -c myftpd.c

daemon.o: daemon.c daemon.h
	gcc -c daemon.c

service.o: service.c service.h
	gcc -c service.c

clean:
	rm -rf *.o
