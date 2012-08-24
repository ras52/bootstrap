.data 
f_declname:
	.string "%s:\n"
fmt:	
	.string "%c (%d): '%s'\n"


####	#  Function:	void int_decl(char* name);
	#
	#  Process an integer declaration for NAME.  The initialiser has
	#  been read.
.data f_int_decl:
	.string ".data\n%s:\n\t.int %s\n"
.text int_decl:
	PUSH	%ebp	
	MOVL	%esp, %ebp

	MOVL	'0', %eax
	PUSH	%eax

	MOVL	token, %eax
	CMPL	'=', %eax
	JNE	.L3
	CALL	next
	CMPL	'num', %eax
	JNE	_error

	MOVL	$value, %eax
	PUSH	%eax
	JMP	.L4
.L3:
	PUSH	%esp
.L4:
	PUSH	8(%ebp)
	MOVL	$f_int_dec, %eax
	PUSH	%eax
	CALL	printf
	POP	%eax
	POP	%eax
	POP	%eax

	CALL	next
	CMPL	';', %eax
	JNE	_error

	LEAVE
	RET


####	#  Function:	void ext_decl();
	#
	#  Process an external declaration
	#
	#    extern-decl ::=   name '=' ival? ';' 
	#                    | name '[' constant ']' ival-list ';'
	#                    | name '(' name-list ')' statement
ext_decl:
	PUSH	%ebp	
	MOVL	%esp, %ebp

	#  Check that we've read a identifier first
	MOVL	token, %eax
	CMPL	'id', %eax
	JNE	_error

	#  Store a copy of the name.  (We can't emit the label yet as we
	#  don't yet know which section it belongs in.)
	SUBL	$16, %esp
	MOVL	$value, %eax
	PUSH	%eax
	CALL	strlen
	CMPL	$11, %eax
	JG	_error
	LEA	-16(%ebp), %eax
	PUSH	%eax
	CALL	strcpy
	POP	%ecx
	POP	%eax
	PUSH	%ecx			# Pointer to name
	
	#  Get the next token and dispatch based on it.
	CALL	next
	CMPL	'(', %eax
	JE	_error			# function are unimplemented
	CMPL	'[', %eax
	JE	_error			# arrays are unimplemented

	
	CALL	int_decl

	LEAVE
	RET


main:
	PUSH	%ebp
	MOVL	%esp, %ebp

.L1:
	CALL	next
	CMPL	$-1, %eax
	JE	.L2

	CMPL	'id', %eax
	JNE	_error
	CALL	ext_decl
	JMP	.L1

	MOVL	$value, %eax
	PUSH	%eax
	MOVL	token, %eax
	PUSH	%eax
	PUSH	%eax
	MOVL	$fmt, %eax
	PUSH	%eax
	CALL	printf
	POP	%eax
	POP	%eax
	POP	%eax
	POP	%eax
	JMP	.L1

.L2:
	XORL	%eax, %eax
	LEAVE
	RET
