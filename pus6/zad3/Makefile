PROJECT = 06
SYSTEM = linux
SRCDIR = src
BINDIR = bin
CC = gcc
CFLAGS = -O2 -pedantic -Wall

all: ipaddrmod

clean:
	@rm -rf $(BINDIR)/*

distrib: clean
	tar -cvf $(SYSTEM)-$(PROJECT).tar $(SRCDIR)/* Makefile

ipaddrmod: ipaddrmod.o libnetlink.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o $(BINDIR)/libnetlink.o -o $(BINDIR)/$@

ipaddrmod.o: $(SRCDIR)/ipaddrmod.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

libnetlink.o: $(SRCDIR)/libnetlink.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o
