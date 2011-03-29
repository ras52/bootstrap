# Makefile

# Copyright (C) 2009, 2011 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

STAGES = 2

all:
	set -e; for n in `seq 0 $(STAGES)`; do $(MAKE) -C stage-$$n all; done

check:
	set -e; for n in `seq 0 $(STAGES)`; do $(MAKE) -C stage-$$n check; done

clean:
	for n in `seq 0 $(STAGES)`; do $(MAKE) -C stage-$$n clean; done


