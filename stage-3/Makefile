# stage-3/Makefile

# Copyright (C) 2011, 2012, 2013 Richard Smith <richard@ex-parrot.com>
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

# as0 is the assemlber symlinked from stage 2.
# as1 is this stage's assembler (written in asm), assembled by as0.
# as  is the same source, assembled by itself (by as1), and therefore similar.
# as2 is a test assembler produced by as; it should be binary identical to as.

# The same conventions apply to ld, except that there is no ld0.

as0 elfify:
	$(MAKE) -C ../stage-2
	$(LN_S) ../stage-2/as as0
	$(LN_S) ../stage-2/elfify

as1:	as.s
	as0 as.s > as.ts
	elfify as.ts > as1
	$(CHMOD) a+x as1
	$(RM) as.ts

ld1:	ld.s
	as0 ld.s > ld.ts
	elfify ld.ts > ld1
	$(CHMOD) a+x ld1
	$(RM) ld.ts

check:	check-as check-ld check-sep

as:	as1 ld1 as.s
	as1 as.s
	ld1 -o as as.o
	$(RM) as.o

ld:	as1 ld1 ld.s
	as1 ld.s
	ld1 -o ld ld.o
	$(RM) ld.o

check-as:	as ld as.s
	$(RM) -f as.o
	as as.s
	ld -o as2 as.o
	$(CMP) as as2
	$(RM) as.o as2

check-ld:	as ld ld.s
	$(RM) -f ld.o
	as ld.s
	ld -o ld2 ld.o
	$(CMP) ld ld2
	$(RM) ld.o ld2

testprog:	as ld test1.s test2.s test3.s
	as test2.s
	as test3.s
	ld -r -o test2+3.o test2.o test3.o
	as test1.s
	ld -o testprog test1.o test2+3.o
	$(RM) test1.o test2.o test3.o test2+3.o

check-sep:	testprog
	testprog
	$(RM) testprog

.INTERMEDIATE:	as.ts ld.ts as.o ld.o as1 ld1

clean:
	$(RM) -f as0 elfify
	$(RM) -f as.ts ld.ts as.o ld.o as1 ld1
	$(RM) -f as2.o ld2.o ld as2 ld2 as ld
	$(RM) -f test1.o test2.o test3.o test2+3.o testprog

world:
	$(MAKE) clean
	$(MAKE) all
	$(MAKE) check
