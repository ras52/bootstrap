# scanner.s  --  code to tokenising B input stream

# Copyright (C) 2012 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

.data 

#  We use TOKEN as an enum for the different token types.
#
#    { IDENTIFIER = 'I' }
#
token:
	.byte	0

#  The VALUE buffer contains tokens as they are being read.
value:
	.zero	80

lineno:
	.int	1



.text

####	#  Function:	int skip_white();
	#
	#  Skips over any white space characters, and returns the next
	#  character (having ungot it).
skip_white:
	PUSH	%ebp
	MOVL	%esp, %ebp

.L1:
	CALL	getchar
	PUSH	%eax
	CALL	isspace
	TESTL	%eax, %eax
	POP	%eax
	JNZ	.L1

	PUSH	%eax
	CALL	ungetchar
	POP	%eax

	POP	%ebp
	RET


####	#  Function:	int isidchar1(int chr);
	#
	#  Test whether CHR can start an identifier.
isidchar1:
	PUSH	%ebp
	MOVL	%esp, %ebp

	MOVL	$1, %eax
	CMPB	$0x5F, 8(%ebp)		# '_'
	JE	.L2
	CMPB	$0x2E, 8(%ebp)		# '.'
	JE	.L2
	PUSH	8(%ebp)
	CALL	isalpha
.L2:
	LEAVE
	RET


####	#  Function:	int isidchar(int chr);
	#
	#  Test whether CHR can occur in an identifier, other than as
	#  the first character.
isidchar:
	PUSH	%ebp
	MOVL	%esp, %ebp

	MOVL	$1, %eax
	CMPB	$0x5F, 8(%ebp)		# '_'
	JE	.L3
	CMPB	$0x2E, 8(%ebp)		# '.'
	JE	.L3
	PUSH	8(%ebp)
	CALL	isalnum
.L3:
	LEAVE
	RET


####	#  Function:	int get_ident();
	#
	#  Reads an identifier or keyword (without distinguishing them)
	#  into VALUE, and returns the next byte (having ungot it).
get_ident:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%edi

	#  Skip whitespace and test for an identifier
	CALL	skip_white
	PUSH	%eax
	CALL	isidchar1
	POP	%ecx
	TESTL	%eax, %eax
	JZ	_error

	MOVB	$0x49,	%al		# 'I' for identifier
	MOVB	%al, token
	MOVL	$value, %eax
	MOVL	%eax, %edi		# string pointer
	DECL	%edi

.L4:	#  Loop reading characters, and check for buffer overflow
	INCL	%edi
	MOVL	$value, %eax
	SUBL	%edi, %eax
	CMPL	$-79, %eax
	JLE	_error

	CALL	getchar
	MOVB	%al, (%edi)
	PUSH	%eax
	CALL	isidchar
	TESTL	%eax, %eax
	POP	%eax
	JNE	.L4

	#  Unget the last character
	PUSH	%eax
	CALL	ungetchar
	POP	%eax

	#  Write null terminator
	XORB	%cl, %cl
	MOVB	%cl, (%edi)

	POP	%edi
	POP	%ebp
	RET


####	#  Function:	int get_number();
	#
	#  Reads the textual representation of a number into VALUE, 
	#  and returns the next byte (having ungot it).
get_number:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%edi

	#  Skip whitespace and test for an identifier
	CALL	skip_white
	PUSH	%eax
	CALL	isdigit
	POP	%ecx
	TESTL	%eax, %eax
	JZ	_error

	MOVB	$0x30,	%al		# '0' for identifier
	MOVB	%al, token
	MOVL	$value, %eax
	MOVL	%eax, %edi		# string pointer
	DECL	%edi

.L5:	#  Loop reading characters, and check for buffer overflow
	INCL	%edi
	MOVL	$value, %eax
	SUBL	%edi, %eax
	CMPL	$-79, %eax
	JLE	_error

	CALL	getchar
	MOVB	%al, (%edi)
	PUSH	%eax
	CALL	isdigit
	TESTL	%eax, %eax
	POP	%eax
	JNE	.L5

	#  Unget the last character
	PUSH	%eax
	CALL	ungetchar
	POP	%eax

	#  Write null terminator
	XORB	%cl, %cl
	MOVB	%cl, (%edi)

	POP	%edi
	POP	%ebp
	RET


####	#  Function:	int next();
	#
	#  Reads the next token, returning the token type (or -1 for EOF)
next:
	PUSH	%ebp
	MOVL	%esp, %ebp

	CALL	skip_white
	CMPL	$-1, %eax
	JE	.L6

	PUSH	%eax
	CALL	isidchar1
	POP	%ecx
	TESTL	%eax, %eax
	JNZ	.L7

	PUSH	%ecx
	CALL	isdigit
	POP	%ecx
	JNZ	.L8
	JMP	_error
	
.L7:
	CALL	get_ident
	JMP	.L9
.L8:
	CALL	get_number
.L9:
	XORL	%eax, %eax
	MOVB	token, %al

.L6:
	POP	%ebp
	RET	
