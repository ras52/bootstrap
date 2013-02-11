# stmt.s  --  statement parser

# Copyright (C) 2012, 2013 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

.text

####	#  Function:	void semicolon();
	#
	#  Require the current token to be ';' and call next()
.local semicolon
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
.local brack_expr
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
	#    if-stmt ::= 'if' brack-expr stmt ( 'else' stmt )?
	# 
	#  Current token is 'if'
.local if_stmt
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


####	#  Function:	void do_stmt(int brk, int cont, int ret);
	#
	#    do-stmt ::= 'do' stmt 'while' brack-expr ';'
	#
	#  Current token is 'do'
.local do_stmt
do_stmt:
	PUSH	%ebp
	MOVL	%esp, %ebp

	CALL	new_label
	PUSH	%eax			# -4(%ebp)  break label, used later

	#  Start of loop
	CALL	new_label
	PUSH	%eax			# -8(%ebp)  start of loop label
	CALL	local_label

	CALL	new_label
	PUSH	%eax			# -12(%ebp) cont label, used later

	CALL	next
	PUSH	16(%ebp)		# ret 
	PUSH	-12(%ebp)		# cont
	PUSH	-4(%ebp)		# brk
	CALL	stmt
	ADDL	$12, %esp

	MOVL	token, %eax
	CMPL	'whil', %eax
	JNE	_error
	CALL	next

	#  End of loop
	CALL	local_label		# cont label
	POP	%eax			# -12(%ebp)
	CALL	brack_expr
	CALL	branch_ifnz		# jump to start
	POP	%eax			# -8(%ebp)
	CALL	local_label		# break label
	POP	%eax			# -4(%ebp)

	CALL	semicolon
	POP	%ebp
	RET

####	#  Function:	void while_stmt(int brk, int cont, int ret);
	#
	#    while-stmt ::= 'while' brack-expr stmt
	#
	#  Current token is 'while'
.local while_stmt
while_stmt:
	PUSH	%ebp
	MOVL	%esp, %ebp

	#  Start of loop
	CALL	new_label
	PUSH	%eax			# -4(%ebp)	cont label
	CALL	local_label

	CALL	next
	CALL	brack_expr

	CALL	new_label
	PUSH	%eax			# -8(%ebp)  break label, used later
	CALL	branch_ifz

	PUSH	16(%ebp)		# ret 
	PUSH	-4(%ebp)		# cont
	PUSH	-8(%ebp)		# brk
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
	#
	#  Current token is 'return'
.local return_stmt
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
	#    break-stmt ::= 'break' ';'
	#
	#  Current token is 'break'
.local break_stmt
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
	#    cont-stmt ::= 'continue' ';'
	#
	#  Current token is 'continue'
.local cont_stmt
cont_stmt:
	PUSH	%ebp
	MOVL	%esp, %ebp

	CALL	next
	CALL	semicolon
	PUSH	12(%ebp)
	CALL	branch

	LEAVE
	RET


####	#  Function:	void init_array(int dim, char const* var);
	#
	#    init-list  ::= assign-expr ( ',' assign-expr )*
	#
	#    init-array ::= ( '=' '{' init-list '}' )?
	#
	#  Current token is '=' if an initialiser is present.
.local init_array
init_array:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%esi

	MOVL	8(%ebp), %edx
	MOVB	$2, %cl
	SHLL	%edx
	PUSH	%edx
	XORL	%eax, %eax
	PUSH	%eax			# arrays are rvalues
	MOVL	$frame_size, %eax
	SUBL	%edx, (%eax)
	PUSH	(%eax)
	PUSH	12(%ebp)
	CALL	save_sym
	POP	%ecx
	POP	%ecx
	POP	%ecx

	#  size is still on stack
	CALL	alloc_stack
	POP	%edx			# size

	MOVL	token, %eax
	CMPL	'=', %eax
	JNE	.L18

	#  We have an initialisation list.
	CALL	next
	CMPL	'{', %eax
	JNE	_error
	MOVL	frame_size, %eax
	MOVL	%eax, %esi

.L19:
	CALL	next
	CALL	assign_expr	
	PUSH	%esi
	CALL	stk_assign
	POP	%eax
	ADDL	$4, %esi

	MOVL	token, %eax
	CMPL	',', %eax
	JE	.L19
	CMPL	'}', %eax
	JNE	_error
	CALL	next

.L18:
	POP	%esi
	POP	%ebp
	RET


####	#  Function:	void init_int(char const* var);
	#
	#    init-int ::= ( '=' assign-expr )?
	#
	#  Current token is '=' if an initialiser is present.
.local init_int
init_int:
	PUSH	%ebp
	MOVL	%esp, %ebp

	MOVL	$4, %eax
	PUSH	%eax			# sizeof(int)
	MOVL	$1, %eax
	PUSH	%eax			# auto ints are lvalues
	MOVL	$frame_size, %eax
	SUBL	$4, (%eax)
	PUSH	(%eax)
	PUSH	8(%ebp)
	CALL	save_sym
	POP	%ecx
	POP	%ecx
	POP	%ecx
	POP	%ecx

	MOVL	token, %eax
	CMPL	'=', %eax
	JNE	.L14
	CALL	next
	CALL	assign_expr	
	PUSH	%eax
	CALL	make_rvalue
	POP	%eax
.L14:
	CALL	push
	POP	%ebp
	RET


####	#  Function:	void auto_stmt(int brk, int cont, int ret);
	#
	#    init-decl ::= name ( init-int | '[' number ']' init-array )
	#
	#    auto-stmt ::= 'auto' init-decl ( ',' init-decl )* ';'
	#
	#  Current token is 'auto'
.local auto_stmt
auto_stmt:
	PUSH	%ebp
	MOVL	%esp, %ebp

	SUBL	$16, %esp		# temporary buffer
	LEA	-16(%ebp), %eax
	PUSH	%eax			# pointer to buffer
.L15:
	CALL	next
	CMPL	'id', %eax
	JNE	_error

	#  Copy the identifier into the temporary buffer
	MOVL	$value, %eax
	PUSH	%eax
	LEA	-16(%ebp), %eax
	PUSH	%eax
	CALL	strcpy
	POP	%eax
	POP	%eax

	CALL	next
	CMPL	'[', %eax
	JNE	.L16

	#  It's an array, and we require an array size
	CALL	next
	CMPL	'num', %eax
	JNE	_error

	PUSH	%eax			# endptr slot
	MOVL	%esp, %ecx
	
	XORL	%eax, %eax
	PUSH	%eax			# guess base
	PUSH	%ecx			# &endptr
	MOVL	$value, %eax
	PUSH	%eax
	CALL	strtol
	ADDL	$12, %esp
	POP	%ecx
	PUSH	%eax			# store size

	#  %ecx contains the end ptr
	MOVL	(%ecx), %ecx
	CMPB	$0, %cl
	JNE	_error

	CALL	next
	CMPL	']', %eax
	JNE	_error
	CALL	next
	CALL	init_array
	POP	%eax

	JMP	.L17

.L16:
	CALL	init_int

.L17:
	MOVL	token, %eax
	CMPL	',', %eax
	JE	.L15
	CALL	semicolon

	LEAVE
	RET	


####	#  Function:	void expr_stmt(int brk, int cont, int ret);
	#
	#    expr-stmt ::= expr ';'
	#
	#  NB. Null statements are handled in stmt
.local expr_stmt
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
	#    stmt ::= if-stmt | while-stmt | return-stmt | break-stmt
	#           | cont-stmt | auto-stmt | expr-stmt
	#
	#  Process a statement.  Current token is first token of statement.
.local stmt
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
	CMPL	'do', %eax
	JE	.L8a
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
.L8a:
	CALL	do_stmt
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
	#    block ::= '{' stmt* '}'
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
	MOVL	$frame_size, %eax
	POP	%ecx
	ADDL	%ecx, (%eax)

	LEAVE
	RET
