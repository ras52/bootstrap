# Makefile

# Copyright (C) 2009, 2011, 2012 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

STAGES = 4

all init check clean world:
	set -e; for n in `seq 0 $(STAGES)`; do $(MAKE) -r -C stage-$$n $@; done
