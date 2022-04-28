PROJECT = 04
SYSTEM = linux
SRCDIR = src
BINDIR = bin
CC = gcc
CFLAGS = -O2 -pedantic -Wall

all: client2 server2

clean:
	@rm -rf $(BINDIR)/*

distrib: clean
	tar -cvf $(SYSTEM)-$(PROJECT).tar $(SRCDIR)/* Makefile

client2: client2.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -o $(BINDIR)/$@ -lsctp

client2.o: $(SRCDIR)/client2.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

server2: server2.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -o $(BINDIR)/$@ -lsctp

server2.o: $(SRCDIR)/server2.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o
