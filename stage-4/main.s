# main.s  --  entry point 

# Copyright (C) 2012, 2013 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

.data
.globl frame_size
frame_size:
	.int	0

####	#  Function:	void int_decl(char* name);
	#
	#    constant ::= number | char
	#
	#    int_decl ::= name ( '=' constant )? ';'
	#
	#  Process an integer declaration for NAME.  The name has been read, 
	#  and TOKEN advanced to the next token: either '=' or ';'
.data .LC1:
	.string "\n.data\n%s:\n\t.int %s\n"
.text
.local int_decl
int_decl:
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
	JE	.L3a
	CMPL	'char', %eax
	JNE	_error

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


####	#  Function:	void func_decl(char* name);
	#
	#    func-params ::= name ( ',' name )*
	#
	#    func-decl   ::= name '(' func-params? ')' block
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
	#    ext-decl ::= func-decl | int-decl
	#  
	#  When called, TOKEN should be the name.
.local ext_decl
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

	CMPL	'id', %eax
	JNE	_error
	CALL	ext_decl
	JMP	.L1
.L2:
	POP	%ebp
	RET


####	#  Function:  int main(int argc, char** argv);
#
main:
	PUSH	%ebp
	MOVL	%esp, %ebp

	CMPL	$3, 8(%ebp)		# Require two arguments: -S file.c
	JNE	_error

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

	#  Use argv[2] as a filename and reopen stdin as it.
	MOVL	'r', %eax
	PUSH	%eax
	MOVL	%esp, %ecx
	MOVL	stdin, %eax
	PUSH	%eax
	PUSH	%ecx
	MOVL	12(%ebp), %eax
	PUSH	8(%eax)
	CALL	freopen
	ADDL	$16, %esp
	TESTL	%eax, %eax
	JZ	_error

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

	#  And reopen stdout as it.
	MOVL	'w', %eax
	PUSH	%eax
	MOVL	%esp, %ecx
	MOVL	stdout, %eax
	PUSH	%eax
	PUSH	%ecx
	MOVL	12(%ebp), %eax
	PUSH	8(%eax)
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
