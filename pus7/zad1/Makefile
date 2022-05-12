PROJECT = 12
SYSTEM = linux
SRCDIR = src
BINDIR = bin
CC = gcc
CFLAGS = -O2 -pedantic -Wall

all: authlist

clean:
	@rm -rf $(BINDIR)/*

distrib: clean
	tar -cvf $(SYSTEM)-$(PROJECT).tar $(SRCDIR)/* Makefile

authlist: authlist.o libcommon.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o  $(BINDIR)/libcommon.o -lssh2 -o $(BINDIR)/$@

authlist.o: $(SRCDIR)/authlist.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

libcommon.o: $(SRCDIR)/libcommon.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o