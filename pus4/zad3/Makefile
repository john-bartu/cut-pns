PROJECT = 04
SYSTEM = linux
SRCDIR = src
BINDIR = bin
CC = gcc
CFLAGS = -O2 -pedantic -Wall -lsctp

all: client3 server3

clean:
	@rm -rf $(BINDIR)/*

distrib: clean
	tar -cvf $(SYSTEM)-$(PROJECT).tar $(SRCDIR)/* Makefile

client3: client3.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -o $(BINDIR)/$@ -lsctp

client3.o: $(SRCDIR)/client3.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

server3: server3.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -o $(BINDIR)/$@ -lsctp

server3.o: $(SRCDIR)/server3.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

