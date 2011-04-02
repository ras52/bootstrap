# stage-0/Makefile

# Copyright (C) 2009, 2011 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

SHELL = /bin/sh
PATH  = .

CHMOD = /bin/chmod
RM    = /bin/rm
CMP   = /usr/bin/cmp

all:

init:

check:	check-unhex

check-unhex:
	$(CHMOD) a+x unhex
	unhex < unhex.x > unhex2
	$(CMP) unhex unhex2
	$(RM) unhex2

clean:
	$(RM) -f unhex2
