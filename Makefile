#Sample Makefile. You can make changes to this file according to your need
# The executable must be named proxy

CC = gcc
CFLAGS = -Wall -g 
LDFLAGS = -lpthread

OBJS1 = T3Server.o csapp.o
OBJS2 = T3Client.o csapp.o

all: T3Client T3Server 

T3Client: $(OBJS2)
	$(CC) $(OBJS2) $(LDFLAGS) -o T3Client

T3Server: $(OBJS1)
	$(CC) $(OBJS1) $(LDFLAGS) -o T3Server

csapp.o: csapp.c
	$(CC) $(CFLAGS) -c csapp.c

T3Client.o: T3Client.c
	$(CC) $(CFLAGS) -c T3Client.c

T3Server.o: T3Server.c
	$(CC) $(CFLAGS) -c T3Server.c
	$(CC) $(CFLAGS) -c T3Client.c

clean:
	rm -f *~ *.o T3Server 
	rm -f *~ *.o T3Client

