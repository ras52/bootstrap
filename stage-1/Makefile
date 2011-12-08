# stage-1/Makefile

# Copyright (C) 2009, 2011 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

SHELL = /bin/sh
PATH  = .

CHMOD = /bin/chmod
RM    = /bin/rm
CMP   = /usr/bin/cmp
LN_S  = /bin/ln -sf
MAKE  = /usr/bin/make

all:	unhexl elfify 

init unhexl elfify: unhex

unhex:
	$(MAKE) -C ../stage-0
	$(LN_S) ../stage-0/unhex

elfify:	unhex elfify.x
	unhex < elfify.x > elfify
	$(CHMOD) a+x elfify

unhexl:	unhex elfify unhexl.ts.x
	unhex < unhexl.ts.x > unhexl.ts
	elfify unhexl.ts > unhexl
	$(CHMOD) a+x unhexl
	$(RM) unhexl.ts

check:	check-unhexl

check-unhexl:	unhexl elfify unhexl.ts.xl
	unhexl < unhexl.ts.xl > unhexl.ts
	elfify unhexl.ts > unhexl2
	$(CHMOD) a+x unhexl2
	unhexl2 < unhexl.ts.xl > unhexl2.ts
	$(CMP) unhexl.ts unhexl2.ts
	$(RM) unhexl.ts unhexl2.ts unhexl2

clean:
	$(RM) -f unhex unhexl elfify unhexl.ts unhexl2.ts
