# stage-3/test1.s

# Copyright (C) 2012, 2013 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

.data
.local bar
bar:
	.int	0x2A

.text
_start:
	MOVL	%esp, %ebp

	#  bar = square(bar)
	MOVL	bar, %eax
	PUSH	%eax
	CALL	square
	POP	%ecx
	MOVL	%eax, bar

	MOVL	bar, %eax
	MOVL	%eax, %ecx
	MOVL	foo, %eax
	CMPL	%eax, %ecx
	SETNE	%al
	MOVZBL	%al, %eax
	MOVL	%eax, foo

	#  Call exit(foo)
	MOVL	foo, %eax
	PUSH	%eax

# It should be safe to embed this literal in the middle of this function
.data
	.string	"\"Hello,\tworld\!\""
	.byte	0xFF

.text
	CALL	exit
	HLT

.data
