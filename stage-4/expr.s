# expr.s  --  expression parser

# Copyright (C) 2012 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

.text

####	#  Function:	int arg_list();
	#
	#    expression ( ',' expression )*
	#
	#  Returns the number of arguments
arg_list:
	PUSH	%ebp
	MOVL	%esp, %ebp
	XORL	%eax, %eax
	PUSH	%eax		# -4(%ebp) is count

	MOVL	token, %eax
	CMPL	')', %eax
	JE	.L41

.L40:
	CALL	expr
	CALL	push
	INCL	-4(%ebp)
	MOVL	token, %eax
	CMPL	',', %eax
	JNE	.L41
	CALL	next
	JMP	.L40
.L41:
	POP	%eax
	POP	%ebp
	RET


####	#  Function: 	void maybe_call();
	#
	#    maybe_call ::= identifier ( '(' params ')' )?
	#
maybe_call:
	PUSH	%ebp
	MOVL	%esp, %ebp
	SUBL	$16, %esp
	
	MOVL	$value, %eax
	PUSH	%eax
	LEA	-16(%ebp), %eax
	PUSH	%eax
	CALL	strcpy
	POP	%eax
	POP	%eax

	CALL	next
	MOVL	token, %eax
	CMPL	'(', %eax
	JNE	.L38
	CALL	next
	CALL	arg_list

	PUSH	%eax
	LEA	-16(%ebp), %eax
	PUSH	%eax
	CALL	call
	POP	%eax
	POP	%eax

	MOVL	token, %eax
	CMPL	')', %eax
	JNE	_error

	CALL	next
	JMP	.L39
.L38:
	LEA	-16(%ebp), %eax
	PUSH	%eax
	CALL	load_var
	POP	%eax
	
.L39:
	LEAVE
	RET


####	#  Function:	void primry_expr();
	#
	#    primry-expr ::= number | identifier | '(' expr ')'
	#
primry_expr:
	PUSH	%ebp	
	MOVL	%esp, %ebp
	
	MOVL	token, %eax
	CMPL	'num', %eax
	JE	.L35
	CMPL	'id', %eax
	JE	.L36
	CMPL	'(', %eax
	JNE	_error

	CALL	next
	CALL	expr
	MOVL	token, %eax
	CMPL	')', %eax
	JNE	_error
	CALL	next
	JMP	.L37

.L35:
	MOVL	$value, %eax
	PUSH	%eax
	CALL	load_const
	POP	%eax
	CALL	next
	JMP	.L37

.L36:
	CALL	maybe_call
	
.L37:
	POP	%ebp
	RET


####	#  Function:	void unary_expr();
	#
	#    unary-op   ::= '+' | '-' | '~' | '!'
	#    unary-expr ::= unary-op unary-expr | primry-expr
	#
unary_expr:
	PUSH	%ebp	
	MOVL	%esp, %ebp
	
	MOVL	token, %eax
	CMPL	'+', %eax
	JE	.L30
	CMPL	'-', %eax
	JE	.L31
	CMPL	'~', %eax
	JE	.L32
	CMPL	'!', %eax
	JE	.L33
	CALL	primry_expr
	JMP	.L34

.L30:
	CALL	next
	CALL	unary_expr
	JMP	.L34

.L31:
	CALL	next
	CALL	unary_expr
	CALL	arith_neg
	JMP	.L34

.L32:
	CALL	next
	CALL	unary_expr
	CALL	bit_not
	JMP	.L34

.L33:
	CALL	next
	CALL	unary_expr
	CALL	logic_not

.L34:
	POP	%ebp
	RET


####	#  Function:	void mult_expr();
	#
	#    mult-op   ::= '*' | '/' | '%'
	#    mult-expr ::= unary-expr ( mult-op unary-expr )*
	#
mult_expr:
	PUSH	%ebp	
	MOVL	%esp, %ebp

	CALL	unary_expr
.L1:
	MOVL	token, %eax
	CMPL	'*', %eax
	JE	.L2
	CMPL	'/', %eax
	JE	.L3
	CMPL	'%', %eax
	JE	.L3a
	JMP	.L4

.L2:
	CALL	next
	CALL	push
	CALL	unary_expr
	CALL	pop_mult
	JMP	.L1

.L3:
	CALL	next
	CALL	push
	CALL	unary_expr
	CALL	pop_div
	JMP	.L1

.L3a:
	CALL	next
	CALL	push
	CALL	unary_expr
	CALL	pop_mod
	JMP	.L1

.L4:
	POP	%ebp
	RET


####	#  Function:	void add_expr();
	#
	#    add-op   ::= '+' | '-' 
	#    add-expr ::= mult-expr ( add-op mult-expr )*
	#
add_expr:
	PUSH	%ebp	
	MOVL	%esp, %ebp

	CALL	mult_expr
.L5:
	MOVL	token, %eax
	CMPL	'+', %eax
	JE	.L6
	CMPL	'-', %eax
	JE	.L7
	JMP	.L8

.L6:
	CALL	next
	CALL	push
	CALL	mult_expr
	CALL	pop_add
	JMP	.L5

.L7:
	CALL	next
	CALL	push
	CALL	mult_expr
	CALL	pop_sub
	JMP	.L5

.L8:
	POP	%ebp
	RET


####	#  Function:	void shift_expr();
	#
	#    shift-op   ::= '<<' | '>>' 
	#    shift-expr ::= add-expr ( shift-op add-expr )*
	#
shift_expr:
	PUSH	%ebp	
	MOVL	%esp, %ebp

	CALL	add_expr
.L5a:
	MOVL	token, %eax
	CMPL	'<<', %eax
	JE	.L6a
	CMPL	'>>', %eax
	JE	.L7a
	JMP	.L8a

.L6a:
	CALL	next
	CALL	push
	CALL	add_expr
	CALL	pop_lshift
	JMP	.L5a

.L7a:
	CALL	next
	CALL	push
	CALL	add_expr
	CALL	pop_rshift
	JMP	.L5a

.L8a:
	POP	%ebp
	RET


####	#  Function:	void rel_expr();
	#
	#    rel-op   ::= '<' | '<=' | '>' | '>='
	#    rel-expr ::= shift_expr ( rel-op shift_expr )*
	#
rel_expr:
	PUSH	%ebp	
	MOVL	%esp, %ebp

	CALL	shift_expr
.L9:
	MOVL	token, %eax
	CMPL	'<', %eax
	JE	.L10
	CMPL	'<=', %eax
	JE	.L11
	CMPL	'>', %eax
	JE	.L12
	CMPL	'>=', %eax
	JE	.L13
	JMP	.L14

.L10:
	CALL	next
	CALL	push
	CALL	shift_expr
	CALL	pop_lt
	JMP	.L9

.L11:
	CALL	next
	CALL	push
	CALL	shift_expr
	CALL	pop_le
	JMP	.L9

.L12:
	CALL	next
	CALL	push
	CALL	shift_expr
	CALL	pop_gt
	JMP	.L9

.L13:
	CALL	next
	CALL	push
	CALL	shift_expr
	CALL	pop_ge
	JMP	.L9

.L14:
	POP	%ebp
	RET


####	#  Function:	void eq_expr();
	#
	#    eq-op   ::= '==' | '!=' 
	#    eq-expr ::= rel-expr ( eq-op rel-expr )*
	#
eq_expr:
	PUSH	%ebp	
	MOVL	%esp, %ebp

	CALL	rel_expr
.L15:
	MOVL	token, %eax
	CMPL	'==', %eax
	JE	.L16
	CMPL	'!=', %eax
	JE	.L17
	JMP	.L18

.L16:
	CALL	next
	CALL	push
	CALL	rel_expr
	CALL	pop_eq
	JMP	.L15

.L17:
	CALL	next
	CALL	push
	CALL	rel_expr
	CALL	pop_ne
	JMP	.L15

.L18:
	POP	%ebp
	RET


####	#  Function:	void bitand_expr();
	#
	#    bitand-expr ::= eq-expr ( '&' eq-expr )*
	#
bitand_expr:
	PUSH	%ebp	
	MOVL	%esp, %ebp

	CALL	eq_expr
.L19:
	MOVL	token, %eax
	CMPL	'&', %eax
	JNE	.L20
	CALL	next
	CALL	push
	CALL	eq_expr
	CALL	pop_bitand
	JMP	.L19

.L20:

	POP	%ebp
	RET


####	#  Function:	void bitxor_expr();
	#
	#    bitxor-expr ::= bitand-expr ( '^' bitand-expr )*
	#
bitxor_expr:
	PUSH	%ebp	
	MOVL	%esp, %ebp

	CALL	bitand_expr
.L21:
	MOVL	token, %eax
	CMPL	'^', %eax
	JNE	.L22
	CALL	next
	CALL	push
	CALL	bitand_expr
	CALL	pop_bitxor
	JMP	.L21

.L22:

	POP	%ebp
	RET


####	#  Function:	void bitor_expr();
	#
	#    bitor-expr ::= bitxor-expr ( '|' bitxor-expr )*
	#
bitor_expr:
	PUSH	%ebp	
	MOVL	%esp, %ebp

	CALL	bitxor_expr
.L23:
	MOVL	token, %eax
	CMPL	'|', %eax
	JNE	.L24
	CALL	next
	CALL	push
	CALL	bitxor_expr
	CALL	pop_bitor
	JMP	.L23

.L24:

	POP	%ebp
	RET


####	#  Function:	void logand_expr();
	#
	#    logand-expr ::= bitor-expr ( '&&' bitor-expr )*
	#
logand_expr:
	PUSH	%ebp	
	MOVL	%esp, %ebp

	CALL	bitor_expr
	MOVL	token, %eax
	CMPL	'&&', %eax
	JNE	.L26
.L25:
	CALL	new_label
	PUSH	%eax		# JMP -4(%ebp) if true
	CALL	branch_ifz
	CALL	next
	CALL	bitor_expr
	CALL	local_label
	POP	%eax

	MOVL	token, %eax
	CMPL	'&&', %eax
	JE	.L25
	CALL	cast_bool
.L26:

	POP	%ebp
	RET


####	#  Function:	void logor_expr();
	#
	#    logor-expr ::= logand-expr ( '||' logand-expr )*
	#
logor_expr:
	PUSH	%ebp	
	MOVL	%esp, %ebp

	CALL	logand_expr
	MOVL	token, %eax
	CMPL	'||', %eax
	JNE	.L28

.L27:
	CALL	new_label
	PUSH	%eax		# JMP -4(%ebp) if true
	CALL	branch_ifnz
	CALL	next
	CALL	logand_expr
	CALL	local_label
	POP	%eax

	MOVL	token, %eax
	CMPL	'||', %eax
	JE	.L27
	CALL	cast_bool
.L28:

	POP	%ebp
	RET


####	#  Function:	int new_label();
	#  Return the next local label id.
.data label_count:
	.int	0
.text new_label:
	PUSH	%ebp	
	MOVL	%esp, %ebp
	MOVL	label_count, %eax
	INCL	%eax
	MOVL	%eax, label_count
        POP     %ebp
        RET	


####	#  Function:	void cond_expr();
	#
	#    cond-expr ::= logor-expr ( '?' expr ':' cond-expr )?
	#
cond_expr:
	PUSH	%ebp
	MOVL	%esp, %ebp

	CALL	logor_expr

	MOVL	token, %eax
	CMPL	'?', %eax
	JNE	.L29

	CALL	new_label
	PUSH	%eax		# JMP -4(%ebp) if false
	CALL	new_label
	PUSH	%eax		# JMP -8(%ebp) to end

	PUSH	-4(%ebp)
	CALL	branch_ifz
	POP	%eax
	CALL	next
	CALL	expr
	PUSH	-8(%ebp)
	CALL	branch
	POP	%eax

	MOVL	token, %eax
	CMPL	':', %eax
	JNE	_error

	PUSH	-4(%ebp)
	CALL	local_label
	POP	%eax
	CALL	next
	CALL	cond_expr
	PUSH	-8(%ebp)
	CALL	local_label
	POP	%eax

.L29:
	LEAVE
	RET


####	#  Function:	void expr();
	#
	#  Process an expression.
expr:
	PUSH	%ebp	
	MOVL	%esp, %ebp

	CALL	cond_expr

	POP	%ebp
	RET
