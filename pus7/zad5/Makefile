PROJECT = 12
SYSTEM = linux
SRCDIR = src
BINDIR = bin
CC = gcc
CFLAGS = -O2 -pedantic -Wall

all: sftp_get

clean:
	@rm -rf $(BINDIR)/*

distrib: clean
	tar -cvf $(SYSTEM)-$(PROJECT).tar $(SRCDIR)/* Makefile

libcommon.o: $(SRCDIR)/libcommon.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o

sftp_get: sftp_get.o libcommon.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o $(BINDIR)/libcommon.o -lssh2 -o $(BINDIR)/$@

sftp_get.o: $(SRCDIR)/sftp_get.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o