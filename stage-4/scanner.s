# scanner.s  --  code to tokenising B input stream

# Copyright (C) 2012, 2013 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

.data 

#  We use TOKEN as an enum for the different token types.
.globl token
token:
	.int	0

#  The VALUE buffer contains tokens as they are being read.
.globl value
value:
	.zero	80


.text

####	#  Function:	void skip_ccomm();
	#
	#  Skips over a C-style comment (the opening /* having been read
	#  already).
.local skip_ccomm
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
.local skip_white
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
.local isidchar1
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
.local isidchar
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
.data 
.local mopchars
mopchars:
	.string	"+-*/<>&|!=%^"
.text 
.local ismopchar
ismopchar:
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
.data 
.local mops2
mops2:
	.int	'++', '--', '<<', '>>', '<=', '>=', '==', '!=', '&&', '||'
	.int	'*=', '%=', '/=', '+=', '-=', '&=', '|=', '^='
	.int	0 	# <-- end of table
	
.text 
.local get_multiop
get_multiop:
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
.local get_word
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

	MOVL	'id', %eax		# 'id' for identifier
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
.data 
.local keywords
.align 12 
keywords:
	.string "auto"		.align 12
	.string "break"		.align 12
	.string "case"		.align 12	# TODO
        .string "continue"      .align 12
	.string "default"	.align 12	# TODO
	.string "else"		.align 12
	.string "extern"	.align 12	# TODO
	.string "goto"		.align 12	# TODO
	.string "if"		.align 12
	.string "return"	.align 12
	.string "switch"	.align 12	# TODO
	.string "while"		.align 12
	.byte  0	# <-- the end of table marker

.text 
.local chk_keyword
chk_keyword:
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
	
	ADDL	$12, %esi
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


####	#  Function:	int get_charlit();
	#
	#  Reads the textual representation of a character literal into VALUE,
	#  including the single quotation marks, and returns the next byte
	#  (having ungot it).
.local get_charlit
get_charlit:
	PUSH	%ebp
	MOVL	%esp, %ebp

	#  Skip whitespace and test for the opening '\''
	CALL	skip_white
	CMPB	'\'', %al
	JNE	_error

	MOVL	'char', %eax		# 'char' for character literal
	MOVL	%eax, token
	MOVL	$value, %eax
	MOVL	%eax, %edi		# string pointer

	CALL	getchar
	MOVB	%al, (%edi)

.L21:	#  Loop reading characters, and check for buffer overflow
	INCL	%edi
	MOVL	$value, %eax
	SUBL	%edi, %eax
	CMPL	$-79, %eax
	JLE	_error

	CALL	getchar
	MOVB	%al, (%edi)
	CMPB	'\'', %al
	JNE	.L21

	#  Write null terminator
	INCL	%edi
	XORB	%cl, %cl
	MOVB	%cl, (%edi)

	#  Peek another character
	CALL	getchar
	PUSH	%eax
	CALL	ungetchar
	POP	%eax

POP	%ebp
	RET


####	#  Function:	int get_strlit();
	#
	#  Reads the textual representation of a string literal into VALUE,
	#  including the quotation marks, and returns the next byte
	#  (having ungot it).
.local get_strlit
get_strlit:
	PUSH	%ebp
	MOVL	%esp, %ebp

	#  Skip whitespace and test for the opening '\"'
	CALL	skip_white
	CMPB	'\"', %al
	JNE	_error

	MOVL	'str', %eax		# 'str' for character literal
	MOVL	%eax, token
	MOVL	$value, %eax
	MOVL	%eax, %edi		# string pointer

	CALL	getchar
	MOVB	%al, (%edi)

.L22:	#  Loop reading characters, and check for buffer overflow
	INCL	%edi
	MOVL	$value, %eax
	SUBL	%edi, %eax
	CMPL	$-79, %eax
	JLE	_error

	CALL	getchar
	MOVB	%al, (%edi)
	CMPB	'\"', %al
	JNE	.L22

	#  Write null terminator
	INCL	%edi
	XORB	%cl, %cl
	MOVB	%cl, (%edi)

	#  Peek another character
	CALL	getchar
	PUSH	%eax
	CALL	ungetchar
	POP	%eax

POP	%ebp
	RET


####	#  Function:	int get_number();
	#
	#  Reads the textual representation of a number into VALUE, 
	#  and returns the next byte (having ungot it).
.local get_number
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

	MOVL	'num', %eax		# 'num' for number
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
	JNZ	.L5

	PUSH	%eax
	CALL	isalpha
	TESTL	%eax, %eax
	JNZ	_error
	POP	%eax

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
	JE	.L6a

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

	CMPB	'\'', %cl
	JE	.L8b
	CMPB	'\"', %cl
	JE	.L8c

	CALL	getchar
.L6a:
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
	JMP	.L9
.L8b:
	CALL	get_charlit
	JMP	.L9
.L8c:
	CALL	get_strlit
	JMP	.L9
.L9:
	MOVL	token, %eax
.L6:
	POP	%ebp
	RET	
