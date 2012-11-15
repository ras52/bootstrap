# stage-3/Makefile

# Copyright (C) 2010, 2011, 2012 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

SHELL = /bin/sh
PATH  = .

CHMOD = /bin/chmod
RM    = /bin/rm
CMP   = /usr/bin/cmp
LN_S  = /bin/ln -sf
MAKE  = /usr/bin/make

all:	as ld

init as ld:	as0 elfify

as0 elfify:
	$(MAKE) -C ../stage-2
	$(LN_S) ../stage-2/as as0
	$(LN_S) ../stage-2/elfify

as:	as.s
	as0 as.s > as.ts
	elfify as.ts > as
	$(CHMOD) a+x as
	$(RM) as.ts

ld:	ld.s
	as0 ld.s > ld.ts
	elfify ld.ts > ld
	$(CHMOD) a+x ld
	$(RM) ld.ts

check:	check-as check-ld check-sep
	$(RM) as2 ld2

as2:	as ld as.s
	as as.s
	ld -o as2 as.o
	$(RM) as.o

ld2:	as ld ld.s
	as ld.s
	ld -o ld2 ld.o
	$(RM) ld.o

check-as:	as2 ld2 as.s
	as2 as.s
	ld2 -o as3 as.o
	$(CMP) as2 as3
	$(RM) as.o as3

check-ld:	as2 ld2 ld.s
	as2 ld.s
	ld2 -o ld3 ld.o
	$(CMP) ld2 ld3
	$(RM) ld.o ld3

testprog:	as2 ld2 test1.s test2.s test3.s
	as2 test2.s
	as2 test3.s
	ld2 -r -o test2+3.o test2.o test3.o
	as2 test1.s
	ld2 -o testprog test1.o test2+3.o
	$(RM) test1.o test2.o test3.o test2+3.o

check-sep:	testprog
	testprog
	$(RM) testprog

clean:
	$(RM) -f as0 elfify as ld as.ts ld.ts as.o ld.o as2 ld2 as3 ld3
	$(RM) -f test1.o test2.o testprog

world:
	$(MAKE) clean
	$(MAKE) all
	$(MAKE) check
