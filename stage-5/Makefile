# stage-5/Makefile

# Copyright (C) 2013 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

SHELL = /bin/sh
PATH  = .

RM    = /bin/rm
LN_S  = /bin/ln -sf
MAKE  = /usr/bin/make

all:	cc

INIT = as ld cc0 libc.o crt0.o

init:	$(INIT)

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

CC_OBJS  = scanner.o symtab.o expr.o stmt.o i386.o codegen.o main.o

cc:	ld libc.o crt0.o $(CC_OBJS)
	ld -o cc libc.o crt0.o $(CC_OBJS)

.INTERMEDIATE:  $(CC_OBJS)

clean:
	$(RM) -f $(INIT)
	$(RM) -f $(CC_OBJS) cc
	$(RM) -f $(CC_OBJS:%.o:%.s)

check:

world:
	$(MAKE) clean
	$(MAKE) all
	$(MAKE) check
