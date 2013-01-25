# stdioobjs.s

# Copyright (C) 2013 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

.data

#  We have two sets of objects here.  In C, they would be
#    FILE __stdout = { ... };
#    FILE* stdout = &__stdout;
#  At present the compiler isn't smart enough to handle this.

#  These are the output buffers themselves
.LC0a:	.zero	128
.LC1a:	.zero	128
.LC2a:	.zero	128

#  These are the FILE structs.
#  bufend is ignored for output streams
#		fd	bufsz	bufp	buffer	bufend
.LC0:	.int	0,	128,	.LC0a,	.LC0a,	.LC0a
.LC1:	.int	1,	128,	.LC1a,	.LC1a,	.LC1a
.LC2:	.int	2,	128,	.LC2a,	.LC2a,	.LC2a


.globl stdin
stdin:
	.int	.LC0

.globl stdout
stdout:
	.int	.LC1

.globl stderr
stderr:
	.int	.LC2
