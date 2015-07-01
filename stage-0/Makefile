# stage-0/Makefile

# Copyright (C) 2009, 2011, 2015 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

SHELL = /bin/sh
PATH  = .

CHMOD = /bin/chmod
RM    = /bin/rm
CMP   = /usr/bin/cmp
MAKE  = /usr/bin/make
CAT   = /bin/cat
PRINTF= /usr/bin/printf

all:	unhex

init:

unhex:	unhex.x
	for x in `$(CAT) unhex.x`; do $(PRINTF) \\x$$x; done > unhex
	$(CHMOD) a+x unhex

check:	check-unhex

check-unhex:
	unhex < unhex.x > unhex2
	$(CMP) unhex unhex2
	$(RM) unhex2

clean:
	$(RM) -f unhex unhex2

world:
	$(MAKE) clean
	$(MAKE) all
	$(MAKE) check
