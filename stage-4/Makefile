# stage-4/Makefile

# Copyright (C) 2012, 2013 Richard Smith <richard@ex-parrot.com>
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

# Suppress the default rules
.SUFFIXES:

%.o:	%.s as
	as $<

%.s:	%.c cc
	cc < $< > $@

LIB0_OBJS = memory.o string.o ctype.o exit.o unistd.o
CC_OBJS  = i386.o scanner.o symtab.o expr.o stmt.o main.o

lib0.o:	ld $(LIB0_OBJS) stdio.o
	ld -r -o lib0.o $(LIB0_OBJS) stdio.o

cc:	ld lib0.o crt0.o $(CC_OBJS)
	ld -o cc lib0.o crt0.o $(CC_OBJS)

.INTERMEDIATE:	$(CC_OBJS) $(LIB0_OBJS) stdio.o

check:	check-hello

LIB1_OBJS = stdioobjs.o output.o char.o

lib.o:	ld $(LIB0_OBJS) $(LIB1_OBJS)
	ld -r -o lib.o $(LIB0_OBJS) $(LIB1_OBJS)

hello:	cc hello.c lib.o crt0.o
	cc < hello.c > hello.s
	as hello.s
	ld -o hello lib.o crt0.o hello.o

check-hello:	hello
	hello

clean:
	$(RM) -f as ld $(LIB0_OBJS) stdio.o lib0.o crt0.o 
	$(RM) -f $(CC_OBJS) cc
	$(RM) -f $(LIB1_OBJS) lib.o hello hello.o hello.s

world:
	$(MAKE) clean
	$(MAKE) all
	$(MAKE) check
