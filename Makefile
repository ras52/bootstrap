# Makefile

# Copyright (C) 2009, 2011 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

STAGES = 3

all init check:
	set -e; for n in `seq 0 $(STAGES)`; do $(MAKE) -C stage-$$n $@; done

clean:
	for n in `seq $(STAGES) -1 0`; do $(MAKE) -C stage-$$n $@; done

world:
	$(MAKE) clean
	$(MAKE) all
	$(MAKE) check
