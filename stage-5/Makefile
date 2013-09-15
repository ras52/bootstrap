# stage-5/Makefile

# Copyright (C) 2013 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

SHELL = /bin/sh
PATH  = .

RM    = /bin/rm
LN_S  = /bin/ln -sf
MAKE  = /usr/bin/make
CMP   = /usr/bin/cmp

all:	cc cpp

INIT = as ld cc0 libc.o crt0.o

init:	$(INIT)

# cc0 is the compiler symlinked from stage 4.
# cc1 is this stage's compiler (written in the cc0 language) compiled with cc0.
# cc  is nearly the same code, though with a replacement node.c, compiled 
#       by itself (by cc1), and is therefore smaller.
# cc2 is a test compiler produced by cc; it should be binary identical to cc.

$(INIT):
	$(MAKE) -C ../stage-4
	set -e; for x in as ld libc.o crt0.o; do $(LN_S) ../stage-4/$$x; done
	$(LN_S) ../stage-4/cc cc0

# Suppress the default rules
.SUFFIXES:

%.o:	%.s as
	as $<

%0.s:	%.c cc0
	cc0 -S -o $@ $<

# The --compat option enables compatibility with the stage-4 cc.
%1.s:	%.c cc1
	cc1 -S --compat -o $@ $<

# We cannot remove the --compat flag on this build, because of the untyped
# use of structs-as-arrays in the code.  They should be coded as structs,
# but that's not possible because it would be too difficult to add struct 
# support to the stage-4 cc.
%2.s:	%.i cc
	cc -S --compat -o $@ $<

%.i:	%.c cpp
	cpp $< > $@

%.s:	%.c cc
	cc -S -o $@ $<

CC_OBJS  = scanbase.o scanner.o symtab.o expr.o stmt.o type.o \
           codegen.o i386.o main.o

# We replace node.o with a new version written using structs (and so that
# won't compile using stage-4 cc), partly as a test of the stage-4 cc.
CC0_OBJS = $(CC_OBJS) node.o
CC1_OBJS = $(CC_OBJS) nodenew.o

cc1:	ld libc.o crt0.o $(CC0_OBJS:%.o=%0.o)
	ld -o $@ libc.o crt0.o $(CC0_OBJS:%.o=%0.o)

cc:	ld libc.o crt0.o $(CC1_OBJS:%.o=%1.o)
	ld -o $@ libc.o crt0.o $(CC1_OBJS:%.o=%1.o)

CPP_OBJS = scanbase.o cpp.o nodenew.o

cpp:	ld libc.o crt0.o $(CPP_OBJS)
	ld -o $@ libc.o crt0.o $(CPP_OBJS)

.INTERMEDIATE:  $(CC0_OBJS:%.o=%0.o) $(CC1_OBJS:%.o=%1.o) cc1

clean:
	$(RM) -f $(INIT)
	$(RM) -f $(CC0_OBJS:%.o=%0.s) $(CC0_OBJS:%.o=%0.o) cc1
	$(RM) -f $(CC1_OBJS:%.o=%1.s) $(CC1_OBJS:%.o=%1.o) cc
	$(RM) -f $(CPP_OBJS:%.o=%.s) $(CPP_OBJS) cpp
	$(RM) -f $(CC1_OBJS:%.o=%2.s) $(CC1_OBJS:%.o=%2.i) $(CC1_OBJS:%.o=%2.o)
	$(RM) -f cc2

check:	check-cc check-cpp

cc2:	ld libc.o crt0.o $(CC1_OBJS:%.o=%2.o)
	ld -o cc2 libc.o crt0.o $(CC1_OBJS:%.o=%2.o)

check-cc:	cc2
	$(CMP) cc2 cc
	$(RM) -f $(CC1_OBJS:%.o=%2.s) $(CC1_OBJS:%.o=%2.o) cc2

# TODO: put some tests here
check-cpp:	cpp

world:
	$(MAKE) clean
	$(MAKE) all
	$(MAKE) check
