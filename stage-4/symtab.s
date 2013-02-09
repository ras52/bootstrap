# symtab.s  --  Code to manipulate the symbol table 

# Copyright (C) 2012, 2013 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

.data

.local st_start
st_start:
	.int	0
.local st_end
st_end:
	.int	0
.local st_endstore
st_endstore:
	.int	0
.local st_scope_id
st_scope_id:
	.int	0

#  struct entry { char sym[12]; int32_t frame_off; int32_t scope_id; 
#                 type_t lval; type_t size; };   -- sizeof(entry) == 28

.text

####	#  Function:	void st_init();
	#
	#  Initialise the symbol table
init_symtab:
	PUSH	%ebp
	MOVL	%esp, %ebp
#	MOVL	$1792, %ecx		# 64 * sizeof(entry)
	MOVL	$28, %ecx		# sizeof(entry)
	PUSH	%ecx
	CALL	malloc
	POP	%ecx
	MOVL	%eax, st_start
	MOVL	%eax, st_end
	ADDL	%ecx, %eax
	MOVL	%eax, st_endstore
	POP	%ebp
	RET


####	#  Function:	void grow_symtab();
	#
	#  Double the size of the symbol table storage
.local grow_symtab
grow_symtab:
	PUSH	%ebp
	MOVL	%esp, %ebp

	MOVL	st_start, %eax
	MOVL	%eax, %edx
        MOVL    st_endstore, %eax
	SUBL	%edx, %eax
	MOVB	$1, %cl
	SHLL	%eax
	
	PUSH	%eax		# new size
	PUSH	%edx		# current start ptr
	CALL	realloc

	#  Store new pointers
	MOVL	%eax, st_start

	MOVL	%eax, %edx	# new ptr
	POP	%ecx		# old ptr
	MOVL	st_end, %eax
	SUBL	%ecx, %eax
	ADDL	%edx, %eax
	MOVL	%eax, st_end

	POP	%eax		# new size
	ADDL	%edx, %eax
	MOVL	%eax, st_endstore

	POP	%ebp
	RET


####	#  Function:	void save_sym( char const* name, int32_t frame_off,
	#                              type_t lval, int32_t sym_size );
	#
	#  Save a local symbol.
save_sym:
	PUSH	%ebp
	MOVL	%esp, %ebp

	PUSH	8(%ebp)		# src  -4(%ebp)
	MOVL	st_end, %eax
	PUSH	%eax		# dest -8(%ebp)

        #  Check that we're not about to overrun the symbol table,
        MOVL    st_endstore, %eax
	CMPL	%eax, -8(%ebp)
	JL	.L1
	CALL	grow_symtab
	MOVL	st_end, %eax
	MOVL	%eax, -8(%ebp)
.L1:
	CALL	strcpy
	POP	%edx
	MOVL	12(%ebp), %ecx
	MOVL	%ecx, 12(%edx)	# save value
	MOVL	st_scope_id, %eax
	MOVL	%eax, 16(%edx)
	MOVL	16(%ebp), %ecx
	MOVL	%ecx, 20(%edx)	# lval flag
	MOVL	20(%ebp), %ecx
	MOVL	%ecx, 24(%edx)	# symbol size
	ADDL	$28, %edx	# sizeof(entry)
	MOVL	%edx, %eax
	MOVL	%eax, st_end

	LEAVE
	RET


####	#  Function:	void new_scope();
	#
	#  Called on parsing '{' or similar to start a new nested scope.
new_scope:
	PUSH	%ebp
	MOVL	%esp, %ebp

	MOVL	st_scope_id, %eax
	INCL	%eax
	MOVL	%eax, st_scope_id

	POP	%ebp
	RET


####	#  Function:	int end_scope();
	#
	#  Called on parsing '}' or similar to remove symbols from the table.
	#  Returns number of bytes that need removing from the stack.
end_scope:
	PUSH	%ebp
	MOVL	%esp, %ebp

	MOVL	st_end, %eax
	MOVL	%eax, %ecx		# %ecx  end
	MOVL	st_start, %eax
	SUBL	$28, %eax		# sizeof(entry)
	MOVL	%eax, %edx		# %edx  ptr
.L4:
	#  Zero %eax in case we jump to the end where %eax is the scope size
	XORL	%eax, %eax

	ADDL	$28, %edx		# sizeof(entry)
	CMPL	%ecx, %edx
	JGE	.L5

	MOVL	st_scope_id, %eax
	CMPL	%eax, 16(%edx)
	JL	.L4

	#  The symbol table is sorted by scope id, so as soon as we find
	#  one symbol in the current scope, all later ones must be too.

	#  First, shrink the table
	MOVL	%edx, %eax
	MOVL	%eax, st_end

	#  Then iterate over the remainder adding up the scope size
	MOVL	24(%edx), %eax		# %edx is now scope size
.L7:
	ADDL	$28, %edx		# sizeof(entry)
	CMPL	%ecx, %edx
	JGE	.L5
	ADDL	24(%edx), %eax
	JMP	.L7
.L5:
	PUSH	%eax			# store frame size
	MOVL	$st_scope_id, %eax
	DECL	(%eax)
	POP	%eax

	POP	%ebp
	RET


####	#  Function:	int lookup_sym(char const* name, int* off);
	#
	#  Return the lvalue flag for the symbol NAME, or 1 if it is 
	#  not defined (as we assume external symbols are lvalues).  
	#  Also set *OFF to the symbol table offset of the symbol, or
	#  0 if it is not defined (as 0 is not a valid offset because 
	#  0(%ebp) is the calling frame's base pointer.)
lookup_sym:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%edi
	PUSH	%esi

	PUSH	8(%ebp)
	MOVL	st_start, %eax
	MOVL	%eax, %edi
	MOVL	st_end, %eax
	MOVL	%eax, %esi
	SUBL	$28, %edi	# sizeof(entry)
.L2:
	ADDL	$28, %edi	# sizeof(entry)
	XORL	%eax, %eax
	CMPL	%esi, %edi
	JGE	.L3
	PUSH	%edi
	CALL	strcmp
	POP	%ecx
	TESTL	%eax, %eax
	JNZ	.L2
	MOVL	20(%edi), %eax		# return lv flag
	MOVL	12(%edi), %edx		# frame offset
	JMP	.L6
.L3:
	#  Symbol not found
	XORL	%eax, %eax
	INCL	%eax			# return +1 (is lvalue) if not found
	XORL	%edx, %edx		# use 0 frame offset for error
.L6:
	# write *off
	MOVL	12(%ebp), %ecx
	MOVL	%edx, (%ecx)

	POP	%ecx
	POP	%esi
	POP	%edi
	POP	%ebp
	RET
	
