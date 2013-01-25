# stage-4/Makefile

# Copyright (C) 2012, 2013 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

SHELL = /bin/sh
PATH  = .

RM    = /bin/rm
LN_S  = /bin/ln -sf
MAKE  = /usr/bin/make
CMP   = /usr/bin/cmp

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

%.s:	%.c cc0
	cc0 < $< > $@

LIB0_OBJS = memory.o string.o ctype.o exit.o unistd.o
CC_OBJS  = i386.o scanner.o symtab.o expr.o stmt.o main.o

lib0.o:	ld $(LIB0_OBJS) stdio.o
	ld -r -o lib0.o $(LIB0_OBJS) stdio.o

cc0:	ld lib0.o crt0.o $(CC_OBJS)
	ld -o cc0 lib0.o crt0.o $(CC_OBJS)

LIB1_OBJS = stdioobjs.o output.o input.o char.o

lib.o:	ld $(LIB0_OBJS) $(LIB1_OBJS)
	ld -r -o lib.o $(LIB0_OBJS) $(LIB1_OBJS)

cc:	ld lib.o crt0.o $(CC_OBJS)
	ld -o cc lib.o crt0.o $(CC_OBJS)

.INTERMEDIATE:	$(CC_OBJS) $(LIB0_OBJS) $(LIB1_OBJS) stdio.o lib0.o cc0

check:	check-output check-input

check-output: cc as
	./cc < output.c > output2.s
	./as output2.s
	$(CMP) output2.o output.o
	$(RM) output2.o output2.s

check-input: cc as
	./cc < input.c > input2.s
	./as input2.s
	$(CMP) input2.o input.o
	$(RM) input2.o input2.s

clean:
	$(RM) -f as ld $(LIB0_OBJS) stdio.o lib0.o crt0.o 
	$(RM) -f $(LIB1_OBJS) input.s output.s lib.o 
	$(RM) -f $(CC_OBJS) cc0 cc
	$(RM) -f output2.o output2.s input2.o input2.s

world:
	$(MAKE) clean
	$(MAKE) all
	$(MAKE) check
