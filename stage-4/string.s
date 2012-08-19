# stdio.s

# Copyright (C) 2012 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

####	#  Function: size_t strlen(char* s)
strlen:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%edi

	MOVL	8(%ebp), %edi
	XORL	%eax, %eax
	XORL	%ecx, %ecx
	DECL	%ecx
	REPNE SCASB
	SUBL	8(%ebp), %edi
	DECL	%edi
	MOVL	%edi, %eax

	POP	%edi
	POP	%ebp
	RET
