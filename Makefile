# Makefile

# Copyright (C) 2009, 2011, 2012, 2013, 2020 
# Richard Smith <richard@ex-parrot.com>
# All rights reserved.

STAGES = 0 1 2 3 4 5

SHELL  = /bin/sh
PATH   = .

RM     = /bin/rm
MKDIR  = /bin/mkdir
MAKE   = /usr/bin/make

BINDIR = bin
LIBDIR = lib
INCDIR = include

world:
	$(RM) -rf $(BINDIR) $(LIBDIR) $(INCDIR)
	$(MAKE) init
	set -e; for n in $(STAGES); do $(MAKE) -r -C stage-$$n $@; done

init:
	$(MKDIR) -p $(BINDIR) $(LIBDIR) $(INCDIR)

check:
	set -e; for n in $(STAGES); do $(MAKE) -r -C stage-$$n $@; done

clean:
	set -e; for n in $(STAGES); do $(MAKE) -r -C stage-$$n $@; done
	$(RM) -rf $(BINDIR) $(LIBDIR) $(INCDIR)

