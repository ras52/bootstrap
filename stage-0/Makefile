# stage-0/Makefile

# Copyright (C) 2009, 2011, 2015, 2020 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

SHELL  = /bin/sh

CHMOD  = /bin/chmod
RM     = /bin/rm
CP     = /bin/cp
CMP    = /usr/bin/cmp
MAKE   = /usr/bin/make
CAT    = /bin/cat
PRINTF = /usr/bin/printf

BINDIR = ../bin
PATH   = $(BINDIR)

all:	init unhex

init:
	@test -d $(BINDIR) || $(MAKE) -C .. init

unhex:	init unhex.x
	for x in `$(CAT) unhex.x`; do $(PRINTF) \\x$$x; done > unhex
	$(CHMOD) a+x unhex

check:	check-unhex

check-unhex:
	./unhex < unhex.x > unhex2
	$(CMP) unhex unhex2
	$(RM) unhex2

install: unhex
	$(CP) unhex $(BINDIR)

clean:
	$(RM) -f unhex unhex2

world:
	set -e; for TARGET in clean init all check install; do \
	    $(MAKE) $$TARGET; \
	done
