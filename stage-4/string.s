# stdio.s

# Copyright (C) 2012 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

####	#  Function:	size_t strlen(char* s);
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

####	#  Function:	int strcmp(char const* a, char const* b);
strcmp:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%esi
	PUSH	%edi

	#  Note the order is chosen so we set CF correctly.
	MOVL	12(%ebp), %edi
	MOVL	8(%ebp), %esi
	CLD
.L1:
	LODSB			# Loads (%esi) to %al
	SCASB			# Compares (%edi) to %al
	JNE	.L2
	CMPB	$0, %al
	JNE	.L1

	#  They're equal
	XORL	%eax, %eax
	JMP	.L3
.L2:
	#  SCAS internally does SUBL (%edi), %al, so if (%edi) > %al
	#  (or b > a), the carry flag will be set.  SBB %eax, %eax 
	#  is a useful trick for setting %eax to -1 if CF.
	SBBL	%eax, %eax
	ORB	$1, %al
.L3:
	POP	%edi
	POP	%esi
	POP	%ebp
	RET
