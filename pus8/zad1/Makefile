PROJECT = 09
SYSTEM = linux
SRCDIR = src
BINDIR = bin
CC = gcc
CFLAGS = -O2 -pedantic -Wall

all: cipher_ecb

clean:
	@rm -rf $(BINDIR)/*

distrib: clean
	tar -cvf $(SYSTEM)-$(PROJECT).tar $(SRCDIR)/* Makefile

cipher_ecb: cipher_ecb.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -lcrypto -o $(BINDIR)/$@ -lcrypto

cipher_ecb.o: $(SRCDIR)/cipher_ecb.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o
