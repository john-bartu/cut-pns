PROJECT = 09
SYSTEM = linux
SRCDIR = src
BINDIR = bin
CC = gcc
CFLAGS = -O2 -pedantic -Wall

all: mac

clean:
	@rm -rf $(BINDIR)/*

distrib: clean
	tar -cvf $(SYSTEM)-$(PROJECT).tar $(SRCDIR)/* Makefile

mac: mac.o
	$(CC) $(CFLAGS) -lssl -lcrypto $(BINDIR)/$@.o -lssl -o $(BINDIR)/$@ -lcrypto

mac.o: $(SRCDIR)/mac.c
	$(CC) $(CFLAGS) -lssl -lcrypto -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o -lcrypto
