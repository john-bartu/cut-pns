PROJECT = 09
SYSTEM = linux
SRCDIR = src
BINDIR = bin
CC = gcc
CFLAGS = -O2 -pedantic -Wall

all: client_hmac server_hmac

clean:
	@rm -rf $(BINDIR)/*

distrib: clean
	tar -cvf $(SYSTEM)-$(PROJECT).tar $(SRCDIR)/* Makefile

client_hmac: client_hmac.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -lcrypto -lssl -o $(BINDIR)/$@ -lcrypto

client_hmac.o: $(SRCDIR)/client_hmac.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

server_hmac: server_hmac.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -lcrypto -lssl -o $(BINDIR)/$@ -lcrypto

server_hmac.o: $(SRCDIR)/server_hmac.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o
