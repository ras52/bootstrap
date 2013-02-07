# i386.s  --  file encapsulating the i386-specific code

# Copyright (C) 2012, 2013 Richard Smith <richard@ex-parrot.com>
# All rights reserved.


####	#  Function:	void prolog( char const* name );
	#  Write function prologue for function NAME.
.data .LC0:
.string	".text %s:\n\tPUSH\t%%ebp\n\tMOVL\t%%esp, %%ebp\n"
.text prolog:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	8(%ebp)
	MOVL	$.LC0, %eax
	PUSH	%eax
	CALL	printf
	LEAVE
	RET

####	#  Function:	void epilog();
	#  Write function epilogue.
.data .LC1:
.string	"\tLEAVE\n\tRET\n\n"
.text epilog:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$.LC1, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET


####	#  Function:	void dereference();
	#  Dereference the accumulator and store its value in the accumulator.
.data .LC2:
.string	"\tMOVL\t(%eax), %eax\n"
.text dereference:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$.LC2, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET


####	#  Function:	void load_const(char const* I);
	#  Load accumulator with an (integer) constant I.
.data .LC3:
.string	"\tMOVL\t$%s, %%eax\n"
.text load_const:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	8(%ebp)
	MOVL	$.LC3, %eax
	PUSH	%eax
	CALL	printf
	LEAVE
	RET

####	#  Function:	void load_chrlit(char const* CHR);
	#  Load accumulator with a character literal, CHR.
.data .LC3a:
.string	"\tMOVL\t%s, %%eax\n"
.text load_chrlit:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	8(%ebp)
	MOVL	$.LC3a, %eax
	PUSH	%eax
	CALL	printf
	LEAVE
	RET

####	#  Function:	int new_clabel();
	#  Return the next local constant label id.
.data
.local label_count
label_count:
	.int	0
.text
.local new_clabel
new_clabel:
	PUSH	%ebp	
	MOVL	%esp, %ebp
	MOVL	label_count, %eax
	INCL	%eax
	MOVL	%eax, label_count
        POP     %ebp
        RET	

####	#  Function:	void load_strlit(char const* STR);
	#  Load accumulator with a pointer to the string literal, STR.
.data .LC3b:
.string	".data .LC%d: .string %s\n.text\tMOVL\t$.LC%d, %%eax\n"
.text load_strlit:
	PUSH	%ebp
	MOVL	%esp, %ebp
	CALL	new_clabel
	PUSH	%eax
	PUSH	8(%ebp)
	PUSH	%eax
	MOVL	$.LC3b, %eax
	PUSH	%eax
	CALL	printf
	LEAVE
	RET

####	#  Function:	void load_local(int frame_offset);
	#  Load accumulator from local variable at frame_offset.
.data .LC4:
.string	"\tLEA\t%d(%%ebp), %%eax\n"
.text load_local:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	8(%ebp)
	MOVL	$.LC4, %eax
	PUSH	%eax
	CALL	printf
	LEAVE
	RET

####	#  Function:	void load_var(char const* name);
	#  Load accumlator from symbol NAME.
.data .LC5:
.string	"\tMOVL\t$%s, %%eax\n"
.text load_var:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	8(%ebp)
	MOVL	$.LC5, %eax
	PUSH	%eax
	CALL	printf
	LEAVE
	RET

####	#  Function:	void save_var(char const* name);
	#  Save accumulator into symbol NAME.
.data .LC6:
.string	"\tMOVL\t%%eax, %s\n"
.text save_var:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	8(%ebp)
	MOVL	$.LC6, %eax
	PUSH	%eax
	CALL	printf
	LEAVE
	RET

####	#  Function:	void arith_neg();
	#  Arithmetically negate the accumulator.
.data .LC7:
.string	"\tNEGL\t%eax\n"
.text arith_neg:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$.LC7, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET

####	#  Function:	void bit_not();
	#  Apply bitwise NOT to the accumulator.
.data .LC8:
.string	"\tNOTL\t%eax\n"
.text bit_not:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$.LC8, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET

####	#  Function:	void logic_not();
	#  Apply logical NOT to accumulator (using T=1, F=0)
.data .LC9:
.string	"\tTESTL\t%eax, %eax\n\tSETZ\t%al\n\tMOVZBL\t%al, %eax\n"
.text logic_not:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$.LC9, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET

####	#  Function:	void push();
	#  Push accumulator onto stack.
.data .LC10:
.string	"\tPUSH\t%eax\n"
.text push:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$.LC10, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET

####	#  Function:	void pop_assign();
	#  Pop value from stack, and assign accumulator to the value
.data .LC11:
.string "\tPOP\t%ecx\n\tMOVL\t%eax,(%ecx)\n"
.text pop_assign:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$.LC11, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET

####	#  Function:	void pop_bitand();
	#  Pop value from stack and logical AND it with the accumulator.
.data .LC12:
.string	"\tPOP\t%ecx\n\tANDL\t%ecx, %eax\n"
.text pop_bitand:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$.LC12, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET

####	#  Function:	void pop_bitor();
	#  Pop value from stack and OR it with the accumulator.
.data .LC13:
.string	"\tPOP\t%ecx\n\tORL\t%ecx, %eax\n"
.text pop_bitor:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$.LC13, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET

####	#  Function:	void pop_bitxor();
	#  Pop value from stack and XOR it with the accumulator.
.data .LC14:
.string	"\tPOP\t%ecx\n\tXORL\t%ecx, %eax\n"
.text pop_bitxor:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$.LC14, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET

####	#  Function:	void pop_eq();
	#  Pop value from stack, compare it with the accumulator,
	#  and set the accumulator to 1 if it's EQ to and 0 otherwise.
.data .LC15:
.string	"\tPOP\t%ecx\n\tCMPL\t%eax, %ecx\n\tSETE\t%al\n\tMOVZBL\t%al, %eax\n"
.text pop_eq:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$.LC15, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET
	
####	#  Function:	void pop_ne();
	#  Pop value from stack, compare it with the accumulator,
	#  and set the accumulator to 1 if it's NE to and 0 otherwise.
.data .LC16:
.string	"\tPOP\t%ecx\n\tCMPL\t%eax, %ecx\n\tSETNE\t%al\n\tMOVZBL\t%al, %eax\n"
.text pop_ne:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$.LC16, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET

####	#  Function:	void pop_gt();
	#  Pop value from stack, compare it with the accumulator,
	#  and set the accumulator to 1 if it's GT to and 0 otherwise.
.data .LC17:
.string	"\tPOP\t%ecx\n\tCMPL\t%eax, %ecx\n\tSETG\t%al\n\tMOVZBL\t%al, %eax\n"
.text pop_gt:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$.LC17, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET

####	#  Function:	void pop_lt();
	#  Pop value from stack, compare it with the accumulator,
	#  and set the accumulator to 1 if it's LT to and 0 otherwise.
.data .LC18:
.string	"\tPOP\t%ecx\n\tCMPL\t%eax, %ecx\n\tSETL\t%al\n\tMOVZBL\t%al, %eax\n"
.text pop_lt:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$.LC18, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET

####	#  Function:	void pop_ge();
	#  Pop value from stack, compare it with the accumulator,
	#  and set the accumulator to 1 if it's GE to and 0 otherwise.
.data .LC19:
.string	"\tPOP\t%ecx\n\tCMPL\t%eax, %ecx\n\tSETGE\t%al\n\tMOVZBL\t%al, %eax\n"
.text pop_ge:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$.LC19, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET
	
####	#  Function:	void pop_le();
	#  Pop value from stack, compare it with the accumulator,
	#  and set the accumulator to 1 if it's LE to and 0 otherwise.
.data .LC20:
.string	"\tPOP\t%ecx\n\tCMPL\t%eax, %ecx\n\tSETLE\t%al\n\tMOVZBL\t%al, %eax\n"
.text pop_le:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$.LC20, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET

####	#  Function:	void pop_lshift();
	#  Pop value from stack, LEFT SHIFT it by the number of bits in
	#  the accumulator, and move the value to the accumulator.
.data .LC21:
.string	"\tMOVL\t%eax, %ecx\n\tPOP\t%eax\nSALL\t%eax"
.text pop_lshift:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$.LC21, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET

####	#  Function:	void pop_rshift();
	#  Pop value from stack, RIGHT SHIFT it by the number of bits in
	#  the accumulator, and move the value to the accumulator.
.data .LC22:
.string	"\tMOVL\t%eax, %ecx\n\tPOP\t%eax\nSARL\t%eax"
.text pop_rshift:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$.LC22, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET

####	#  Function:	void pop_add();
	#  Pop value from stack and ADD it to the accumulator.
.data .LC23:
.string	"\tPOP\t%ecx\n\tADDL\t%ecx, %eax\n"
.text pop_add:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$.LC23, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET

####	#  Function:	void pop_sub();
	#  Pop value from stack and SUBTRACT the accumulator,
	#  moving the result into the accumulator.
.data .LC24:
.string	"\tPOP\t%ecx\n\tSUBL\t%eax, %ecx\n\tMOVL\t%ecx, %eax\n"
.text pop_sub:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$.LC24, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET

####	#  Function:	void pop_mult();
	#  Pop value from stack and MULTIPLY it by the accumulator.
.data .LC25:
.string	"\tPOP\t%ecx\n\tIMULL\t%ecx\n"
.text pop_mult:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$.LC25, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET

####	#  Function:	void pop_div();
	#  Pop value from stack and DIVIDE by the accumulator,
	#  moving the quotient into the accumulator.
.data .LC26:
.string	"\tMOVL\t%eax, %ecx\n\tXORL\t%edx, %edx\n\tPOP\t%eax\n\tIDIVL\t%ecx\n"
.text pop_div:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$.LC26, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET

####	#  Function:	void pop_mod();
	#  Pop value from stack and divide by the accumulator,
	#  moving the REMAINDER into the accumulator.
.data .LC27:
.string	"\tMOVL\t%edx, %eax\n"
.text pop_mod:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$.LC26, %eax
	PUSH	%eax
	CALL	putstr
	POP	%eax
	MOVL	$.LC27, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET

####	#  Function:	void increment();
	#  Increment the value pointed to by the accumulator.
.data .LC28:
.string	"\tINCL\t(%eax)\n"
.text increment:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$.LC28, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET

####	#  Function:	void decrement();
	#  Decrement the value pointed to by the accumulator.
.data .LC29:
.string	"\tDECL\t(%eax)\n"
.text decrement:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$.LC29, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET

####	#  Function:	void local_label(int id);
.data .LC30:
.string	".L%d:\n"
.text local_label:
	PUSH	%ebp	
	MOVL	%esp, %ebp
	PUSH	8(%ebp)
	MOVL	$.LC30, %eax
	PUSH	%eax
	CALL	printf
        LEAVE
        RET	

####	#  Function:	void cast_bool();
	#  Cast the accumulator to a boolean
.data .LC31:
.string "\tTESTL\t%eax, %eax\n\tSETNZ\t%al\n\tMOVZBL\t%al, %eax\n"
.text cast_bool:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$.LC31, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET

####	#  Function:	void branch(int id);
	#  Jump to the local label ID.
.data .LC32:
.string	"\tJMP\t.L%d\n"
.text branch:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	8(%ebp)
	MOVL	$.LC32, %eax
	PUSH	%eax
	CALL	printf
	LEAVE
	RET

####	#  Function:	void branch_ifz(int id);
	#  Jump to the local label ID if the accumulator is zero (false).
.data .LC33:
.string	"\tTESTL\t%%eax, %%eax\n\tJZ\t.L%d\n"
.text branch_ifz:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	8(%ebp)
	MOVL	$.LC33, %eax
	PUSH	%eax
	CALL	printf
	LEAVE
	RET

####	#  Function:	void branch_ifnz(int id);
	#  Jump to the local label ID if the accumulator is non-zero (true).
.data .LC34:
.string	"\tTESTL\t%%eax, %%eax\n\tJNZ\t.L%d\n"
.text branch_ifnz:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	8(%ebp)
	MOVL	$.LC34, %eax
	PUSH	%eax
	CALL	printf
	LEAVE
	RET

####	#  Function:	void clr_stack(int n_bytes);
.data .LC35:
.string "\tADDL\t$%d, %%esp\n"
.text clear_stack:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	8(%ebp), %eax
	TESTL	%eax, %eax
	JZ	.L4

	PUSH	%eax
	MOVL	$.LC35, %eax
	PUSH	%eax
	CALL	printf
.L4:
	LEAVE
	RET

####	#  Function:	void call(char const* name, int nargs);
	#  We've just pushed NARGS arguments.  Now call NAME.
.data .LC36:
.string "\tMOVL\t%esp, %eax\n"
.data .LC37:
.string "\tPUSH\t%d(%%eax)\n"
.data .LC38:
.string "\tCALL\t%s\n"
.text call:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%esi
	PUSH	%edi

	XORL	%edi, %edi
	MOVL	12(%ebp), %esi
	TESTL	%esi, %esi
	JZ	.L1

	MOVL	$.LC36, %eax
	PUSH	%eax
	CALL	putstr
	POP	%eax

.L2:
	ADDL	$4, %edi
	DECL	%esi
	TESTL	%esi, %esi
	JZ	.L1

	PUSH	%edi
	MOVL	$.LC37, %eax
	PUSH	%eax
	CALL	printf
	POP	%eax
	POP	%eax

	JMP	.L2
.L1:

	PUSH	8(%ebp)
	MOVL	$.LC38, %eax
	PUSH	%eax
	CALL	printf
	POP	%eax
	POP	%eax
	
	MOVL	12(%ebp), %eax
	TESTL	%eax, %eax
	JZ	.L3
	MOVL	$8, %ecx
	IMULL	%ecx			# on %edx:%eax
	SUBL	$4, %eax
	PUSH	%eax
	CALL	clear_stack
	POP	%eax
	
.L3:
	POP	%edi
	POP	%esi
	POP	%ebp
	RET

.data .LC39:
.string	"\tMOVB\t$2, %cl\n\tSHLL\t%eax\n"
.text conv_w2bo:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$.LC39, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET

####	#  Function:	void alloc_stack(int n_bytes);
.data .LC40:
.string "\tSUBL\t$%d, %%esp\n"
.text alloc_stack:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	8(%ebp)
	MOVL	$.LC40, %eax
	PUSH	%eax
	CALL	printf
	LEAVE
	RET

####	#  Function:	void stk_assign(int offset);
	#  Assign accumulator to an address on the stack
.data .LC41:
.string "\tMOVL\t%%eax,%d(%%ebp)\n"
.text stk_assign:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	8(%ebp)
	MOVL	$.LC41, %eax
	PUSH	%eax
	CALL	printf
	LEAVE
	RET

####	#  Function:	void postfix_inc();
	#  Increment the value pointed to by the accumulator.
.data .LC42:
.string	"\tMOVL\t%eax, %ecx\n\tMOVL\t(%eax), %eax\n\tINCL\t(%ecx)\n"
.text postfix_inc:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$.LC42, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET

####	#  Function:	void postfix_dec();
	#  Decrement the value pointed to by the accumulator.
.data .LC43:
.string	"\tMOVL\t%eax, %ecx\n\tMOVL\t(%eax), %eax\n\tDECL\t(%ecx)\n"
.text postfix_dec:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$.LC43, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET


