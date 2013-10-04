# main.s  --  entry point 

# Copyright (C) 2012, 2013 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

.data
.globl frame_size
frame_size:
	.int	0

####	#  Function:	void strg_class(char* name, bool is_static);
	#
	#  Emit a .globl or .local directive for NAME as appropriate for 
	#  the storage class in IS_STATIC
.data .LC2:
	.string ".globl\t%s\n"
.LC3:
	.string ".local\t%s\n"
.text
.local strg_class
strg_class:
	PUSH	%ebp	
	MOVL	%esp, %ebp

	PUSH	8(%ebp)
	MOVL	12(%ebp), %eax
	TESTL	%eax, %eax
	JZ	.L9
	MOVL	$.LC3, %eax
	JMP	.L10
.L9:
	MOVL	$.LC2, %eax	
.L10:
	PUSH	%eax
	CALL	printf

	LEAVE
	RET


####	#  Function:	int init_a_decl(int dim);
	#
	#    init-a-list ::= constant ( ',' constant )*
	#
	#    init-a-decl ::= ( '=' '{' init-a-list '}' )? ';'
	#
	#  Current token is '=' if an initialiser is present.   
	#  Returns the number of uninitialised elements.
.data .LC5:
	.string ".int\t"
.text
.local init_a_decl
init_a_decl:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	8(%ebp)			# -4(%ebp) local copy of dim

	MOVL	token, %eax
	CMPL	'=', %eax
	JNE	.L11
	CALL	next
	CMPL	'{', %eax
	JNE	_error

	MOVL	$.LC5, %eax
	PUSH	%eax
	CALL	putstr
	POP	%eax

.L12:	#  Loop over initialisers.
	#  Do we have too many?
	DECL	-4(%ebp)
	CMPL	$0, -4(%ebp)
	JL	_error
	
	CALL	next
	CMPL	'num', %eax
	JE	.L13
	CMPL	'char', %eax
	JE	.L13
	CMPL	'id', %eax
	JE	.L13
	JMP	_error

.L13:	# number or char
	MOVL	$value, %eax
	PUSH	%eax
	CALL	putstr
	POP	%eax

	CALL	next
	CMPL	',', %eax
	JNE	.L14

	PUSH	%eax
	CALL	putchar
	POP	%eax
	JMP	.L12

.L14:
	CMPL	'}', %eax
	JNE	_error

	MOVL	'\n', %eax
	PUSH	%eax
	CALL	putchar
	POP	%eax

	CALL	next
.L11:
	CMPL	';', %eax
	JNE	_error
	CALL	next

	POP	%eax
	POP	%ebp
	RET


####	#  Function:	void array_decl(char* name, bool is_static);
	#
	#    array-decl ::= ( 'static' )? name '[' number '] init-a-decl
	#
	#  Process an array declaration for NAME.  Current token is '['.

.data .LC4:
	.string ".data\n%s:\n"
.LC6:
	.string ".zero\t%d\n"
.text
.local array_decl
array_decl:
	PUSH	%ebp
	MOVL	%esp, %ebp

	#  Require an array size
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

	MOVB	$2, %cl			# *= sizeof(int)
	SHLL	%eax
	PUSH	%eax			# byte size: probably not neeed?
	XORL	%eax, %eax
	PUSH	%eax			# not an lvalue
	PUSH	%eax			# frame offset == 0 (i.e. for linker)
	PUSH	8(%ebp)			# name
	CALL	save_sym
	ADDL	$16, %esp
	
	CALL	next
	CMPL	']', %eax
	JNE	_error

	#  Emit the symbol name
	PUSH	8(%ebp)
	MOVL	$.LC4, %eax
	PUSH	%eax
	CALL	printf
	POP	%eax
	POP	%eax

	#  Do a series of .int decls for the initialisers
	CALL	next
	CALL	init_a_decl

	MOVB	$2, %cl			# *= sizeof(int)
	SHLL	%eax
	TESTL	%eax, %eax
	JZ	.L15

	PUSH	%eax
	MOVL	$.LC6, %eax
	PUSH	%eax
	CALL	printf
	POP	%eax
	POP	%eax

.L15:
	LEAVE
	RET	


####	#  Function:	void int_decl(char* name, bool is_static);
	#
	#    constant ::= number | char | name
	#
	#    int_decl ::= ( 'static' )? name ( '=' constant )? ';'
	#
	#  Process an integer declaration for NAME.  The name has been read, 
	#  and TOKEN advanced to the next token: either '=' or ';'
.data .LC1:
	.string ".data\n%s:\n\t.int %s\n"
.text
.local int_decl
int_decl:
	PUSH	%ebp	
	MOVL	%esp, %ebp

	XORL	%eax, %eax
	PUSH	%eax			# external, so zero size on stack
	INCL	%eax
	PUSH	%eax			# objects are lvalues
	DECL	%eax
	PUSH	%eax			# frame_off == 0 for undefined
	PUSH	8(%ebp)			# name
	CALL	save_sym
	ADDL	$16, %esp

	MOVL	'0', %eax
	PUSH	%eax
	MOVL	token, %eax
	CMPL	'=', %eax
	JNE	.L3

	#  Read an initialiser
	CALL	next
	CMPL	'num', %eax
	JE	.L3a
	CMPL	'char', %eax
	JE	.L3a
	CMPL	'id', %eax
	JE	.L3a
	JMP	_error

.L3a:
	#  Because the next token is punctuation, value is not set.
	CALL	next
	MOVL	$value, %eax
	PUSH	%eax
	JMP	.L4
.L3:
	PUSH	%esp
.L4:
	PUSH	8(%ebp)
	MOVL	$.LC1, %eax
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


####	#  Function:	void func_decl(char* name, bool is_static);
	#
	#    func-params ::= name ( ',' name )*
	#
	#    func-head   ::= ( 'static' )? name '(' func-params? ')'
	#
	#    func-decl   ::= func-head param-decls block
	#
	#  Process a function declaration.  Current token is '('.
.local func_decl
func_decl:
	PUSH	%ebp	
	MOVL	%esp, %ebp

	CALL	new_scope
	XORL	%eax, %eax
	MOVL	%eax, frame_size

	CALL	next
	CMPL	')', %eax
	JE	.L5

	MOVL	$4, %ecx
	PUSH	%ecx		# all parameters have size 4	-4(%ebp)
	MOVL	$1, %ecx
	PUSH	%ecx		# parameters are lvalues	-8(%ebp)
	MOVL	$8, %ecx
	PUSH	%ecx		# frame_off			-12(%ebp)

.L5a:
	CMPL	'id', %eax
	JNE	_error

	MOVL	$value, %eax
	PUSH	%eax
	CALL	save_sym
	POP	%eax
	CALL	next

	ADDL	$4, -12(%ebp)
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
	CALL	param_decls
	CALL	block
	POP	%eax
	POP	%eax
	CALL	local_label
	POP	%eax

	#  Don't call clear_stack because this scope only contains 
	#  function parameters, and the caller cleans up the stack.
	CALL	end_scope
	CALL	epilog

	LEAVE
	RET

####	#  Function:	void ext_decl();
	#
	#  Process an external declaration.
	#
	#    ext-decl ::= func-decl | int-decl | array-decl
	#  
	#  When called, TOKEN should be the name.
.local ext_decl
ext_decl:
	PUSH	%ebp	
	MOVL	%esp, %ebp

	SUBL	$16, %esp		# -16(%ebp) buffer

	#  Are we static?
	XORL	%eax, %eax
	PUSH	%eax			# bool is_static = false;  -20(%ebp)
	MOVL	token, %eax
	CMPL	'stat', %eax
	JNE	.L8
	INCL	-20(%ebp)		# is_static = 1
	CALL	next
.L8:
	CALL	skip_type
.L8a:
	#  Skip any pointer declarators
	MOVL	token, %eax
	CMPL	'*', %eax
	JNE	.L8b
	CALL	next
	JMP	.L8a
.L8b:
	#  Check that we've read a identifier first
	CMPL	'id', %eax
	JNE	_error

	#  Store a copy of the name.  (We can't emit the label yet as we
	#  don't yet know which section it belongs in.)
	MOVL	$value, %eax
	PUSH	%eax			# src
	CALL	strlen
	CMPL	$11, %eax
	JG	_error
	LEA	-16(%ebp), %eax
	PUSH	%eax			# dest
	CALL	strcpy
	POP	%ecx
	POP	%eax
	PUSH	%ecx			# Pointer to name

	CALL	strg_class
	
	#  Get the next token and dispatch based on it.
	CALL	next
	CMPL	'(', %eax
	JE	.L6
	CMPL	'[', %eax
	JE	.L6a
	CALL	int_decl
	JMP	.L7
.L6:	
	CALL	func_decl
	JMP	.L7
.L6a:
	CALL	array_decl
.L7:
	LEAVE
	RET

####	#  Function:  void program();
	#  
	#    program ::= ext-decl*
	#
program:
	PUSH	%ebp
	MOVL	%esp, %ebp
.L1:
	MOVL	token, %eax
	CMPL	$-1, %eax
	JE	.L2

	CALL	ext_decl

	#  Add a blank line 
	MOVL	'\n', %eax
	PUSH	%eax
	CALL	putchar
	POP	%eax
	JMP	.L1
.L2:
	POP	%ebp
	RET


####	#  Function:  int main(int argc, char** argv);
#
main:
	PUSH	%ebp
	MOVL	%esp, %ebp

	CMPL	$3, 8(%ebp)	# Require at least two arguments: -S file.c
	JL	_error

	#  Check that argv[1] == '-S'
	MOVL	'-S', %eax
	PUSH	%eax
	PUSH	%esp
	MOVL	12(%ebp), %eax
	PUSH	4(%eax)
	CALL	strcmp
	ADDL	$12, %esp
	TESTL	%eax, %eax
	JNZ	_error

	#  Do we have -o file.o?
	MOVL	$8, %edx	# 8 == 4*argn
	CMPL	$3, 8(%ebp)	# If just two args (-S file.c)
	JE	.L16
	CMPL	$5, 8(%ebp)	# Otherwise four (-S -o file.o file.c)
	JNE	_error

	#  Check that argv[2] == '-o'
	MOVL	'-o', %eax
	PUSH	%eax
	PUSH	%esp
	MOVL	12(%ebp), %eax
	PUSH	8(%eax)
	CALL	strcmp
	ADDL	$12, %esp
	TESTL	%eax, %eax
	JNZ	_error

	MOVL	$16, %edx	# 16 == 4*argn

.L16:
	#  Use argv[4*argn] as a filename and reopen stdin as it.
	MOVL	'r', %eax
	PUSH	%eax
	MOVL	%esp, %ecx
	MOVL	stdin, %eax
	PUSH	%eax			# stream
	PUSH	%ecx			# mode
	MOVL	12(%ebp), %eax
	ADDL	%edx, %eax
	PUSH	(%eax)			# filename (argv[argn])
	CALL	freopen
	ADDL	$16, %esp
	TESTL	%eax, %eax
	JZ	_error

	#  Do we have an explicit output filename?
	MOVL	12(%ebp), %eax
	MOVL	12(%eax), %edx
	CMPL	$5, 8(%ebp)
	JE	.L17

	#  Construct the output filename.
	MOVL	12(%ebp), %eax
	PUSH	8(%eax)
	CALL	strlen
	POP	%edx
	ADDL	%eax, %edx
	CMPB	'c', -1(%edx)
	JNE	_error
	CMPB	'.', -2(%edx)
	JNE	_error
	MOVB	's', -1(%edx)
	MOVL	12(%ebp), %eax
	MOVL	8(%eax), %edx

.L17:
	#  And reopen stdout as it.
	MOVL	'w', %eax
	PUSH	%eax
	MOVL	%esp, %ecx
	MOVL	stdout, %eax
	PUSH	%eax		# stream
	PUSH	%ecx		# mode
	PUSH	%edx		# filename
	CALL	freopen
	ADDL	$16, %esp
	TESTL	%eax, %eax
	JZ	_error

	CALL	init_symtab
	CALL	next
	CALL	program
	XORL	%eax, %eax
	LEAVE
	RET


####	#  Function:	void _error()
	#
	#  All error handling is done here.  
	#  NB. There is a duplicate (identical) defintion in libc0.o
	#  (Note we can JMP here instead of CALLing it, as we never RET.)
_error:
	MOVL	$1, %eax
	PUSH	%eax
	CALL	exit
	HLT


