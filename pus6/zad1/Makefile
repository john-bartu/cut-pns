PROJECT = 06
SYSTEM = linux
SRCDIR = src
BINDIR = bin
CC = gcc
CFLAGS = -O2 -pedantic -Wall

all: list

clean:
	@rm -rf $(BINDIR)/*

distrib: clean
	tar -cvf $(SYSTEM)-$(PROJECT).tar $(SRCDIR)/* Makefile

list: list.o
	$(CC) $(CFLAGS) $(BINDIR)/$@.o -o $(BINDIR)/$@

list.o: $(SRCDIR)/list.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/$(basename $@).c -o $(BINDIR)/$(basename $@).o
