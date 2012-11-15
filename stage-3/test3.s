# stage-3/test3.s

# Copyright (C) 2012 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

.text
square:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	8(%ebp), %eax
	MOVL	%eax, %ecx
	MULL	%ecx
	LEAVE
	RET
