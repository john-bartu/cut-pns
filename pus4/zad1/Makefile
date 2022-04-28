PROJECT = 04
SYSTEM = linux
SRCDIR = src
BINDIR = bin
CC = gcc
CFLAGS = -O2 -pedantic -Wall -lsctp

all: client1 server1

clean:
	@rm -rf $(BINDIR)/*

distrib: clean
	tar -cvf $(SYSTEM)-$(PROJECT).tar $(SRCDIR)/* Makefile

client1: client1.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -o $(BINDIR)/$@

client1.o: $(SRCDIR)/client1.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

server1: server1.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -o $(BINDIR)/$@

server1.o: $(SRCDIR)/server1.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o
