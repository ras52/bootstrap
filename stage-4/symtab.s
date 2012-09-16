# symtab.s  --  Code to manipulate the symbol table 

# Copyright (C) 2012 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

.data

st_start:
	.int	0
st_end:
	.int	0
st_endstore:
	.int	0

#  struct entry { char sym[12]; int32_t frame_off; };

.text

####	#  Function:	void st_init();
	#
	#  Initialise the symbol table
init_symtab:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$1024, %ecx		# 64 * sizeof(entry)
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
	#  Save a local symbol
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
	POP	%eax
	MOVL	12(%ebp), %ecx
	MOVL	%ecx, 12(%eax)	# save value
	ADDL	$16, %eax	# sizeof(entry)
	MOVL	%eax, st_end

	LEAVE
	RET

####	#  Function:	void clr_symtab();
	#
	#  Clear the symbol table at end of scope
clr_symtab:
	PUSH	%ebp
	MOVL	%esp, %ebp

	MOVL	st_start, %eax
	MOVL	%eax, st_end

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
	SUBL	$16, %edi	# sizeof(entry)
.L2:
	ADDL	$16, %edi	# sizeof(entry)
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
	
