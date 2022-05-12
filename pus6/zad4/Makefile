PROJECT = 06
SYSTEM = linux
SRCDIR = src
BINDIR = bin
CC = gcc
CFLAGS = -O2 -pedantic -Wall

all: event

clean:
	@rm -rf $(BINDIR)/*

distrib: clean
	tar -cvf $(SYSTEM)-$(PROJECT).tar $(SRCDIR)/* Makefile

event: event.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -o $(BINDIR)/$@

event.o: $(SRCDIR)/event.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o
