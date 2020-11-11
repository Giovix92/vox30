# $Id: Makefile,v 1.3 2012/09/20 12:46:01 nanard Exp $
CC=gcc
CFLAGS=-Wall -I.
ifeq ($(CONFIG_SUPPORT_BUILD_DEBUG), 1)
CFLAGS += -g 
endif
RM=rm -f

all: testipfwrdr

clean:
	$(RM) *.o testipfwrdr

testipfwrdr: testipfwrdr.o ipfwrdr.o ipfwaux.o
	$(CC) -o $@ $^

ipfwrdr.o: ipfwrdr.c ipfwaux.c

testipfwrdr.o: testipfwrdr.c

