PROJECT = 09
SYSTEM = linux
SRCDIR = src
BINDIR = bin
CC = gcc
CFLAGS = -O2 -pedantic -Wall

all: cipher_bio

clean:
	@rm -rf $(BINDIR)/*

distrib: clean
	tar -cvf $(SYSTEM)-$(PROJECT).tar $(SRCDIR)/* Makefile

cipher_bio: cipher_bio.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -lcrypto -o $(BINDIR)/$@ -lcrypto

cipher_bio.o: $(SRCDIR)/cipher_bio.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o