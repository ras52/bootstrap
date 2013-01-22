# char.s

# Copyright (C) 2013 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

# The functions here are needed to work around the lack of a character type
# in the B language.

.text
.globl char

####	#  Function:  char char(char const* s, size_t n);
	#  Returns the byte S[N], zero padded in a word
char:
	PUSH	%ebp
	MOVL	%esp, %ebp

	XORL	%eax, %eax
	MOVL	8(%ebp), %edx
	ADDL	12(%ebp), %edx
	MOVB	(%edx), %al

	POP	%ebp
	RET

####	#  Function: char lchar(char const* s, size_t n, char c);
	#  Sets S[N] = C, and returns C, zero padded in a word
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
	
