# stage-5/Makefile

# Copyright (C) 2013, 2014, 2015 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

SHELL = /bin/sh
PATH  = .

RM    = /bin/rm
LN_S  = /bin/ln -sf
MAKE  = /usr/bin/make

all:	ccx cpp cc cmp

# INIT and XINIT are programs imported from the previous stage.  The only 
# difference is that XINIT gets a '0' appended on import as they get 
# rebuilt in this stage.
INIT  = as ld libc.o crt0.o
XINIT = cc0

init:	$(INIT) $(XINIT)

# cc0  is the compiler symlinked from stage 4.
# ccx1 is this stage's compiler (written in the cc0 language) compiled with cc0.
# ccx  is nearly the same code, though with a replacement node.c, compiled 
#        by itself (by ccx1), and is therefore smaller.
# ccx2 is a test compiler produced by ccx; it should be binary identical to ccx.

$(INIT) $(XINIT):
	$(MAKE) -C ../stage-4
	set -e; for x in $(INIT); do $(LN_S) ../stage-4/$$x; done
	$(LN_S) ../stage-4/cc cc0

# Suppress the default rules
.SUFFIXES:

%.o:	%.s as
	as $<

%0.s:	%.c cc0
	cc0 -S -o $@ $<

# The --compatibility=4 flag enables compatibility with the stage-4 cc.
%1.s:	%.c ccx1
	ccx1 --compatibility=4 -o $@ $<

# We cannot remove --compatibility=4 on this build, because of the untyped
# use of structs-as-arrays in the code.  They should be coded as structs,
# but that's not possible because it would be too difficult to add struct 
# support to the stage-4 cc.
%2.s:	%.i ccx
	ccx --compatibility=4 -o $@ $<

%.i:	%.c cpp
	cpp -Iinclude -o $@ $<

%.s:	%.c ccx
	ccx -o $@ $<

# All these files get compiled with --compatibility=4
CCX_OBJS  = scanbase.o scanner.o symtab.o expr.o stmt.o type.o \
            codegen.o i386.o main.o cli.o

# We replace node.o with a new version written using structs (and so that
# won't compile using stage-4 cc), partly as a test of the stage-4 cc.
CCX0_OBJS = $(CCX_OBJS) node.o
CCX1_OBJS = $(CCX_OBJS) nodenew.o

ccx1:	ld libc.o crt0.o $(CCX0_OBJS:%.o=%0.o)
	ld -o $@ crt0.o $(CCX0_OBJS:%.o=%0.o) libc.o

ccx:	ld libc.o crt0.o $(CCX1_OBJS:%.o=%1.o)
	ld -o $@ crt0.o $(CCX1_OBJS:%.o=%1.o) libc.o

# These files need to compile without preprocessing, and are not given
# --compatiibility=4.  Note that cli.o, scanbase.o and expr.o are also 
# used by the compiler and need also to compile with the stage 4 cc.
CPP_OBJS = scanbase.o cpp.o nodenew.o macros.o pvector.o cli.o expr.o \
           cpptype.o eval.o

cpp:	ld libc.o crt0.o $(CPP_OBJS)
	ld -o $@ crt0.o $(CPP_OBJS) libc.o

CC_OBJS = pvector.o timeconv.o cc.o cli.o

cc:	ld libc.o crt0.o $(CC_OBJS:%.o=%2.o)
	ld -o $@ crt0.o $(CC_OBJS:%.o=%2.o) libc.o

cmp:	cc cmp.c cli.c
	cc -o cmp cmp.c cli.c

.INTERMEDIATE:  $(CCX0_OBJS:%.o=%0.o) $(CCX1_OBJS:%.o=%1.o) \
                $(CCX1_OBJS:%.o=%.s) $(CCX1_OBJS) $(CPP_OBJS) $(CC_OBJS)

clean:
	$(RM) -f $(INIT) $(XINIT)
	$(RM) -f $(CCX0_OBJS:%.o=%0.s) $(CCX0_OBJS:%.o=%0.o) ccx1
	$(RM) -f $(CCX1_OBJS:%.o=%1.s) $(CCX1_OBJS:%.o=%1.o) ccx
	$(RM) -f $(CPP_OBJS:%.o=%.s) $(CPP_OBJS) cpp
	$(RM) -f $(CCX1_OBJS:%.o=%2.s) $(CCX1_OBJS:%.o=%2.i) 
	$(RM) -f $(CC_OBJS:%.o=%.i) $(CC_OBJS:%.o=%2.s) $(CC_OBJS:%.o=%2.s) cc
	$(RM) -f $(CCX1_OBJS:%.o=%2.o) ccx2 $(CC_OBJS:%.o=%2.o) cc2
	$(RM) -f cmp.o cmp.s cmp.i cmp

check-cmp: ccx2 cc2 cmp
	cmp ccx2 ccx
	cmp cc2 cc
	! cmp -s cc2 ccx2
	$(RM) -f cc2 ccx2

# Build ccx2 and cc2 with a single command.  This is a good test of the 
# driver logic.
ccx2:	cc $(CCX1_OBJS:%.o=%.c)
	cc -o ccx2 --compatibility=4 $(CCX1_OBJS:%.o=%.c)

cc2:	cc ccx2 $(CC_OBJS:%.o=%.c)
	cc -o cc2 --with-ccx=ccx2 $(CC_OBJS:%.o=%.c)

check:	check-cmp
	$(MAKE) -r -C cpp-tests $@

world:
	$(MAKE) clean
	$(MAKE) all
	$(MAKE) check
