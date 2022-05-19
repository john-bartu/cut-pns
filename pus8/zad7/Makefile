PROJECT = 09
SYSTEM = linux
SRCDIR = src
BINDIR = bin
CC = gcc
CFLAGS = -O2 -pedantic -Wall

all: rsa client_rsa server_rsa

clean:
	@rm -rf $(BINDIR)/*

distrib: clean
	tar -cvf $(SYSTEM)-$(PROJECT).tar $(SRCDIR)/* Makefile

rsa: rsa.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -lcrypto -o $(BINDIR)/$@ -lcrypto

rsa.o: $(SRCDIR)/rsa.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

client_rsa: client_rsa.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -lcrypto -o $(BINDIR)/$@ -lcrypto

client_rsa.o: $(SRCDIR)/client_rsa.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

server_rsa: server_rsa.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -lcrypto -o $(BINDIR)/$@ -lcrypto

server_rsa.o: $(SRCDIR)/server_rsa.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

