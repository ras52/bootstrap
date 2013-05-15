# char.s  --  functions write/read a character to/from a string

# Copyright (C) 2013 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

# The functions here are needed to work around the lack of a character type
# in the B language.

.text

####	#  Function:  char rchar(char const* s, size_t n);
	#
	#  B's char function, renamed to avoid forward compatibility problems 
	#  with C's keyword.  Returns the byte S[N], zero padded in a word.
.globl rchar
rchar:
	PUSH	%ebp
	MOVL	%esp, %ebp

	XORL	%eax, %eax
	MOVL	8(%ebp), %edx
	ADDL	12(%ebp), %edx
	MOVB	(%edx), %al

	POP	%ebp
	RET

####	#  Function: char lchar(char* s, size_t n, char c);
	#
	#  B's lchar function.  Sets S[N] = C, and returns C, zero padded in 
	#  a word.
.globl lchar
lchar:
	PUSH	%ebp
	MOVL	%esp, %ebp

	XORL	%eax, %eax
	MOVB	16(%ebp), %al
	MOVL	8(%ebp), %edx
	ADDL	12(%ebp), %edx
	MOVB	%al, (%edx)

	POP	%ebp
	RET
	
