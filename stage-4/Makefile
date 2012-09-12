# stage-4/Makefile

# Copyright (C) 2012 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

SHELL = /bin/sh
PATH  = .

RM    = /bin/rm
LN_S  = /bin/ln -sf
MAKE  = /usr/bin/make

all:	cc

init:	as ld

as ld:
	$(MAKE) -C ../stage-3
	$(LN_S) ../stage-3/as
	$(LN_S) ../stage-3/ld

%.o:	%.s as
	as $<

#LIB_OBJS = memory.o stdio.o string.o ctype.o crt0.o
LIB_OBJS = stdio.o string.o ctype.o crt0.o
BJ_OBJS  = i386.o scanner.o expr.o main.o

cc:	ld $(LIB_OBJS) $(BJ_OBJS)
	ld -o cc $(LIB_OBJS) $(BJ_OBJS)

check:	all

clean:
	$(RM) -f as ld $(LIB_OBJS) $(BJ_OBJS) cc

world:
	$(MAKE) clean
	$(MAKE) all
	$(MAKE) check
