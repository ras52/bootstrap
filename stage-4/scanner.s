# scanner.s  --  code to tokenising B input stream

# Copyright (C) 2012 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

.data 

#  We use TOKEN as an enum for the different token types.
#
#    { IDENTIFIER = 'I', NUMBER = '0' }
#
token:
	.int	0

#  The VALUE buffer contains tokens as they are being read.
value:
	.zero	80


.text

####	#  Function:	void skip_ccomm();
	#
	#  Skips over a C-style comment (the opening /* having been read
	#  already).
skip_ccomm:
	PUSH	%ebp
	MOVL	%esp, %ebp

.L20:
	CALL	getchar
	CMPL	$-1, %eax
	JE	_error
	CMPB	'*', %al
	JNE	.L20

	CALL	getchar
	CMPL	$-1, %eax
	JE	_error
	CMPB	'/', %al
	JNE	.L20

	POP	%ebp
	RET


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

	#  Handle comments
	CMPB	'/', %al
	JNE	.L18
	PUSH	%eax
	CALL	getchar
	CMPB	'*', %al
	JNE	.L19
	POP	%eax
	CALL	skip_ccomm
	JMP	.L1

.L19:
	PUSH	%eax
	CALL	ungetchar
	POP	%eax
	POP	%eax

.L18:
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

	MOVL	8(%ebp), %ecx
	MOVL	$1, %eax
	CMPB	'_', %cl
	JE	.L2
	CMPB	'.', %cl
	JE	.L2
	PUSH	%ecx
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

	MOVL	8(%ebp), %ecx
	MOVL	$1, %eax
	CMPB	'_', %cl
	JE	.L3
	CMPB	'.', %cl
	JE	.L3
	PUSH	%ecx
	CALL	isalnum
.L3:
	LEAVE
	RET


####	#  Function:	int ismopchar(int chr);
	#
	#  Is CHR a character than can occur at the start of a multi-character
	#  operator?
.data mopchars:
	.string	"+-*/<>&|!=%^"
.text ismopchar:
	PUSH	%ebp
	MOVL	%esp, %ebp

	PUSH	8(%ebp)
	MOVL	$mopchars, %eax
	PUSH	%eax
	CALL	strchr

	LEAVE
	RET

####	#  Function:	void get_multiop();
	#
	#  Reads a multi-character operator.
.data mops2:
	.int	'++', '--', '<<', '>>', '<=', '>=', '==', '!='
	.int	'*=', '%=', '/=', '+=', '-=', '&=', '|=', '^='
	.int	0 	# <-- end of table
	
.text get_multiop:
	PUSH	%ebp
	MOVL	%esp, %ebp

	CALL	getchar
	MOVL	%eax, token

	CALL	getchar
	CMPL	$-1, %eax
	JE	.L13
	PUSH	%eax
	MOVL	token, %eax
	MOVL	%eax, %ecx
	MOVB	-4(%ebp), %ch		# %ecx is now the two-char token

	MOVL	$mops2, %eax
	MOVL	%eax, %edx
.L14:
	#  Loop testing tokens
	CMPL	%ecx, (%edx)
	JE	.L15
	INCL	%edx
	CMPL	$0, (%edx)
	JNE	.L14
	JMP	.L17

.L15:
	#  Definitely got a two-character token.  What about a third?
	POP	%eax
	MOVL	%ecx, %eax
	MOVL	%eax, token
	CMPL	'<<', %ecx
	JE	.L16
	CMPL	'>>', %ecx
	JE	.L16
	JMP	.L13
.L16:
	# Handle <<= and >>=
	CALL	getchar
	CMPL	$-1, %eax
	JE	.L13
	PUSH	%eax
	CMPB	'=', %al
	JNE	.L17
	POP	%edx
	MOVB	$16, %cl
	SALL	%edx
	MOVL	token, %eax
	ORL	%edx, %eax
	MOVL	%eax, token
	JMP	.L13
.L17:
	CALL	ungetchar
	POP	%eax
.L13:
	POP	%ebp
	RET

####	#  Function:	int get_word();
	#
	#  Reads an identifier or keyword (without distinguishing them)
	#  into VALUE, and returns the next byte (having ungot it).
get_word:
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

	MOVL	'I', %eax		# 'I' for identifier
	MOVL	%eax, token
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

	CALL	chk_keyword

	POP	%edi
	POP	%ebp
	RET


####	#  Function:	void chk_keyword();
	#
	#  Check whether VALUE contains a keyword, and if so, sets TOKEN
	#  accordingly.
	#
.data .align 8 keywords:
	.string "auto"		.align 8
	.string "break"		.align 8
	.string "case"		.align 8
	.string "default"	.align 8
	.string "else"		.align 8
	.string "extrn"		.align 8
	.string "goto"		.align 8
	.string "if"		.align 8
	.string "return"	.align 8
	.string "switch"	.align 8
	.string "while"		.align 8
	.byte  0	# <-- the end of table marker

.text chk_keyword:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%edi
	PUSH	%esi

	MOVL	$value, %edi
	MOVL	$keywords, %esi
.L10:
	CMPB	$0, (%esi)
	JE	.L12
	PUSH	%edi
	PUSH	%esi
	CALL	strcmp
	POP	%ecx
	POP	%ecx
	TESTL	%eax, %eax
	JZ	.L11
	
	ADDL	$8, %esi
	JMP	.L10
.L11:
	#  Found it.  Use the first dword of the name to put in TOKEN.
	MOVL	(%esi), %eax
	MOVL	%eax, token

.L12:
	POP	%esi
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

	MOVL	'0', %eax		# '0' for identifier
	MOVL	%eax, token
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
	TESTL	%eax, %eax
	JNZ	.L8

	PUSH	%ecx
	CALL	ismopchar
	POP	%ecx
	TESTL	%eax, %eax
	JNZ	.L8a

	CALL	getchar
	MOVL	%eax, token
	JMP	.L6
	
.L7:
	CALL	get_word
	JMP	.L9
.L8:
	CALL	get_number
	JMP	.L9
.L8a:
	CALL	get_multiop
.L9:
	MOVL	token, %eax
.L6:
	POP	%ebp
	RET	
