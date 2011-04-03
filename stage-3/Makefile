# stage-2/Makefile

# Copyright (C) 2010, 2011 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

SHELL = /bin/sh
PATH  = .

CHMOD = /bin/chmod
RM    = /bin/rm
CMP   = /usr/bin/cmp
LN_S  = /bin/ln -s
MAKE  = /usr/bin/make

all:	as

init as:	as0 elfify

as0 elfify:
	$(MAKE) -C ../stage-2
	$(LN_S) ../stage-2/elfify
	$(LN_S) ../stage-2/as as0

as:	as.s
	as0 as.s > as.ts
	elfify as.ts > as
	$(CHMOD) a+x as
	$(RM) as.ts

check:	check-as

check-as:	as as.s
	as as.s
	elfify as.o > as2
	$(CHMOD) a+x as2
	as2 as.s
	elfify as.o > as3
	$(CMP) as2 as3
	$(RM) as.o as2 as3

clean:
	$(RM) -f as0 elfify as as.ts as.o as2
