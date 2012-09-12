# i386.s  --  file encapsulating the i386-specific code

# Copyright (C) 2012 Richard Smith <richard@ex-parrot.com>
# All rights reserved.


####	#  Function:	void prolog( char const* name );
	#  Write function prologue for function NAME.
.data s_prolog:
.string	".text %s:\n\tPUSH\t%%ebp\n\tMOVL\t%%esp, %%ebp\n"
.text prolog:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	8(%ebp)
	MOVL	$s_prolog, %eax
	PUSH	%eax
	CALL	printf
	LEAVE
	RET

####	#  Function:	void epilog();
	#  Write function epilogue.
.data s_epilog:
.string	"\tLEAVE\n\tRET\n"
.text epilog:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$s_epilog, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET

####	#  Function:	void load_const(char const* I);
	#  Load accumulator with an (integer) constant I.
.data s_l_const:
.string	"\tMOVL\t$%s, %%eax\n"
.text load_const:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	8(%ebp)
	MOVL	$s_l_const, %eax
	PUSH	%eax
	CALL	printf
	LEAVE
	RET

####	#  Function:	void load_var(char const* name);
	#  Load accumlator from symbol NAME.
.data s_l_var:
.string	"\tMOVL\t%s, %%eax\n"
.text load_var:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	8(%ebp)
	MOVL	$s_l_var, %eax
	PUSH	%eax
	CALL	printf
	LEAVE
	RET

####	#  Function:	void save_var(char const* name);
	#  Save accumulator into symbol NAME.
.data s_s_var:
.string	"\tMOVL\t%%eax, %s\n"
.text save_var:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	8(%ebp)
	MOVL	$s_s_var, %eax
	PUSH	%eax
	CALL	printf
	LEAVE
	RET

####	#  Function:	void arith_neg();
	#  Arithmetically negate the accumulator.
.data s_a_neg:
.string	"\tNEGL\t%eax\n"
.text arith_neg:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$s_a_neg, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET

####	#  Function:	void bit_not();
	#  Apply bitwise NOT to the accumulator.
.data s_a_not:
.string	"\tNOTL\t%eax\n"
.text bit_not:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$s_a_not, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET

####	#  Function:	void logic_not();
	#  Apply logical NOT to accumulator (using T=1, F=0)
.data s_log_not:
.string	"\tTESTL\t%eax, %eax\n\tSETZ\t%al\n\tMOVZBL\t%al, %eax\n"
logic_not:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$s_log_not, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET

####	#  Function:	void push();
	#  Push accumulator onto stack.
.data s_push:
.string	"\tPUSH\t%eax\n"
.text push:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$s_push, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET

####	#  Function:	void pop_bitand();
	#  Pop value from stack and logical AND it with the accumulator.
.data s_pop_band:
.string	"\tPOP\t%ecx\n\tANDL\t%ecx, %eax\n"
.text pop_bitand:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$s_pop_band, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET

####	#  Function:	void pop_bitor();
	#  Pop value from stack and OR it with the accumulator.
.data s_pop_bor:
.string	"\tPOP\t%ecx\n\tORL\t%ecx, %eax\n"
.text pop_bitor:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$s_pop_bor, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET

####	#  Function:	void pop_bitxor();
	#  Pop value from stack and XOR it with the accumulator.
.data s_pop_bxor:
.string	"\tPOP\t%ecx\n\tXORL\t%ecx, %eax\n"
.text pop_bitxor:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$s_pop_bxor, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET

####	#  Function:	void pop_eq();
	#  Pop value from stack, compare it with the accumulator,
	#  and set the accumulator to 1 if it's EQ to and 0 otherwise.
.data s_pop_eq:
.string	"\tPOP\t%ecx\n\tCMPL\t%eax, %ecx\n\tSETE\t%al\n\tMOVZBL\t%al, %eax\n"
.text pop_eq:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$s_pop_eq, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET
	
####	#  Function:	void pop_ne();
	#  Pop value from stack, compare it with the accumulator,
	#  and set the accumulator to 1 if it's NE to and 0 otherwise.
.data s_pop_ne:
.string	"\tPOP\t%ecx\n\tCMPL\t%eax, %ecx\n\tSETNE\t%al\n\tMOVZBL\t%al, %eax\n"
.text pop_ne:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$s_pop_ne, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET

####	#  Function:	void pop_gt();
	#  Pop value from stack, compare it with the accumulator,
	#  and set the accumulator to 1 if it's GT to and 0 otherwise.
.data s_pop_gt:
.string	"\tPOP\t%ecx\n\tCMPL\t%eax, %ecx\n\tSETG\t%al\n\tMOVZBL\t%al, %eax\n"
.text pop_gt:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$s_pop_gt, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET

####	#  Function:	void pop_lt();
	#  Pop value from stack, compare it with the accumulator,
	#  and set the accumulator to 1 if it's LT to and 0 otherwise.
.data s_pop_lt:
.string	"\tPOP\t%ecx\n\tCMPL\t%eax, %ecx\n\tSETL\t%al\n\tMOVZBL\t%al, %eax\n"
.text pop_lt:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$s_pop_lt, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET

####	#  Function:	void pop_ge();
	#  Pop value from stack, compare it with the accumulator,
	#  and set the accumulator to 1 if it's GE to and 0 otherwise.
.data s_pop_ge:
.string	"\tPOP\t%ecx\n\tCMPL\t%eax, %ecx\n\tSETGE\t%al\n\tMOVZBL\t%al, %eax\n"
.text pop_ge:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$s_pop_ge, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET
	
####	#  Function:	void pop_le();
	#  Pop value from stack, compare it with the accumulator,
	#  and set the accumulator to 1 if it's LE to and 0 otherwise.
.data s_pop_le:
.string	"\tPOP\t%ecx\n\tCMPL\t%eax, %ecx\n\tSETLE\t%al\n\tMOVZBL\t%al, %eax\n"
.text pop_le:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$s_pop_le, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET

####	#  Function:	void pop_lshift();
	#  Pop value from stack, LEFT SHIFT it by the number of bits in
	#  the accumulator, and move the value to the accumulator.
.data s_pop_lshft:
.string	"\tMOVL\t%eax, %ecx\n\tPOP\t%eax\nSALL\t%eax"
.text pop_lshift:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$s_pop_lshft, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET

####	#  Function:	void pop_rshift();
	#  Pop value from stack, RIGHT SHIFT it by the number of bits in
	#  the accumulator, and move the value to the accumulator.
.data s_pop_rshft:
.string	"\tMOVL\t%eax, %ecx\n\tPOP\t%eax\nSARL\t%eax"
.text pop_rshift:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$s_pop_rshft, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET

####	#  Function:	void pop_add();
	#  Pop value from stack and ADD it to the accumulator.
.data s_pop_add:
.string	"\tPOP\t%ecx\n\tADDL\t%ecx, %eax\n"
.text pop_add:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$s_pop_add, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET

####	#  Function:	void pop_sub();
	#  Pop value from stack and SUBTRACT the accumulator,
	#  moving the result into the accumulator.
.data s_pop_sub:
.string	"\tPOP\t%ecx\n\tSUBL\t%eax, %ecx\n\tMOVL\t%ecx, %eax\n"
.text pop_sub:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$s_pop_sub, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET

####	#  Function:	void pop_mult();
	#  Pop value from stack and MULTIPLY it by the accumulator.
.data s_pop_mul:
.string	"\tPOP\t%ecx\n\tIMULL\t%ecx\n"
.text pop_mult:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$s_pop_mul, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET

####	#  Function:	void pop_div();
	#  Pop value from stack and DIVIDE by the accumulator,
	#  moving the quotient into the accumulator.
.data s_pop_div:
.string	"\tMOVL\t%eax, %ecx\n\tXORL\t%edx, %edx\n\tPOP\t%eax\n\tIDIVL\t%ecx\n"
.text pop_div:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$s_pop_div, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET

####	#  Function:	void pop_mod();
	#  Pop value from stack and divide by the accumulator,
	#  moving the REMAINDER into the accumulator.
.data s_pop_mod:
.string	"\tMOVL\t%edx, %eax\n"
.text pop_mod:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$s_pop_div, %eax
	PUSH	%eax
	CALL	putstr
	POP	%eax
	MOVL	$s_pop_mod, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET

####	#  Function:	void local_label(int id);
.data f_loc_label:
.string	".L%d:\n"
.text local_label:
	PUSH	%ebp	
	MOVL	%esp, %ebp
	PUSH	8(%ebp)
	MOVL	$f_loc_label, %eax
	PUSH	%eax
	CALL	printf
        LEAVE
        RET	

####	#  Function:	void cast_bool();
	#  Cast the accumulator to a boolean
.data s_cast_bool:
.string "\tTESTL\t%eax, %eax\n\tSETNZ\t%al\n\tMOVZBL\t%al, %eax\n"
.text cast_bool:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$s_cast_bool, %eax
	PUSH	%eax
	CALL	putstr
	LEAVE
	RET

####	#  Function:	void branch(int id);
	#  Jump to the local label ID.
.data s_branch:
.string	"\tJMP\t.L%d\n"
.text branch:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	8(%ebp)
	MOVL	$s_branch, %eax
	PUSH	%eax
	CALL	printf
	LEAVE
	RET

####	#  Function:	void branch_ifz(int id);
	#  Jump to the local label ID if the accumulator is zero (false).
.data s_bra_ifz:
.string	"\tTESTL\t%%eax, %%eax\n\tJZ\t.L%d\n"
.text branch_ifz:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	8(%ebp)
	MOVL	$s_bra_ifz, %eax
	PUSH	%eax
	CALL	printf
	LEAVE
	RET

####	#  Function:	void branch_ifnz(int id);
	#  Jump to the local label ID if the accumulator is non-zero (true).
.data s_bra_ifnz:
.string	"\tTESTL\t%%eax, %%eax\n\tJNZ\t.L%d\n"
.text branch_ifnz:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	8(%ebp)
	MOVL	$s_bra_ifnz, %eax
	PUSH	%eax
	CALL	printf
	LEAVE
	RET
