#Sample Makefile. You can make changes to this file according to your need
# The executable must be named proxy

CC = gcc
CFLAGS = -Wall -g 
LDFLAGS = -lpthread

OBJS1 = proxyfork.o csapp.o
OBJS2 = proxythread.o csapp.o

all: proxythread proxyfork 

proxythread: $(OBJS2)
	$(CC) $(OBJS2) $(LDFLAGS) -o proxythread

proxyfork: $(OBJS1)
	$(CC) $(OBJS1) $(LDFLAGS) -o proxyfork

csapp.o: csapp.c
	$(CC) $(CFLAGS) -c csapp.c

proxythread.o: proxythread.c
	$(CC) $(CFLAGS) -c proxythread.c

proxyfork.o: proxyfork.c
	$(CC) $(CFLAGS) -c proxyfork.c

clean:
	rm -f *~ *.o proxyfork 
	rm -f *~ *.o proxythread

