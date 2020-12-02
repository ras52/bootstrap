# stage-5/cpp-tests/Makefile

# Copyright (C) 2015, 2016, 2018, 2020 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

SHELL = /bin/sh
PATH  = ..

MAKE  = /usr/bin/make

all:	check

# We have a proper test suite for the preprocessor
CPP_TESTS = empty nocpp obj builtin fn simple suppress directive hash \
            rescan macros include includemacro

check:	$(CPP_TESTS:%=%.run)

# Suppress the default rules
.SUFFIXES:

../cpp ../cmp:
	$(MAKE) -C .. $(@:../%=%)

# The exit status of a pipeline is the status of the rightmost command.
# That means that if the preprocessor exits with non-zero status (e.g.
# because it aborts or segfaults) after it has written everything we 
# expect, the error is ignored and the cmp succeeds.  In bash we'd fix
# this with set -o pipefail, but that doesn't exist in more primitive
# shells.  The 2>&1 ensures that the error message is diverted to the cmp
# where it breaks the comparison forcing a test failure.
%.run:	%.c %.i ../cpp ../cmp
	cpp -P $< 2>&1 | cmp - $(<:%.c=%.i)

