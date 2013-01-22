# stdioobjs.s

# Copyright (C) 2013 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

.data

#  We have two sets of objects here.  In C, they would be
#    FILE __stdout = { ... };
#    FILE* stdout = &__stdout;
#  At present the compiler isn't smart enough to handle this.

#  These are the output buffers themselves
.LC0:	.zero	128
.LC1:	.zero	128

#  These are the FILE structs
#		fd	bufsz	bufp	buffer
.LC2:	.int	1,	128,	.LC0,	.LC0
.LC3:	.int	2,	128,	.LC1,	.LC1


.globl stdout
stdout:
	.int	.LC2

.globl stderr
stderr:
	.int	.LC3
