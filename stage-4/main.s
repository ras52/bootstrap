# main.s  --  entry point 

# Copyright (C) 2012 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

####	#  Function:	void int_decl(char* name);
	#
	#  Process an integer declaration for NAME.  The name has been read, 
	#  and TOKEN advanced to the next token: either '=' or ';'
.data f_int_decl:
	.string "\n.data\n%s:\n\t.int %s\n"
.text int_decl:
	PUSH	%ebp	
	MOVL	%esp, %ebp

	MOVL	'0', %eax
	PUSH	%eax
	MOVL	token, %eax
	CMPL	'=', %eax
	JNE	.L3

	#  Read an initialiser
	CALL	next
	CMPL	'num', %eax
	JNE	_error

	#  Because the next token is punctuation, value is not set.
	CALL	next
	MOVL	$value, %eax
	PUSH	%eax
	JMP	.L4
.L3:
	PUSH	%esp
.L4:
	PUSH	8(%ebp)
	MOVL	$f_int_decl, %eax
	PUSH	%eax
	CALL	printf
	POP	%eax
	POP	%eax
	POP	%eax

	MOVL	token, %eax
	CMPL	';', %eax
	JNE	_error
	CALL	next

	LEAVE
	RET


####	#  Function:	void int_decl(char* name);
	#
	#  Process a function declaration.  Current token is '('.
func_decl:
	PUSH	%ebp	
	MOVL	%esp, %ebp

	CALL	next
	CMPL	')', %eax
	JE	.L5
	MOVL	$8, %ecx
	PUSH	%ecx

.L5a:
	CMPL	'id', %eax
	JNE	_error

	MOVL	$value, %eax
	PUSH	%eax
	CALL	save_sym
	POP	%eax
	CALL	next

	ADDL	$4, -4(%ebp)
	CMPL	',', %eax
	JNE	.L5b
	CALL	next
	JMP	.L5a
	
.L5b:
	CMPL	')', %eax
	JNE	_error
	POP	%eax
.L5:
	PUSH	8(%ebp)
	CALL	prolog
	POP	%eax

	CALL	new_label	# For return
	PUSH	%eax
	XORL	%eax, %eax
	PUSH	%eax
	PUSH	%eax

	CALL	next
	CALL	block
	POP	%eax
	POP	%eax
	CALL	local_label
	POP	%eax

	CALL	epilog
	CALL	clr_symtab

	LEAVE
	RET

####	#  Function:	void ext_decl();
	#
	#  Process an external declaration.
	#
	#    extern-decl ::=   name ( '=' number )? ';' 
	#                    | name '(' ')' '{' '}'
	#  
	#  When called, TOKEN should be the name.
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
	JE	.L6
	CALL	int_decl
	JMP	.L7
.L6:	
	CALL	func_decl
.L7:
	LEAVE
	RET


main:
	PUSH	%ebp
	MOVL	%esp, %ebp

	CALL	init_symtab
	CALL	next

.L1:
	MOVL	token, %eax
	CMPL	$-1, %eax
	JE	.L2

	CMPL	'id', %eax
	JNE	_error
	CALL	ext_decl
	JMP	.L1

.L2:
	XORL	%eax, %eax
	LEAVE
	RET
