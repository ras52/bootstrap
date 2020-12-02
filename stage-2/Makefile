# stage-2/Makefile

# Copyright (C) 2010, 2011, 2020 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

SHELL  = /bin/sh

CHMOD  = /bin/chmod
RM     = /bin/rm
CP     = /bin/cp
CMP    = /usr/bin/cmp
MAKE   = /usr/bin/make

BINDIR = ../bin
PATH   = $(BINDIR)

all:	init as

init:
	@test -d $(BINDIR) || $(MAKE) -C .. init
	@test -x $(BINDIR)/unhexl || $(MAKE) -C ../stage-1 install

as:	as.ts.xl
	unhexl < as.ts.xl > as.ts
	elfify as.ts > as
	$(CHMOD) a+x as
	$(RM) as.ts

check:	check-as

check-as:	as as.s
	./as as.s > as2.ts
	elfify as2.ts > as2
	$(CHMOD) a+x as2
	./as2 as.s > as3.ts
	$(CMP) as2.ts as3.ts
	$(RM) as2.ts as3.ts as2

install: as
	$(CP) as $(BINDIR)/as0
	$(RM) -f $(BINDIR)/unhexl

clean:
	$(RM) -f as as.ts as2.ts as3.ts as2

world:
	set -e; for TARGET in clean init all check install; do \
	    $(MAKE) $$TARGET; \
	done
