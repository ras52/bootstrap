# stage-2/Makefile

# Copyright (C) 2010, 2011 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

SHELL = /bin/sh
PATH  = .

CHMOD = /bin/chmod
RM    = /bin/rm -f
CMP   = /usr/bin/cmp
LN_S  = /bin/ln -sf
MAKE  = /usr/bin/make

all:	as

init as:	unhexl elfify

unhexl elfify:
	$(MAKE) -C ../stage-1
	for f in unhexl elfify; do $(LN_S) ../stage-1/$$f; done

as:	as.ts.xl
	unhexl < as.ts.xl > as.ts
	elfify as.ts > as
	$(CHMOD) a+x as
	$(RM) as.ts

check:	check-as

check-as:	as as.s
	as as.s > as2.ts
	elfify as2.ts > as2
	$(CHMOD) a+x as2
	as2 as.s > as3.ts
	$(CMP) as2.ts as3.ts
	$(RM) as2.ts as3.ts as2

clean:
	$(RM) -f unhexl elfify as as.ts as2.ts as3.ts as2
