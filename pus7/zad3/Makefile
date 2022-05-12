PROJECT = 12
SYSTEM = linux
SRCDIR = src
BINDIR = bin
CC = gcc
CFLAGS = -O2 -pedantic -Wall

all: exec_rsa

clean:
	@rm -rf $(BINDIR)/*

distrib: clean
	tar -cvf $(SYSTEM)-$(PROJECT).tar $(SRCDIR)/* Makefile

libcommon.o: $(SRCDIR)/libcommon.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

exec_rsa: exec_rsa.o libcommon.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o $(BINDIR)/libcommon.o -lssh2 -o $(BINDIR)/$@

exec_rsa.o: $(SRCDIR)/exec_rsa.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o