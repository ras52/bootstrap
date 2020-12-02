# stage-1/Makefile

# Copyright (C) 2009, 2011, 2020 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

SHELL  = /bin/sh

CHMOD  = /bin/chmod
RM     = /bin/rm
CP     = /bin/cp
CMP    = /usr/bin/cmp
MAKE   = /usr/bin/make

BINDIR = ../bin
PATH   = $(BINDIR)


all:	init unhexl elfify

init:
	@test -d $(BINDIR) || $(MAKE) -C .. init
	@test -x $(BINDIR)/unhex || $(MAKE) -C ../stage-0 install

elfify:	elfify.x
	unhex < elfify.x > elfify
	$(CHMOD) a+x elfify

unhexl:	elfify unhexl.ts.x
	unhex < unhexl.ts.x > unhexl.ts
	./elfify unhexl.ts > unhexl
	$(CHMOD) a+x unhexl
	$(RM) unhexl.ts

check:	check-unhexl

check-unhexl:	unhexl elfify unhexl.ts.xl
	./unhexl < unhexl.ts.xl > unhexl.ts
	./elfify unhexl.ts > unhexl2
	$(CHMOD) a+x unhexl2
	./unhexl2 < unhexl.ts.xl > unhexl2.ts
	$(CMP) unhexl.ts unhexl2.ts
	./elfify unhexl2.ts > unhexl3
	$(CMP) unhexl2 unhexl3
	$(RM) unhexl.ts unhexl2.ts unhexl2 unhexl3

install: unhexl elfify
	$(CP) unhexl elfify $(BINDIR)
	$(RM) -f $(BINDIR)/unhex

clean:
	$(RM) -f unhexl elfify unhexl.ts unhexl2.ts unhexl2 unhexl3

world:
	set -e; for TARGET in clean init all check install; do \
	    $(MAKE) $$TARGET; \
	done
