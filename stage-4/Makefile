# stage-4/Makefile

# Copyright (C) 2012 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

SHELL = /bin/sh
PATH  = .

RM    = /bin/rm
LN_S  = /bin/ln -sf
MAKE  = /usr/bin/make

all:	bj

init:	as ld

as ld:
	$(MAKE) -C ../stage-3
	$(LN_S) ../stage-3/as
	$(LN_S) ../stage-3/ld

%.o:	%.s as
	as $<

#LIB_OBJS = memory.o stdio.o string.o ctype.o crt0.o
LIB_OBJS = stdio.o string.o ctype.o crt0.o
BJ_OBJS  = scanner.o main.o

bj:	ld $(LIB_OBJS) $(BJ_OBJS)
	ld -o bj $(LIB_OBJS) $(BJ_OBJS)

check:	all

clean:
	$(RM) -f as ld $(LIB_OBJS) $(BJ_OBJS) bj

world:
	$(MAKE) clean
	$(MAKE) all
	$(MAKE) check
