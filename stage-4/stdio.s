# stdio.s

# Copyright (C) 2012 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

####	#  Function: void putstr(char* s)
putstr:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%ebx

	PUSH	8(%ebp)
	CALL	strlen
	POP	%ecx

	MOVL	%eax, %edx
	MOVL	8(%ebp), %ecx
	MOVL	$1, %ebx		# 1 == STDOUT_FILENO
	MOVL	$4, %eax		# 4 == __NR_write
	INT	$0x80
	CMPL	$-4096, %eax		# -4095 <= %eax < 0 for errno
	JA	_error			# unsigned comparison handles above

	POP	%ebx
	POP	%ebp
	RET


####	#  Function: void putchar(char c)
putchar:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%ebx

	MOVL	$1, %edx
	LEA	8(%ebp), %ecx
	MOVL	$1, %ebx		# 1 == STDOUT_FILENO
	MOVL	$4, %eax		# 4 == __NR_write
	INT	$0x80
	CMPL	$-4096, %eax		# -4095 <= %eax < 0 for errno
	JA	_error			# unsigned comparison handles above

	POP	%ebx
	POP	%ebp
	RET

####	#  Function: void printf(char* fmt, ...);
	#  This is a *very* light-weight printf, handling just %%, %c, %d, %s
printf:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%ebx
	PUSH	%esi
	PUSH	%edi
	
	MOVL	8(%ebp), %esi		# %esi is ptr into fmt
	LEA	8(%ebp), %edi		# %edi is ptr to prev va_arg

.L2:	#  Loop over the format string
	CMPB	$0, (%esi)
	JE	.L3
	MOVB	(%esi), %al
	CMPB	$0x25, %al		# '%'
	JE	.L4

.L6:	#  Write a raw character
	PUSH	%eax
	CALL	putchar
	POP	%eax
.L5:
	INCL	%esi
	JMP	.L2

.L4:
	#  We have a format specifier
	INCL	%esi
	CMPB	$0, (%esi)
	JE	_error
	MOVB	(%esi), %al
	CMPB	$0x25, %al		# '%'
	JE	.L6			# write a literal '%'

	#  Read the next vararg into %eax, putting the format char in %cl
	ADDL	$4, %edi		# read next va_arg
	MOVB	%al, %cl
	MOVL	(%edi), %eax

	#  Test for 'c', 's' and 'd' format specifiers, otherwise fail  
	CMPB	$0x63, %cl		# 'c'
	JE	.L6			# write the va_arg character
	CMPB	$0x73, %cl		# 's'
	JE	.L7
	CMPB	$0x64, %cl		# 'd'
	JE	.L8
	JMP	_error

.L7:	#  Handle the %s format specifier
	PUSH	%eax
	CALL	putstr
	POP	%eax
	JMP	.L5

.L8:	#  Handle the %d format specifier.  Special case 0
	CMPL	$0, %eax
	JNE	.L9
	MOVB	$0x30, %al		# '0'
	JMP	.L6

.L9:	#  Do we need a -ve sign?
	CMPL	$0, %eax
	JG	.L10
	PUSH	%eax
	MOVB	$0x2D, %cl		# '-'
	PUSH	%ecx
	CALL	putchar
	POP	%ecx
	POP	%eax
	NEGL	%eax

.L10:	#  Set up a temporary buffer, as we'll write from right to left
	MOVL	%esp, %ebx
	DECL	%ebx
	SUBL	$16, %esp
	MOVB	$0, (%ebx)		# '\0' terminator
	DECL	%ebx
	MOVL	$10, %ecx

.L11:
	XORL	%edx, %edx
	IDIVL	%ecx			# acts on %edx:%eax
	ADDB	$0x30, %dl		# remainder is in %dl, conv. to char
	MOVB	%dl, (%ebx)
	CMPL	$0, %eax
	JE	.L12
	DECL	%ebx
	JMP	.L11

.L12:
	PUSH	%ebx
	CALL	putstr
	POP	%ebx
	ADDL	$16, %esp
	JMP	.L5

.L3:	#  Cleanup
	POP	%edi
	POP	%esi
	POP	%ebx
	POP	%ebp
	RET
