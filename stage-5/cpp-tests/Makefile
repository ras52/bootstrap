# stage-5/cpp-tests/Makefile

# Copyright (C) 2015, 2016 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

SHELL = /bin/sh
PATH  = ..

MAKE  = /usr/bin/make

all:	check

# We have a proper test suite for the preprocessor
CPP_TESTS = empty simple directive hash include macros

check:	$(CPP_TESTS:%=%.run)

# Suppress the default rules
.SUFFIXES:

../cpp ../cmp:
	$(MAKE) -C .. $(@:../%=%)

%.run:	%.c %.i ../cpp ../cmp
	cpp -P $< | cmp - $(<:%.c=%.i)

