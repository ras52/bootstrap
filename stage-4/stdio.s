# stdio.s

# Copyright (C) 2012 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

####	#  Function:	void putstr(char* str)
	#
	#  The B library putstr() function.  Writes STR to standard output.
	#  Unlike the C library puts(), no terminating '\n' is added 
	#  automatically.
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


####	#  Function:	void putchar(int chr)
	#
	#  The C standard library putchar() function.  Writes the one
	#  characters in CHR to standard output.  The B library version
	#  should write multiple characters from CHR (up to four): this
	#  is prohibited by the C standard, and we do not currently do it.
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

####	#  Function:	void printf(char* fmt, ...);
	#
	#  A very light-weight version of printf, handling just the 
	#  %%, %c, %d and %s format specifiers, with no widths or 
	#  precisions.  The B library version also support %o which we
	#  don't do yet.
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
	CMPB	'%', %al
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
	CMPB	'%', %al
	JE	.L6			# write a literal '%'

	#  Read the next vararg into %eax, putting the format char in %cl
	ADDL	$4, %edi		# read next va_arg
	MOVB	%al, %cl
	MOVL	(%edi), %eax

	#  Test for 'c', 's' and 'd' format specifiers, otherwise fail  
	CMPB	'c', %cl
	JE	.L6			# write the va_arg character
	CMPB	's', %cl
	JE	.L7
	CMPB	'd', %cl
	JE	.L8
	JMP	_error

.L7:	#  Handle the %s format specifier
	PUSH	%eax
	CALL	putstr
	POP	%eax
	JMP	.L5

.L8:	#  Handle the %d format specifier.  Special case 0
	TESTL	%eax, %eax
	JNZ	.L9
	MOVB	'0', %al
	JMP	.L6

.L9:	#  Do we need a -ve sign?
	CMPL	$0, %eax
	JG	.L10
	PUSH	%eax
	MOVB	'-', %cl
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
	ADDB	'0', %dl		# remainder is in %dl, conv. to char
	MOVB	%dl, (%ebx)
	TESTL	%eax, %eax
	JZ	.L12
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

.data

unget_slot:
	.int	0		# %al -- bool: is slot in use?
				# %ah -- char: slot content

.text

####	#  Function:	int ungetchar(int c);
	#
	#  A version of the C standard library ungetc() that acts
	#  on standard input.
ungetchar:
	PUSH	%ebp
	MOVL	%esp, %ebp

	#  If c == EOF, we should do nothing and return EOF.
	MOVL	8(%ebp), %eax	
	CMPL	$-1, %eax
	JE	.L15

	#  Write the character to the unget slot.
	MOVL	unget_slot, %eax
	CMPB	$0, %al
	JNE	_error
	MOVB	8(%ebp), %ah
	INCB	%al
	MOVL	%eax, unget_slot

	#  And return the character
	XORL	%eax, %eax
	MOVB	8(%ebp), %al
.L15:
	POP	%ebp
	RET


####	#  Function:	int getchar(void);
	#
	#  The C standard library getchar() function.  Reads one character
	#  from standard input and returns it.  If end-of-file occurs, we
	#  return -1: in this respect it differs from the B library 
	#  version which returns the ASCII EOT character (0x04, ^D).
getchar:
	PUSH	%ebp
	MOVL	%esp, %ebp

	#  Has a character been ungetc'd?
	MOVL	unget_slot, %eax
	CMPB	$0, %al
	JE	.L14

	XORL	%ecx, %ecx
	MOVB	%ah, %cl
	XORB	%al, %al
	MOVL	%eax, unget_slot
	MOVL	%ecx, %eax
	JMP	.L13

.L14:	#  Read from OS
	PUSH	%ebx
	XORL	%eax, %eax
	PUSH	%eax			# A 4-byte buffer: %esp points here
	MOVL	$1, %edx
	MOVL	%esp, %ecx
	MOVL	$0, %ebx		# 0 == STDOUT_FILENO
	MOVL	$3, %eax		# 3 == __NR_read
	INT	$0x80
	MOVL	%eax, %ecx
	POP	%eax
	POP	%ebx
	CMPL	$1, %ecx		# Successfully read one byte
	JE	.L13
	CMPL	$0, %ecx		# Necessarily indicates end of file
	JNE	_error
	MOVL	$-1, %eax		# -1 == EOF
.L13:
	POP	%ebp
	RET

