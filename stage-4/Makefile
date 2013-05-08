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
	cc0 -S $<

LIB0_OBJS = string.o ctype.o exit.o unistd.o
CC_OBJS  = i386.o scanner.o symtab.o expr.o stmt.o main.o

libc0.o:	ld $(LIB0_OBJS) stdio.o memory.o
	ld -r -o libc0.o $(LIB0_OBJS) stdio.o memory.o

cc0:	ld libc0.o crt0.o $(CC_OBJS)
	ld -o cc0 libc0.o crt0.o $(CC_OBJS)

LIB1_OBJS = output.o input.o char.o malloc.o signal.o

libc.o:	ld $(LIB0_OBJS) $(LIB1_OBJS)
	ld -r -o libc.o $(LIB0_OBJS) $(LIB1_OBJS)

cc:	ld libc.o crt0.o $(CC_OBJS)
	ld -o cc libc.o crt0.o $(CC_OBJS)

.INTERMEDIATE:	$(CC_OBJS) $(LIB0_OBJS) $(LIB1_OBJS) \
	stdio.o memory.o libc0.o cc0

check:	check-output check-input check-malloc

check-output: cc as output.c output.o
	$(LN_S) output.c output2.c
	cc -S output2.c
	as output2.s
	$(CMP) output2.o output.o
	$(RM) output2.c output2.o output2.s

check-input: cc as input.c input.o
	$(LN_S) input.c input2.c
	cc -S input2.c
	as input2.s
	$(CMP) input2.o input.o
	$(RM) input2.c input2.o input2.s

check-malloc: cc as malloc.c malloc.o
	$(LN_S) malloc.c malloc2.c
	cc -S malloc2.c
	as malloc2.s
	$(CMP) malloc2.o malloc.o
	$(RM) malloc2.c malloc2.o malloc2.s

clean:
	$(RM) -f as ld $(LIB0_OBJS) stdio.o memory.o libc0.o crt0.o 
	$(RM) -f $(LIB1_OBJS) $(LIB1_OBJS:%o=%.s) libc.o
	$(RM) -f $(CC_OBJS) cc0 cc
	$(RM) -f output2.o output2.s input2.o input2.s malloc2.o malloc2.s

world:
	$(MAKE) clean
	$(MAKE) all
	$(MAKE) check
