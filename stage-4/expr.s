# expr.s  --  expression parser

# Copyright (C) 2012, 2013 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

# Many of the functions here return a typedef int type_t.
# Currently this is simply a boolean recording whether the expression is
# an lvalue (1) or not (0).  If the expression is an lvalue, then the
# accumulator contains the address.


.text

####	#  Function:	int arg_list();
	#
	#    expression ( ',' expression )*
	#
	#  Returns the number of arguments
.local arg_list
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


####	#  Function: 	type_t maybe_call();
	#
	#    maybe_call ::= identifier ( '(' params ')' )?
	#
.local maybe_call
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
	XORL	%eax, %eax		# rvalue
	JMP	.L39
.L38:
	LEA	-16(%ebp), %eax
	PUSH	%eax
	CALL	lookup_sym
	TESTL	%eax, %eax
	JZ	.L38a
	POP	%ecx
	PUSH	%eax
	CALL	load_local
	POP	%ecx
	JMP	.L39
.L38a:
	CALL	load_var
	POP	%eax
	XORL	%eax, %eax
	INCL	%eax			# lvalue
	
.L39:
	LEAVE
	RET


####	#  Function:	type_t primry_expr();
	#
	#    primry-expr ::= number | identifier | '(' expr ')'
	#
.local primry_expr
primry_expr:
	PUSH	%ebp	
	MOVL	%esp, %ebp
	
	MOVL	token, %eax
	CMPL	'num', %eax
	JE	.L35
	CMPL	'char', %eax
	JE	.L35a
	CMPL	'str', %eax
	JE	.L35b
	CMPL	'id', %eax
	JE	.L36
	CMPL	'(', %eax
	JNE	_error

	CALL	next
	CALL	expr
	PUSH	%eax			# store type_t
	MOVL	token, %eax
	CMPL	')', %eax
	JNE	_error
	CALL	next
	POP	%eax			# type_t
	JMP	.L37
.L35:
	MOVL	$value, %eax
	PUSH	%eax
	CALL	load_const
	POP	%eax
	JMP	.L35c

.L35a:
	MOVL	$value, %eax
	PUSH	%eax
	CALL	load_chrlit
	POP	%eax
	JMP	.L35c

.L35b:
	MOVL	$value, %eax
	PUSH	%eax
	CALL	load_strlit
	POP	%eax

.L35c:
	CALL	next
	XORL	%eax, %eax
	JMP	.L37

.L36:
	CALL	maybe_call
	
.L37:
	POP	%ebp
	RET


####	#  Function:	void make_rvalue(type_t type);
	#
	#  Convert the accumulator (which is of type TYPE) into an rvalue
.local make_rvalue
make_rvalue:
	PUSH	%ebp	
	MOVL	%esp, %ebp

	MOVL	8(%ebp), %eax
	TESTL	%eax, %eax
	JZ	.L42
	CALL	dereference
.L42:		
	POP	%ebp
	RET


####	#  Function:	type_t unary_expr();
	#
	#    unary-op   ::= '+' | '-' | '~' | '!' | '*' | '&' | '++' | '--'
	#    unary-expr ::= unary-op unary-expr | primry-expr
	#
.local unary_expr
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
	CMPL	'*', %eax
	JE	.L44
	CMPL	'&', %eax
	JE	.L45
	CMPL	'++', %eax
	JE	.L46
	CMPL	'--', %eax
	JE	.L47

	CALL	primry_expr
	JMP	.L34

.L30:	# +
	CALL	next
	CALL	unary_expr
	PUSH	%eax
	CALL	make_rvalue
	JMP	.L34a

.L31:	# -
	CALL	next
	CALL	unary_expr
	PUSH	%eax
	CALL	make_rvalue
	CALL	arith_neg
	JMP	.L34a

.L32:	# ~
	CALL	next
	CALL	unary_expr
	PUSH	%eax
	CALL	make_rvalue
	CALL	bit_not
	JMP	.L34a

.L33:	# !
	CALL	next
	CALL	unary_expr
	PUSH	%eax
	CALL	make_rvalue
	CALL	logic_not
	JMP	.L34a

.L44:	# *
	CALL	next
	CALL	unary_expr
	PUSH	%eax
	CALL	make_rvalue
	XORL	%eax, %eax
	INCL	%eax		# an lvalue
	JMP	.L34

.L45:	# & -- check for lvalue flag, clear it, and return
	CALL	next
	CALL	unary_expr
	TESTL	%eax, %eax
	JZ	_error
	JMP	.L34a

.L46:	# ++
	CALL	next
	CALL	unary_expr
	TESTL	%eax, %eax
	JZ	_error
	PUSH	%eax
	CALL	increment
	CALL	make_rvalue
	JMP	.L34a

.L47:	# --	
	CALL	next
	CALL	unary_expr
	TESTL	%eax, %eax
	JZ	_error
	PUSH	%eax
	CALL	decrement
	CALL	make_rvalue
.L34a:
	XORL	%eax, %eax
.L34:
	LEAVE
	RET


####	#  Function:	type_t mult_expr();
	#
	#    mult-op   ::= '*' | '/' | '%'
	#    mult-expr ::= unary-expr ( mult-op unary-expr )*
	#
.local mult_expr
mult_expr:
	PUSH	%ebp	
	MOVL	%esp, %ebp

	CALL	unary_expr
	PUSH	%eax
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
	CALL	make_rvalue
	CALL	next
	CALL	push
	CALL	unary_expr
	PUSH	%eax
	CALL	make_rvalue
	POP	%eax
	CALL	pop_mult
	MOVL	$0, -4(%ebp)
	JMP	.L1

.L3:
	CALL	make_rvalue
	CALL	next
	CALL	push
	CALL	unary_expr
	PUSH	%eax
	CALL	make_rvalue
	POP	%eax
	CALL	pop_div
	MOVL	$0, -4(%ebp)
	JMP	.L1

.L3a:
	CALL	make_rvalue
	CALL	next
	CALL	push
	CALL	unary_expr
	PUSH	%eax
	CALL	make_rvalue
	POP	%eax
	CALL	pop_mod
	MOVL	$0, -4(%ebp)
	JMP	.L1

.L4:
	POP	%eax
	POP	%ebp
	RET


####	#  Function:	type_t add_expr();
	#
	#    add-op   ::= '+' | '-' 
	#    add-expr ::= mult-expr ( add-op mult-expr )*
	#
.local add_expr
add_expr:
	PUSH	%ebp	
	MOVL	%esp, %ebp

	CALL	mult_expr
	PUSH	%eax
.L5:
	MOVL	token, %eax
	CMPL	'+', %eax
	JE	.L6
	CMPL	'-', %eax
	JE	.L7
	JMP	.L8

.L6:
	CALL	make_rvalue
	CALL	next
	CALL	push
	CALL	mult_expr
	PUSH	%eax
	CALL	make_rvalue
	POP	%eax
	CALL	pop_add
	MOVL	$0, -4(%ebp)
	JMP	.L5

.L7:
	CALL	make_rvalue
	CALL	next
	CALL	push
	CALL	mult_expr
	PUSH	%eax
	CALL	make_rvalue
	POP	%eax
	CALL	pop_sub
	MOVL	$0, -4(%ebp)
	JMP	.L5

.L8:
	POP	%eax
	POP	%ebp
	RET


####	#  Function:	type_t shift_expr();
	#
	#    shift-op   ::= '<<' | '>>' 
	#    shift-expr ::= add-expr ( shift-op add-expr )*
	#
.local shift_expr
shift_expr:
	PUSH	%ebp	
	MOVL	%esp, %ebp

	CALL	add_expr
	PUSH	%eax
.L5a:
	MOVL	token, %eax
	CMPL	'<<', %eax
	JE	.L6a
	CMPL	'>>', %eax
	JE	.L7a
	JMP	.L8a

.L6a:
	CALL	make_rvalue
	CALL	next
	CALL	push
	CALL	add_expr
	PUSH	%eax
	CALL	make_rvalue
	POP	%eax
	CALL	pop_lshift
	MOVL	$0, -4(%ebp)
	JMP	.L5a

.L7a:
	CALL	make_rvalue
	CALL	next
	CALL	push
	CALL	add_expr
	PUSH	%eax
	CALL	make_rvalue
	POP	%eax
	CALL	pop_rshift
	MOVL	$0, -4(%ebp)
	JMP	.L5a

.L8a:
	POP	%eax
	POP	%ebp
	RET


####	#  Function:	type_t rel_expr();
	#
	#    rel-op   ::= '<' | '<=' | '>' | '>='
	#    rel-expr ::= shift_expr ( rel-op shift_expr )*
	#
.local rel_expr
rel_expr:
	PUSH	%ebp	
	MOVL	%esp, %ebp

	CALL	shift_expr
	PUSH	%eax
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
	CALL	make_rvalue
	CALL	next
	CALL	push
	CALL	shift_expr
	PUSH	%eax
	CALL	make_rvalue
	POP	%eax
	CALL	pop_lt
	MOVL	$0, -4(%ebp)
	JMP	.L9

.L11:
	CALL	make_rvalue
	CALL	next
	CALL	push
	CALL	shift_expr
	PUSH	%eax
	CALL	make_rvalue
	POP	%eax
	CALL	pop_le
	MOVL	$0, -4(%ebp)
	JMP	.L9

.L12:
	CALL	make_rvalue
	CALL	next
	CALL	push
	CALL	shift_expr
	PUSH	%eax
	CALL	make_rvalue
	POP	%eax
	CALL	pop_gt
	MOVL	$0, -4(%ebp)
	JMP	.L9

.L13:
	CALL	make_rvalue
	CALL	next
	CALL	push
	CALL	shift_expr
	PUSH	%eax
	CALL	make_rvalue
	POP	%eax
	CALL	pop_ge
	MOVL	$0, -4(%ebp)
	JMP	.L9

.L14:
	POP	%eax
	POP	%ebp
	RET


####	#  Function:	type_t eq_expr();
	#
	#    eq-op   ::= '==' | '!=' 
	#    eq-expr ::= rel-expr ( eq-op rel-expr )*
	#
.local eq_expr
eq_expr:
	PUSH	%ebp	
	MOVL	%esp, %ebp

	CALL	rel_expr
	PUSH	%eax
.L15:
	MOVL	token, %eax
	CMPL	'==', %eax
	JE	.L16
	CMPL	'!=', %eax
	JE	.L17
	JMP	.L18

.L16:
	CALL	make_rvalue
	CALL	next
	CALL	push
	CALL	rel_expr
	PUSH	%eax
	CALL	make_rvalue
	POP	%eax
	CALL	pop_eq
	MOVL	$0, -4(%ebp)
	JMP	.L15

.L17:
	CALL	make_rvalue
	CALL	next
	CALL	push
	CALL	rel_expr
	PUSH	%eax
	CALL	make_rvalue
	POP	%eax
	CALL	pop_ne
	MOVL	$0, -4(%ebp)
	JMP	.L15

.L18:
	POP	%eax
	POP	%ebp
	RET


####	#  Function:	type_t bitand_expr();
	#
	#    bitand-expr ::= eq-expr ( '&' eq-expr )*
	#
.local bitand_expr
bitand_expr:
	PUSH	%ebp	
	MOVL	%esp, %ebp

	CALL	eq_expr
	PUSH	%eax
.L19:
	MOVL	token, %eax
	CMPL	'&', %eax
	JNE	.L20
	CALL	make_rvalue
	CALL	next
	CALL	push
	CALL	eq_expr
	PUSH	%eax
	CALL	make_rvalue
	POP	%eax
	CALL	pop_bitand
	MOVL	$0, -4(%ebp)
	JMP	.L19

.L20:
	POP	%eax
	POP	%ebp
	RET


####	#  Function:	type_t bitxor_expr();
	#
	#    bitxor-expr ::= bitand-expr ( '^' bitand-expr )*
	#
.local bitxor_expr
bitxor_expr:
	PUSH	%ebp	
	MOVL	%esp, %ebp

	CALL	bitand_expr
	PUSH	%eax
.L21:
	MOVL	token, %eax
	CMPL	'^', %eax
	JNE	.L22
	CALL	make_rvalue
	CALL	next
	CALL	push
	CALL	bitand_expr
	PUSH	%eax
	CALL	make_rvalue
	POP	%eax
	CALL	pop_bitxor
	MOVL	$0, -4(%ebp)
	JMP	.L21

.L22:
	POP	%eax
	POP	%ebp
	RET


####	#  Function:	type_t bitor_expr();
	#
	#    bitor-expr ::= bitxor-expr ( '|' bitxor-expr )*
	#
.local bitor_expr
bitor_expr:
	PUSH	%ebp	
	MOVL	%esp, %ebp

	CALL	bitxor_expr
	PUSH	%eax
.L23:
	MOVL	token, %eax
	CMPL	'|', %eax
	JNE	.L24
	CALL	make_rvalue
	CALL	next
	CALL	push
	CALL	bitxor_expr
	PUSH	%eax
	CALL	make_rvalue
	POP	%eax
	CALL	pop_bitor
	MOVL	$0, -4(%ebp)
	JMP	.L23

.L24:
	POP	%eax
	POP	%ebp
	RET


####	#  Function:	type_t logand_expr();
	#
	#    logand-expr ::= bitor-expr ( '&&' bitor-expr )*
	#
.local logand_expr
logand_expr:
	PUSH	%ebp	
	MOVL	%esp, %ebp

	CALL	bitor_expr
	PUSH	%eax
	MOVL	token, %eax
	CMPL	'&&', %eax
	JNE	.L26
.L25:
	CALL	make_rvalue
	CALL	new_label
	PUSH	%eax		# JMP -8(%ebp) if true
	CALL	branch_ifz
	CALL	next
	CALL	bitor_expr
	PUSH	%eax
	CALL	make_rvalue
	POP	%eax
	CALL	local_label
	POP	%eax
	MOVL	$0, -4(%ebp)

	MOVL	token, %eax
	CMPL	'&&', %eax
	JE	.L25
	CALL	cast_bool
.L26:
	POP	%eax
	POP	%ebp
	RET


####	#  Function:	type_t logor_expr();
	#
	#    logor-expr ::= logand-expr ( '||' logand-expr )*
	#
.local logor_expr
logor_expr:
	PUSH	%ebp	
	MOVL	%esp, %ebp

	CALL	logand_expr
	PUSH	%eax
	MOVL	token, %eax
	CMPL	'||', %eax
	JNE	.L28

.L27:
	CALL	make_rvalue
	CALL	new_label
	PUSH	%eax		# JMP -8(%ebp) if true
	CALL	branch_ifnz
	CALL	next
	CALL	logand_expr
	PUSH	%eax
	CALL	make_rvalue
	POP	%eax
	CALL	local_label
	POP	%eax
	MOVL	$0, -4(%ebp)

	MOVL	token, %eax
	CMPL	'||', %eax
	JE	.L27
	CALL	cast_bool
.L28:
	POP	%eax
	POP	%ebp
	RET


####	#  Function:	int new_label();
	#  Return the next local label id.
.data
.local label_count
label_count:
	.int	0
.text
new_label:
	PUSH	%ebp	
	MOVL	%esp, %ebp
	MOVL	label_count, %eax
	INCL	%eax
	MOVL	%eax, label_count
        POP     %ebp
        RET	


####	#  Function:	type_t cond_expr();
	#
	#    cond-expr ::= logor-expr ( '?' expr ':' cond-expr )?
	#
.local cond_expr
cond_expr:
	PUSH	%ebp
	MOVL	%esp, %ebp

	CALL	logor_expr
	PUSH	%eax

	MOVL	token, %eax
	CMPL	'?', %eax
	JNE	.L29

	CALL	make_rvalue
	CALL	new_label
	PUSH	%eax		# JMP -4(%ebp) if false
	CALL	new_label
	PUSH	%eax		# JMP -8(%ebp) to end

	PUSH	-4(%ebp)
	CALL	branch_ifz
	POP	%eax
	CALL	next
	CALL	expr
	PUSH	%eax
	CALL	make_rvalue
	POP	%eax
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
	PUSH	%eax
	CALL	make_rvalue
	POP	%eax
	PUSH	-8(%ebp)
	CALL	local_label
	POP	%eax
	MOVL	$0, -4(%ebp)

.L29:
	POP	%eax
	LEAVE
	RET


####	#  Function:	type_t assign_expr();
	#
	#    assign-op   ::= '='
	#    assign-expr ::= cond-expr ( assign-op assign-expr )?
	#
assign_expr:
	PUSH	%ebp
	MOVL	%esp, %ebp

	CALL	cond_expr
	PUSH	%eax

	MOVL	token, %eax
	CMPL	'=', %eax
	JNE	.L43

	POP	%eax
	TESTL	%eax, %eax
	JZ	_error
	CALL	next
	CALL	push
	CALL	assign_expr
	PUSH	%eax
	CALL	make_rvalue
	POP	%eax
	CALL	pop_assign
	XORL	%eax, %eax
	PUSH	%eax
.L43:
	POP	%eax
	POP	%ebp
	RET


####	#  Function:	type_t expr();
	#
	#  Process an expression.
expr:
	PUSH	%ebp	
	MOVL	%esp, %ebp

	CALL	assign_expr

	POP	%ebp
	RET


####	#  Function:	type_t rvalue_expr();
	#
	#  Process an expression.
rvalue_expr:
	PUSH	%ebp	
	MOVL	%esp, %ebp

	CALL	expr
	PUSH	%eax
	CALL	make_rvalue
	POP	%eax

	POP	%ebp
	RET
