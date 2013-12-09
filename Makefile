# Makefile

# Copyright (C) 2009, 2011, 2012, 2013 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

STAGES = 0 1 2 3 4 5

SHELL = /bin/sh
PATH  = .

MAKE  = /usr/bin/make

all init check clean world:
	set -e; for n in $(STAGES); do $(MAKE) -r -C stage-$$n $@; done
