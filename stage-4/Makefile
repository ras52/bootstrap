# stage-4/Makefile

# Copyright (C) 2012, 2013, 2014, 2016, 2020 
# Richard Smith <richard@ex-parrot.com>
# All rights reserved.

SHELL  = /bin/sh

RM     = /bin/rm
CP     = /bin/cp
LN_S   = /bin/ln -sf
MAKE   = /usr/bin/make
CMP    = /usr/bin/cmp

BINDIR = ../bin
LIBDIR = ../lib
PATH   = $(BINDIR)


all:	init cc libc.o crt0.o

init:
	@test -d $(LIBDIR) || $(MAKE) -C .. init
	@test -x $(BINDIR)/ld || $(MAKE) -C ../stage-3 install

# Suppress the default rules
.SUFFIXES:

%.o:	%.s
	as $<

%.s:	%.c cc0
	./cc0 -S $<

# LIB_OBJS contain a minimal C library written entirely in assembler.
# LIB0_OBJS adds bootstrap .o files written in assembler that will be replaced.
# LIB1_OBJS adds to and replaces the LIB0_OBJS with versions written in C.

LIB_OBJS = string.o ctype.o unistd.o char.o imath.o
LIB0_OBJS = $(LIB_OBJS) error.o stdio.o memory.o
LIB1_OBJS = $(LIB_OBJS) exit.o output.o input.o malloc.o signal.o string2.o \
  stdarg.o

CC_OBJS  = i386.o scanner.o symtab.o expr.o stmt.o main.o

# libc0.o is the primative libc, written solely in assembler.
# cc0     is the compiler linked against the libc0.o.
# libc.o  is the complete stage-4 libc, with some C code compiled using cc0.
# cc      is the compiler relinked against the new libc.o.

libc0.o:	$(LIB0_OBJS)
	ld -r -o libc0.o $(LIB0_OBJS)

cc0:	libc0.o crt0.o $(CC_OBJS)
	ld -o cc0 libc0.o crt0.o $(CC_OBJS)


libc.o:	$(LIB1_OBJS)
	ld -r -o libc.o $(LIB1_OBJS)

cc:	libc.o crt0.o $(CC_OBJS)
	ld -o cc libc.o crt0.o $(CC_OBJS)

.INTERMEDIATE:	$(CC_OBJS) $(LIB1_OBJS) $(LIB0_OBJS) libc0.o cc0

check:	check-output check-input check-malloc

check-output: cc output.c output.o
	$(LN_S) output.c output2.c
	./cc -S output2.c
	as output2.s
	$(CMP) output2.o output.o
	$(RM) output2.c output2.o output2.s

check-input: cc input.c input.o
	$(LN_S) input.c input2.c
	./cc -S input2.c
	as input2.s
	$(CMP) input2.o input.o
	$(RM) input2.c input2.o input2.s

check-malloc: cc malloc.c malloc.o
	$(LN_S) malloc.c malloc2.c
	./cc -S malloc2.c
	as malloc2.s
	$(CMP) malloc2.o malloc.o
	$(RM) malloc2.c malloc2.o malloc2.s

install: init cc libc.o crt0.o
	$(CP) cc $(BINDIR)/cc0
	$(CP) libc.o crt0.o $(LIBDIR)

clean:
	$(RM) -f $(LIB0_OBJS)
	$(RM) -f $(LIB1_OBJS) output.s input.s malloc.s signal.s libc.o
	$(RM) -f $(CC_OBJS) cc0 cc crt0.o
	$(RM) -f output2.o output2.s input2.o input2.s malloc2.o malloc2.s

world:
	set -e; for TARGET in clean init all check install; do \
	    $(MAKE) $$TARGET; \
	done
