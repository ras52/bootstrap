# stmt.s  --  statement parser

# Copyright (C) 2012 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

.text

####	#  Function:	void semicolon();
	#
	#  Require the current token to be ';' and call next()
semicolon:
	PUSH	%ebp
	MOVL	%esp, %ebp

	MOVL	token, %eax
	CMPL	';', %eax
	JNE	_error
	CALL	next
	
	POP	%ebp
	RET


####	#  Function:	void brack_expr();
	#
	#    brack-expr ::= '(' expr ')'
	#
	#  Current token is '('.
brack_expr:
	PUSH	%ebp
	MOVL	%esp, %ebp

	MOVL	token, %eax
	CMPL	'(', %eax
	JNE	_error
	CALL	next
	CALL	rvalue_expr
	MOVL	token, %eax
	CMPL	')', %eax
	JNE	_error
	CALL	next

	POP	%ebp
	RET


#  We pass around (int brk, int cont, int ret) for the labels to jump to 
#  to execute a break, a continue, and a return.

####	#  Function:	void if_stmt(int brk, int cont, int ret);
	#
	#    if-stmt ::= 'if' '(' expr ')' stmt
	# 
	#  Current token is 'if'
if_stmt:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%ebx

	CALL	next
	CALL	brack_expr
	CALL	new_label
	PUSH	%eax
	CALL	branch_ifz

	PUSH	16(%ebp)
	PUSH	12(%ebp)
	PUSH	8(%ebp)
	CALL	stmt
	ADDL	$12, %esp

	MOVL	token, %eax
	CMPL	'else', %eax
	JNE	.L7
	CALL	next

	CALL	new_label
	PUSH	%eax
	CALL	branch
	POP	%ebx
	CALL	local_label
	POP	%eax
	PUSH	%ebx

	PUSH	16(%ebp)
	PUSH	12(%ebp)
	PUSH	8(%ebp)
	CALL	stmt
	ADDL	$12, %esp
.L7:
	CALL	local_label
	POP	%eax
	
	POP	%ebx
	POP	%ebp
	RET


####	#  Function:	void while_stmt(int brk, int cont, int ret);
	#
	#    while-stmt ::= 'while' '(' expr ')' stmt
	#
	#  Current token is 'while'
while_stmt:
	PUSH	%ebp
	MOVL	%esp, %ebp

	CALL	new_label
	PUSH	%eax
	CALL	local_label

	CALL	next
	CALL	brack_expr

	CALL	new_label
	PUSH	%eax
	CALL	branch_ifz

	PUSH	16(%ebp)	# ret 
	PUSH	-4(%ebp)	# cont
	PUSH	-8(%ebp)	# brk
	CALL	stmt
	ADDL	$12, %esp

	PUSH	-4(%ebp)
	CALL	branch
	POP	%eax
	CALL	local_label

	LEAVE
	RET


####	#  Function:	void return_stmt(int brk, int cont, int ret);
	#
	#    return-stmt ::= 'return' expr? ';'
return_stmt:
	PUSH	%ebp
	MOVL	%esp, %ebp

	CALL	next
	CMPL	';', %eax
	JE	.L10

	CALL	rvalue_expr
.L10:
	CALL	semicolon
	PUSH	16(%ebp)
	CALL	branch

	LEAVE
	RET


####	#  Function:	void break_stmt(int brk, int cont, int ret);
	#
	#    return-stmt ::= 'break' ';'
break_stmt:
	PUSH	%ebp
	MOVL	%esp, %ebp

	CALL	next
	CALL	semicolon
	PUSH	8(%ebp)
	CALL	branch

	LEAVE
	RET


####	#  Function:	void cont_stmt(int brk, int cont, int ret);
	#
	#    return-stmt ::= 'continue' ';'
cont_stmt:
	PUSH	%ebp
	MOVL	%esp, %ebp

	CALL	next
	CALL	semicolon
	PUSH	12(%ebp)
	CALL	branch

	LEAVE
	RET


####	#  Function:	void auto_stmt(int brk, int cont, int ret);
	#
	#    init-decl ::= name ( '=' assign-expr )?
	#    auto-stmt ::= 'auto' init-decl ( ',' init-decl )* ';'
	#
	#  Current token is 'auto'
auto_stmt:
	PUSH	%ebp
	MOVL	%esp, %ebp

.L15:
	CALL	next
	CMPL	'id', %eax
	JNE	_error

	MOVL	$frame_size, %eax
	SUBL	$4, (%eax)
	PUSH	(%eax)
	MOVL	$value, %eax
	PUSH	%eax
	CALL	save_sym
	POP	%ecx
	POP	%ecx

	CALL	next
	CMPL	'=', %eax
	JNE	.L14
	CALL	next
	CALL	assign_expr	
.L14:
	CALL	push
	MOVL	token, %eax
	CMPL	',', %eax
	JE	.L15
	CALL	semicolon

	POP	%ebp
	RET	


####	#  Function:	void expr_stmt(int brk, int cont, int ret);
	#
	#    expr-stmt ::= expr ';'
	#
	#  NB. Null statements are handled in stmt
expr_stmt:
	PUSH	%ebp
	MOVL	%esp, %ebp

	CALL	rvalue_expr
	MOVL	token, %eax
	CMPL	';', %eax
	JNE	_error
	CALL	next

	POP	%ebp
	RET


####	#  Function:	void stmt(int brk, int cont, int ret);
	#
	#  Process a statement.  Current token is first token of statement.
stmt:
	PUSH	%ebp	
	MOVL	%esp, %ebp

	PUSH	16(%ebp)
	PUSH	12(%ebp)
	PUSH	8(%ebp)

	MOVL	token, %eax
	CMPL	'if', %eax
	JE	.L3
	CMPL	'whil', %eax
	JE	.L8
	CMPL	'retu', %eax
	JE	.L9
	CMPL	'brea', %eax
	JE	.L11
	CMPL	'cont', %eax
	JE	.L12
	CMPL	'auto', %eax
	JE	.L13
	CMPL	';', %eax
	JE	.L4
	CMPL	'{', %eax
	JE	.L5
	CALL	expr_stmt
	JMP	.L6
.L3:
	CALL	if_stmt
	JMP	.L6
.L8:
	CALL	while_stmt
	JMP	.L6
.L9:
	CALL	return_stmt
	JMP	.L6
.L11:
	CALL	break_stmt
	JMP	.L6
.L12:
	CALL	cont_stmt
	JMP	.L6
.L13:
	CALL	auto_stmt
	JMP	.L6

.L4:	# The null statement
	CALL	next
	JMP	.L6
.L5:	
	CALL	block
.L6:

	LEAVE
	RET


####	#  Function:	void block(int brk, int cont, int ret);
	#
	#  Process a block.  Current token is '{'.
block:
	PUSH	%ebp	
	MOVL	%esp, %ebp

	CALL	new_scope
	MOVL	token, %eax
	CMPL	'{', %eax
	JNE	_error
	CALL	next

	PUSH	16(%ebp)
	PUSH	12(%ebp)
	PUSH	8(%ebp)
.L1:
	MOVL	token, %eax
	CMPL	'}', %eax
	JE	.L2
	CALL	stmt	
	JMP	.L1
.L2:
	CALL	end_scope
	PUSH	%eax
	CALL	clear_stack
	CALL	next

	LEAVE
	RET
