# stage-5/Makefile

# Copyright (C) 2013 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

SHELL = /bin/sh
PATH  = .

RM    = /bin/rm
LN_S  = /bin/ln -sf
MAKE  = /usr/bin/make
CMP   = /usr/bin/cmp

all:	cc

INIT = as ld cc0 libc.o crt0.o

init:	$(INIT)

# cc0 is the compiler imported from stage 4.
# cc1 is this stage's compiler (written in the cc0 language) compiled with cc0
# cc  is this stage's compiler compiled with itself (i.e. with cc1)

$(INIT):
	$(MAKE) -C ../stage-4
	set -e; for x in as ld libc.o crt0.o; do $(LN_S) ../stage-4/$$x; done
	$(LN_S) ../stage-4/cc cc0

# Suppress the default rules
.SUFFIXES:

%.o:	%.s as
	as $<

%.s:	%.c cc0
	cc0 -S $<

%1.s:	%.c cc1
	cc1 -S -o $@ $<

%2.s:	%.c cc
	cc -S -o $@ $<

CC_OBJS  = scanner.o symtab.o expr.o stmt.o i386.o codegen.o main.o

cc1:	ld libc.o crt0.o $(CC_OBJS)
	ld -o cc1 libc.o crt0.o $(CC_OBJS)

cc:	ld libc.o crt0.o $(CC_OBJS:%.o=%1.o)
	ld -o cc libc.o crt0.o $(CC_OBJS:%.o=%1.o)

.INTERMEDIATE:  $(CC_OBJS) $(CC_OBJS:%.o=%1.o) cc1

clean:
	$(RM) -f $(INIT)
	$(RM) -f $(CC_OBJS:%.o=%.s) $(CC_OBJS) cc1
	$(RM) -f $(CC_OBJS:%.o=%1.s) $(CC_OBJS:%.o=%1.o) cc
	$(RM) -f $(CC_OBJS:%.o=%2.s) $(CC_OBJS:%.o=%2.o) cc2

check:	check-cc

cc2:	ld libc.o crt0.o $(CC_OBJS:%.o=%2.o)
	ld -o cc2 libc.o crt0.o $(CC_OBJS:%.o=%2.o)

check-cc:	cc2
	$(CMP) cc2 cc
	$(RM) -f $(CC_OBJS:%.o=%2.s) $(CC_OBJS:%.o=%2.o) cc2

world:
	$(MAKE) clean
	$(MAKE) all
	$(MAKE) check
