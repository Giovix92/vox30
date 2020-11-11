# Makefile for uClibc
#
# Copyright (C) 2002-2003 Erik Andersen <andersen@uclibc.org>
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU Library General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option) any
# later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU Library General Public License for more
# details.
#
# You should have received a copy of the GNU Library General Public License
# along with this program; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

CFLAGS += -DHAVE_CONFIG_H -I. 
CFLAGS += -Os -Wall -s -fPIC

LIBFL=libfl.so

CSRC = buf.c ecs.c misc.c parse.c scanflags.c  sym.c  tblcmp.c \
       ccl.c filter.c nfa.c regex.c scanopt.c tables.c  yylex.c \
       dfa.c gen.c main.c options.c scan.c skel.c tables_shared.c

OBJS=$(patsubst %.c,%.o, $(CSRC))

all: $(LIBFL)

$(LIBFL) : $(OBJS)
	$(CC) -Os -s -shared -fPIC $(LDFLAGS) -o $@ -Wl,-soname,$@ $(OBJS)

$(OBJS): %.o : %.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ): Makefile

install:
	install -m 644 $(LIBFL) $(INSTALLDIR)/lib

clean: 
	$(RM) *.[oa] *~ core $(LIBFL)*
