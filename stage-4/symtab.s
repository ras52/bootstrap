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

#  struct entry { char sym[12]; int32_t frame_off; int32_t scope_id; };

.text

####	#  Function:	void st_init();
	#
	#  Initialise the symbol table
init_symtab:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$1280, %ecx		# 64 * sizeof(entry)
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


####	#  Function:	void save_sym( char const* name, int32_t frame_off );
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
.L1:
	CALL	strcpy
	POP	%edx
	MOVL	12(%ebp), %ecx
	MOVL	%ecx, 12(%edx)	# save value
	MOVL	st_scope_id, %eax
	MOVL	%eax, 16(%edx)
	ADDL	$20, %edx	# sizeof(entry)
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

	MOVL	st_scope_id, %eax
	MOVL	%eax, %edx		# %edx  id
	MOVL	st_end, %eax
	MOVL	%eax, %ecx		# %ecx  end
	MOVL	st_start, %eax		# %eax  ptr
	SUBL	$20, %eax		# sizeof(entry)
.L4:
	ADDL	$20, %eax		# sizeof(entry)
	CMPL	%ecx, %eax
	JGE	.L5

	CMPL	%edx, 16(%eax)
	JL	.L4

	MOVL	%eax, %ecx
	MOVL	st_end, %eax
	XCHGL	%eax, %ecx
	MOVL	%eax, st_end
.L5:
	DECL	%edx
	MOVL	%edx, %eax
	MOVL	%eax, st_scope_id

	#  Return number of variables removed
	MOVL	st_end, %eax
	SUBL	%ecx, %eax
	NEGL	%eax
	XORL	%edx, %edx
	MOVL	$5, %ecx		# sizeof(entry)/4
	IDIVL	%ecx

	POP	%ebp
	RET


####	#  Function:	int lookup_sym(char const* name);
	#
	#  Return the frame offset of the symbol NAME, or 0 if it is 
	#  not defined.  (0 is not a valid offset because (%ebp) is the 
	#  calling frame's base pointer.)
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
	SUBL	$20, %edi	# sizeof(entry)
.L2:
	ADDL	$20, %edi	# sizeof(entry)
	XORL	%eax, %eax
	CMPL	%esi, %edi
	JGE	.L3
	PUSH	%edi
	CALL	strcmp
	POP	%ecx
	TESTL	%eax, %eax
	JNZ	.L2
	MOVL	12(%edi), %eax
.L3:
	POP	%ecx

	POP	%esi
	POP	%edi
	POP	%ebp
	RET
	
