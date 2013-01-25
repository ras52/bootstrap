# stdio.s

# Copyright (C) 2012, 2013 Richard Smith <richard@ex-parrot.com>
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
	LEA	-1(%edi), %eax		# DEC %edi; MOVL %edi, %eax

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


####	#  Function:	int strchr(char const* a, int c);
	#
strchr:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%esi

	MOVL	12(%ebp), %ecx
	MOVL	8(%ebp), %esi
.L4:
	LODSB
	CMPB	%cl, %al
	JE	.L5
	CMPB	$0, %al
	JNE	.L4

	#  Not found	
	XORL	%eax, %eax
	JMP	.L6
	
.L5:	# Found it.  Note LODSB will have incremented %esi
	LEA	-1(%esi), %eax		# DEC %esi; MOVL %esi, %eax
.L6:
	POP	%esi
	POP	%ebp
	RET


####	#  Function:	int strcpy(char* dest, char const* str);
	#
strcpy:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%esi
	PUSH	%edi

	MOVL	12(%ebp), %esi
	MOVL	8(%ebp), %edi
.L7:
	LODSB
	STOSB
	CMPB	$0, %al
	JNE	.L7
  
	MOVL	8(%ebp), %eax

	POP	%edi
	POP	%esi
	POP	%ebp
	RET


####	#  Function:	int memcpy(char* dest, char const* str, size_t n);
	#
memcpy:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%esi
	PUSH	%edi

	MOVL	16(%ebp), %ecx
	MOVL	12(%ebp), %esi
	MOVL	8(%ebp), %edi

	REP MOVSB
  
	MOVL	8(%ebp), %eax

	POP	%edi
	POP	%esi
	POP	%ebp
	RET
