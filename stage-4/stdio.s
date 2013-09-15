# stdio.s  --  bootstrap code for I/O

# Copyright (C) 2012, 2013 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

####	#  Function:	void putstr(char* str)
	#
	#  The B library putstr() function.  Writes STR to standard output.
	#  Unlike the C library puts(), no terminating '\n' is added 
	#  automatically.
putstr:
	PUSH	%ebp
	MOVL	%esp, %ebp

	PUSH	8(%ebp)
	CALL	strlen
	POP	%ecx

	PUSH	%eax
	PUSH	%ecx
	MOVL	$1, %eax		# 1 == STDOUT_FILENO
	PUSH	%eax
	CALL	write
	POP	%ecx
	POP	%ecx
	POP	%ecx

	CMPL	%eax, %ecx
	JNE	_error

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

	MOVL	$1, %eax
	PUSH	%eax			# strlen
	LEA	8(%ebp), %eax
	PUSH	%eax			# &chr
	MOVL	$1, %eax		# 1 == STDOUT_FILENO
	PUSH	%eax
	CALL	write
	POP	%ecx
	POP	%ecx
	POP	%ecx

	CMPL	%eax, %ecx
	JNE	_error

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

####	#  Function:	int ungetchar(int c);
	#
	#  A version of the C standard library ungetc() that acts
	#  on standard input.
.data 
.local unget_count 
unget_count:
	.byte	0		# How many characters are in the unget slot?
.local unget_data
unget_data:
	.int	0		# %al -- bool: is slot in use?
				# %ah -- char: slot content
.text ungetchar:
	PUSH	%ebp
	MOVL	%esp, %ebp

	#  If c == EOF, we should do nothing and return EOF.
	MOVL	8(%ebp), %eax	
	CMPL	$-1, %eax
	JE	.L15

	#  Have we space to write another character?
	MOVB	unget_count, %al
	CMPB	$4, %al
	JAE	_error
	INCB	%al
	MOVB	%al, unget_count

	#  Write the character to the unget slot.
	MOVL	unget_data, %eax
	MOVB	$8, %cl
	SHLL	%eax
	MOVB	8(%ebp), %al
	MOVL	%eax, unget_data

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
	MOVB	unget_count, %al
	CMPB	$0, %al
	JE	.L14
	DECB	%al
	MOVB	%al, unget_count

	#  Read the character from the unget slot.
	MOVL	unget_data, %eax
	XORL	%edx, %edx
	MOVB	%al, %dl
	MOVB	$8, %cl
	SHRL	%eax
	MOVL	%eax, unget_data
	MOVL	%edx, %eax
	JMP	.L13

.L14:	#  Read from OS
	XORL	%eax, %eax
	PUSH	%eax			# A 4-byte buffer: %esp points here
	MOVL	%esp, %ecx

	MOVL	$1, %eax
	PUSH	%eax			# strlen
	PUSH	%ecx			# ptr to buffer
	XORL	%eax, %eax
	PUSH	%eax			# 0 == STDOUT_FILENO
	CALL	read
	MOVL	%eax, %edx
	POP	%ecx
	POP	%ecx
	POP	%ecx			# strlen == 1
	POP	%eax			# character read

	CMPL	%edx, %ecx		# Successfully read one byte
	JE	.L13
	CMPL	$0, %edx		# Necessarily indicates end of file
	JNE	_error
	MOVL	$-1, %eax		# -1 == EOF
.L13:
	POP	%ebp
	RET


__io_flush:
	RET

.data:
stdout: .int 1  # These are nominally pointers and need to be distinguishable
stdin:	.int 1  # from the NULL pointer returned on error from e.g. freopen.


.text:
####	#  Function: FILE* freopen( char const* filename, char const* mode,
	#                           FILE* stream );
	#
	#  A minimal freopen, just to get the main() function of the compiler
	#  to work.
freopen:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%ebx

	MOVL	12(%ebp), %eax
	CMPB	'w', (%eax)
	JE	.L18

	# reopen stdin
	XORL	%eax, %eax		# 0 == O_RDONLY
	PUSH	%eax
	PUSH	8(%ebp)
	CALL	open
	ADDL	$8, %esp
	CMPL	$-1, %eax
	JE	_error

	XORL	%ecx, %ecx		# stdin
	JMP	.L19
.L18:
	# reopen stdout
	MOVL	$0644, %eax		# permissions
	PUSH	%eax
	MOVL	$0x241, %eax		# O_WRONLY=1|O_CREAT=0x40|O_TRUNC=0x200
	PUSH	%eax
	PUSH	8(%ebp)
	CALL	open
	ADDL	$12, %esp
	CMPL	$-1, %eax
	JE	_error
	
	XORL	%ecx, %ecx
	INCL	%ecx			# stdout

.L19:
	PUSH	%ecx			# new_fd
	PUSH	%eax			# old_fd
	CALL	dup2
	CMPL	$-1, %eax
	JE	_error
	CALL	close
	POP	%eax
	POP	%eax

	MOVL	16(%ebp), %eax

	POP	%ebx
	POP	%ebp
	RET


####	#  Function: int strtol( char const* str, char const **endptr );
	#
	#  Convert STR to an integer.  This is really the standard atoi,
	#  rather than than strtol.  As the whole file is discarded after
	#  the bootstrap cc0 is created, this is okay.
strtol:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%esi

	XORL	%eax, %eax			# value
	MOVL	8(%ebp), %esi			# ptr

	#  Recurse to process a -ve sign
	CMPB	'-', (%esi)
	JNE	.L16
	INCL	%esi
	PUSH	%esi
	CALL	strtol
	POP	%edx
	NEGL	%eax
	JMP	.L17
.L16:
	XORL	%ecx, %ecx
	MOVB	(%esi), %cl
	SUBL	'0', %ecx
	CMPL	'9', %ecx
	JA	.L17			# unsigned, so everything else is > '9'

	PUSH	%ecx
	MOVL	$10, %ecx
	MULL	%ecx
	POP	%ecx
	ADDL	%ecx, %eax

	INCL	%esi
	JMP	.L16	
.L17:
	MOVL	12(%ebp), %ecx
	MOVL	%esi, (%ecx)
	
	POP	%esi
	POP	%ebp
	RET
