# stage-3/test3.s

# Copyright (C) 2012, 2013 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

.text
.global square
square:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	8(%ebp), %eax
	MOVL	%eax, %ecx
	MULL	%ecx
	LEAVE
	RET

.data 
.globl foo
foo:
	.int	1764	# == 0x2A * 0x2A

