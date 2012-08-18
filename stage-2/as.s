# as.s

# Copyright (C) 2011 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

# ########################################################################
# Mnemonic tables

# Fixed sized entries of form
#   struct mnemonic {
#     char name[12];  # Null terminated
#     char type;      # Types 00 to 05, or FF for directives; documented below
#     char params[3]; # Meaning dependent on type; documented below
#   };
#
# List of mnemonics is terminated by an entirely null marker.

mnemonics:

# Type 00 instructions.   Void instructions (i.e. with no arguments), e.g.
#
#   NOP                     90
#
# The three parameters are: 
#   1) the number of bytes in the op-code (01 here),
#   2) the first op-code byte (90), and 
#   3) the second op-code byte (n/a here).

.hex	4E 4F 50 00  00 00 00 00  00 00 00 00    00 01 90 00    # NOP
.hex	50 55 53 48  46 00 00 00  00 00 00 00    00 01 9C 00    # PUSHF
.hex	50 4F 50 46  00 00 00 00  00 00 00 00    00 01 9D 00    # POPF
.hex	52 45 50 00  00 00 00 00  00 00 00 00    00 01 F3 00    # REP
.hex	52 45 50 45  00 00 00 00  00 00 00 00    00 01 F3 00    # REPE
.hex	52 45 50 4E  45 00 00 00  00 00 00 00    00 01 F2 00    # REPNE
.hex	4D 4F 56 53  42 00 00 00  00 00 00 00    00 01 A4 00    # MOVSB
.hex	43 4D 50 53  42 00 00 00  00 00 00 00    00 01 A6 00    # CMPSB
.hex	53 43 41 53  42 00 00 00  00 00 00 00    00 01 AE 00    # SCASB
.hex	43 42 57 00  00 00 00 00  00 00 00 00    00 02 66 98    # CBW
.hex	43 57 44 45  00 00 00 00  00 00 00 00    00 01 98 00    # CWDE
.hex	43 44 51 00  00 00 00 00  00 00 00 00    00 01 99 00    # CDQ
.hex	52 45 54 00  00 00 00 00  00 00 00 00    00 01 C3 00    # RET
.hex	48 4C 54 00  00 00 00 00  00 00 00 00    00 01 F4 00    # HLT
.hex	4C 45 41 56  45 00 00 00  00 00 00 00    00 01 C9 00    # LEAVE


# Type 01 instructions.   A single immediate 8-bit argument, e.g.
#
#   INT     imm8            CD
#
# The three parameters are: 
#   1) the number of bytes in the op-code (01 here),
#   2) the first op-code byte (CD), and 
#   3) the second op-code byte (n/a here).

.hex	49 4E 54 00  00 00 00 00  00 00 00 00    01 01 CD 00    # INT


# Type 02 instructions.   A single immediate 32-bit argument, e.g.
#
#   CALL    rel32           E8
#
# The three parameters are: 
#   1) the number of bytes in the op-code (01 here),
#   2) the first op-code byte (E8), and 
#   3) the second op-code byte (n/a here).

.hex	43 41 4C 4C  00 00 00 00  00 00 00 00    02 01 E8 00    # CALL
.hex	4A 4D 50 00  00 00 00 00  00 00 00 00    02 01 E9 00    # JMP
.hex	4A 45 00 00  00 00 00 00  00 00 00 00    02 02 0F 84    # JE
.hex	4A 47 00 00  00 00 00 00  00 00 00 00    02 02 0F 8F    # JG
.hex	4A 47 45 00  00 00 00 00  00 00 00 00    02 02 0F 8D    # JGE
.hex	4A 41 00 00  00 00 00 00  00 00 00 00    02 02 0F 87    # JA
.hex	4A 41 45 00  00 00 00 00  00 00 00 00    02 02 0F 83    # JAE
.hex	4A 4C 00 00  00 00 00 00  00 00 00 00    02 02 0F 8C    # JL
.hex	4A 4C 45 00  00 00 00 00  00 00 00 00    02 02 0F 8E    # JLE
.hex	4A 42 00 00  00 00 00 00  00 00 00 00    02 02 0F 82    # JB
.hex	4A 42 45 00  00 00 00 00  00 00 00 00    02 02 0F 86    # JBE
.hex	4A 4E 45 00  00 00 00 00  00 00 00 00    02 02 0F 85    # JNE
.hex	4A 43 00 00  00 00 00 00  00 00 00 00    02 02 0F 82    # JC == JB
.hex	4A 4F 00 00  00 00 00 00  00 00 00 00    02 02 0F 80    # JO


# Type 03 instructions.   A single r/m8 operand, e.g.
#
#   INCB    r/m8            FE /0
#
# The three parameters are:
#   1) the number 1 (to allow it to be treated as the opcode length)
#   2) the op-code byte (FE), and 
#   3) the three-bit reg field for the ModR/M byte (0 here).

.hex	49 4E 43 42  00 00 00 00  00 00 00 00    03 01 FE 00    # INCB
.hex	44 45 43 42  00 00 00 00  00 00 00 00    03 01 FE 01    # DECB
.hex	4E 45 47 42  00 00 00 00  00 00 00 00    03 01 F6 03    # NEGB
.hex	53 41 4C 42  00 00 00 00  00 00 00 00    03 01 D2 04    # SALB
.hex	53 48 4C 42  00 00 00 00  00 00 00 00    03 01 D2 04    # SHLB
.hex	53 41 52 42  00 00 00 00  00 00 00 00    03 01 D2 07    # SARB
.hex	53 48 52 42  00 00 00 00  00 00 00 00    03 01 D2 05    # SHRB
.hex	4D 55 4C 42  00 00 00 00  00 00 00 00    03 01 F6 04    # MULB
.hex	49 4D 55 4C  42 00 00 00  00 00 00 00    03 01 F6 05    # IMULB
.hex	44 49 56 42  00 00 00 00  00 00 00 00    03 01 F6 06    # DIVB
.hex	49 44 49 56  42 00 00 00  00 00 00 00    03 01 F6 07    # IDIVB


# Type 04 instructions.   A single r/m32 operand, e.g.
#
#   INCL    r/m32           FF /0
#
# The three parameters are:
#   1) the number 1 (to allow it to be treated as the opcode length)
#   2) the op-code byte (FF), and 
#   3) the three-bit reg field for the ModR/M byte (0 here).

.hex	49 4E 43 4C  00 00 00 00  00 00 00 00    04 01 FF 00    # INCL
.hex	44 45 43 4C  00 00 00 00  00 00 00 00    04 01 FF 01    # DECL
.hex	4E 45 47 4C  00 00 00 00  00 00 00 00    04 01 F7 03    # NEGL
.hex	50 55 53 48  00 00 00 00  00 00 00 00    04 01 FF 06    # PUSH
.hex	50 4F 50 00  00 00 00 00  00 00 00 00    04 01 8F 00    # POP
.hex	53 41 4C 4C  00 00 00 00  00 00 00 00    04 01 D3 04    # SALL
.hex	53 48 4C 4C  00 00 00 00  00 00 00 00    04 01 D3 04    # SHLL
.hex	53 41 52 4C  00 00 00 00  00 00 00 00    04 01 D3 07    # SARL
.hex	53 48 52 4C  00 00 00 00  00 00 00 00    04 01 D3 05    # SHRL
.hex	4D 55 4C 4C  00 00 00 00  00 00 00 00    04 01 F7 04    # MULL
.hex	49 4D 55 4C  4C 00 00 00  00 00 00 00    04 01 F7 05    # IMULL
.hex	44 49 56 4C  00 00 00 00  00 00 00 00    04 01 F7 06    # DIVL
.hex	49 44 49 56  4C 00 00 00  00 00 00 00    04 01 F7 07    # IDIVL


# Type 05 instructions.   An r/m32 operand followed by a r32 operand:
#
#   LEA     r/m32, r32      8D
#
# The three parameters are: 
#   1) the number of bytes in the op-code (01 here),
#   2) the first op-code byte (8D), and 
#   3) the second op-code byte (n/a here).

.hex	4C 45 41 00  00 00 00 00  00 00 00 00    05 01 8D 00    # LEA


# Type 06 instructions.   These represent a large family of op-codes, e.g.
#
#   MOVB    r8, r/m8        88         } These two opcodes are constrained
#   MOVB    r/m8, r8        8A         } to $N and $N+2.
#   MOVB    imm8, r/m8      C6 /0
#
# The three parameters are:
#   1) the r8, r/m8 op code (88 here), the 
# immediate op code (C6 here), and the three op-code extension bits
# that are in the reg part of the mod-r/m byte (0 here).

.hex	4D 4F 56 42  00 00 00 00  00 00 00 00    06 88 C6 00    # MOVB
.hex	41 44 44 42  00 00 00 00  00 00 00 00    06 00 80 00    # ADDB
.hex	53 55 42 42  00 00 00 00  00 00 00 00    06 28 80 05    # SUBB
.hex	43 4D 50 42  00 00 00 00  00 00 00 00    06 38 80 07    # CMPB
.hex	41 4E 44 42  00 00 00 00  00 00 00 00    06 20 80 04    # ANDB
.hex	4F 52 42 00  00 00 00 00  00 00 00 00    06 08 80 01    # ORB
.hex	58 4F 52 42  00 00 00 00  00 00 00 00    06 30 80 06    # XORB


# Type 07 instructions.   These represent a large family of op-codes, e.g.
#
#   MOVL    r32,r/m32       89         } These two opcodes are constrained
#   MOVL    r/m32,r32       8B         } to $N and $N+2.
#   MOVL    imm32,r/m32     C7 /0
#
# The three parameters are the register op code (88 here), the 
# immediate op code (C6 here), and the three op-code extension bits
# that are in the reg part of the mod-r/m byte (0 here).

.hex	4D 4F 56 4C  00 00 00 00  00 00 00 00    07 89 C7 00    # MOVL
.hex	41 44 44 4C  00 00 00 00  00 00 00 00    07 01 81 00    # ADDL
.hex	53 55 42 4C  00 00 00 00  00 00 00 00    07 29 81 05    # SUBL
.hex	43 4D 50 4C  00 00 00 00  00 00 00 00    07 39 81 07    # CMPL
.hex	41 4E 44 4C  00 00 00 00  00 00 00 00    07 21 81 04    # ANDL
.hex	4F 52 4C 00  00 00 00 00  00 00 00 00    07 09 81 01    # ORL
.hex	58 4F 52 4C  00 00 00 00  00 00 00 00    07 31 81 06    # XORL


# Type FF 'instructions'.  These are actually directives.    
#
#   .hex
#
# The sole parameter is an unique code used internally to represent the 
# directive.  Each directive is special-cased in the code.

.hex	2E 68 65 78  00 00 00 00  00 00 00 00    FF 00 00 00    # .hex



# End of table marker -- first byte of name is NULL.
.hex	00 00 00 00  00 00 00 00  00 00 00 00    00 00 00 00 
# ########################################################################


####    #  Function: bool ishws(char)
	#  Tests whether its argument is in [ \t]
ishws:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	8(%ebp), %eax
.L1a:
	CMPB	$0x20, %al	# ' '
	JE	.L1
	CMPB	$0x09, %al	# '\t'
	JE	.L1
	XORL	%eax, %eax
.L1:
	POP	%ebp
	RET

####    #  Function: bool isws(char)
	#  Tests whether its argument is in [ \t\r\n]
isws:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	8(%ebp), %eax
	CMPB	$0x0D, %al	# '\r'
	JE	.L1
	CMPB	$0x0A, %al	# '\n'
	JE	.L1
	JMP	.L1a


####	#  Function: bool isidchr(char)
	#  Tests whether its argument is in [0-9A-Za-z_]
isidchr:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	8(%ebp), %eax
	CMPB	$0x30, %al	# '0'
	JL	.L2
	CMPB	$0x39, %al	# '9'
	JLE	.L3
.L3a:
	CMPB	$0x41, %al	# 'A'
	JL	.L2
	CMPB	$0x5A, %al	# 'Z'
	JLE	.L3
	CMPB	$0x5F, %al	# '_'
	JE	.L3
	CMPB	$0x61, %al	# 'a'
	JL	.L2
	CMPB	$0x7A, %al	# 'z'
	JLE	.L3
.L2:
	XORL	%eax, %eax
.L3:
	POP	%ebp
	RET


####	#  Function: bool isid1chr(char)
	#  Tests whether its argument is in [.A-Za-z_]
isid1chr:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	8(%ebp), %eax
	CMPB	$0x2E, %al	# '.'
	JE	.L3
	JMP	.L3a


####	#  Function: int xchr(int c)
	#  Tests whether its argument, c, is a character in [0-9A-F], and if 
	#  so, coverts it to an integer; otherwise returns -1.
xchr:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	8(%ebp), %eax
	CMPB	$0x30, %al	# '0'
	JL	.L4
	CMPB	$0x39, %al	# '9'
	JLE	.L5
	CMPB	$0x41, %al	# 'A'
	JL	.L4
	CMPB	$0x46, %al	# 'F'
	JG	.L4
	SUBB	$0x37, %al	# 'A'-10
	JMP	.L7
.L4:
	MOVB	$-1, %al
	JMP	.L7
.L5:
	SUBB	$0x30, %al	# '0'
.L7:
	POP	%ebp
	RET


####	#  Function: int dchr(int c)
	#  Tests whether its argument, c, is a character in [0-9], and if 
	#  so, coverts it to an integer; otherwise returns -1.
dchr:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	8(%ebp), %eax
	CMPB	$0x30, %al	# '0'
	JL	.L4
	CMPB	$0x39, %al	# '9'
	JLE	.L5
	JMP	.L4


####    #  Not a proper function.
	#  Exits program
error:
	MOVL    $1, %ebx
success:
	MOVL    $1, %eax   # 1 == __NR_exit
	INT     $0x80


####	#  Function:	void writebyte( int c, ofile* of )
	#  Write out character c
writebyte:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$1, %edx
.L5a:
	PUSH	%ebx

	#  Increment counter
	MOVL	12(%ebp), %ebx
	ADDL	%edx, 4(%ebx)	# inc counter

	#  If the fd is -ve, we're not actually writing
	MOVL	(%ebx), %ebx	# fd from ofile
	CMPL	$0, %ebx
	JL	.L5d

	LEA	8(%ebp), %ecx	# ofile*
	MOVL	$4, %eax   # 4 == __NR_write
	INT	$0x80
.L5d:
	POP	%ebx
	POP	%ebp
	RET



####	#  Function:	void writedword( int dw, ofile* of )
	#  Write out dword dw
writedword:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$4, %edx
	JMP	.L5a


####	#  Function:	void writedata( int bits, int data, ofile* of )
writedata:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	16(%ebp)	# ofile
	PUSH	12(%ebp)	# data
	
	CMPL	$8, 8(%ebp)
	JE	.L5b
	CALL	writedword
	JMP	.L5c
.L5b:
	CALL	writebyte
.L5c:
	LEAVE
	RET

	

####	#  Function:	void unread( char c, ifile* f )
	#  Puts c onto the pushback slot
unread:
	PUSH	%ebp
	MOVL	%esp, %ebp

	#  Locate the pback_slot.
	MOVL	12(%ebp), %eax
	CMPB	$0, 4(%eax)  # Test has_pback?
	JNE	error

	#  Write to the pback_slot
	MOVB	$1, 4(%eax)
	MOVB	8(%ebp), %cl
	MOVB	%cl, 5(%eax)
	POP	%ebp
	RET
	

####	#  Function:	void readone( char* p, ifile* f ) 
	#  Reads one byte into p which should already be set.
	#  Returns the number of bytes read in %eax.
	#
	#  This function contains the sole read syscall (syscall #3) so we 
	#  can plumb in use of the pushback slot (pback_slot)
readone:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%ebx
	MOVL	8(%ebp), %ecx

	#  Locate the pback_slot.
	MOVL	12(%ebp), %ebx
	CMPB	$0, 4(%ebx)  # Test has_pback?
	JE	.L9

	#  The pback_slot has a value in it
	MOVB	$0, 4(%ebx)
	MOVB    5(%ebx), %al
	MOVB	%al, (%ecx)
	MOVL	$1, %eax
	JMP	.L9a

.L9:
	MOVL	$1, %edx
	MOVL	(%ebx), %ebx	# fd from ifile
	MOVL	$3, %eax	# 3 = __NR_read
	INT	$0x80
.L9a:
	POP	%ebx
	POP	%ebp
	RET
	

####	#  Function:	void readonex( char* p, ifile* f ) 
	#  Reads one byte into p which should already be set (as above);
	#  exits unsuccessfully in case of error reading.
readonex:
	PUSH	%ebp
	MOVL	%esp, %ebp

	PUSH	12(%ebp)
	PUSH	8(%ebp)
	CALL	readone
	POP	%edx
	POP	%edx

	CMPL	$1, %eax
	JNE	error

	POP	%ebp
	RET


####	#  Function:	int getone( ifile* in )
	#  Reads a byte into a temporary buffer (which is freed again) and 
	#  returns the byte.  Exits on error or failure to read.
getone:
	PUSH	%ebp
	MOVL	%esp, %ebp
	XORL	%ecx, %ecx
	PUSH	%ecx		# char buf[4];

	PUSH	8(%ebp)
	LEA	-4(%ebp), %ecx	# char* bufp;
	PUSH	%ecx

	CALL	readonex
	MOVB	-4(%ebp), %al

	LEAVE
	RET


####	#  Function:	int read_r32( ifile* in )
	#  We've already read a '%'.  Now read a 32-bit register name.  Return
	#    0 => %eax, 1 => %ecx, 2 => %edx, 3 => %ebx
	#    4 => %esp, 5 => %ebp, 6 => %esi, 7 => %edi
.L9h:
	MOVL	%ecx, %eax
	POP	%edx		# value
	POP	%edx		# ifile
	POP	%ebp
	RET

.L9g:
	#  Is it %esp or %esi?
	MOVL	%ecx, -4(%ebp)	# value
	CALL	getone
	MOVL	-4(%ebp), %ecx	# value
	CMPB	$0x70, %al	# 'p'
	JE	.L9h
	CMPB	$0x69, %al	# 'i'
	JNE	error
	MOVL	$6, %ecx	# == %esi
	JMP	.L9h

.L9f:
	#  Is it %edi?
	CMPB	$0x69, %al	# 'i'
	JNE	error
	MOVL	$7, %ecx	# == %edi
	JMP	.L9h
	
.L9e:
	#  We've had %e[a-d][^x]
	CMPL	$2, %ecx
	JL	error		# %ea* / %ec*
	JE	.L9f		# %ed*

	#  Is it %ebp?
	CMPB	$0x70, %al	# 'p'
	JNE	error
	MOVL	$5, %ecx	# == %ebp
	JMP	.L9h
	
.L9d:	
	#  Have parsed two characters of %eax or %ecx, check for 'x'
	MOVL	%ecx, -4(%ebp)	# value
	CALL	getone
	MOVL	-4(%ebp), %ecx	# value
	CMPB	$0x78, %al	# 'x'
	JNE	.L9e
	JMP	.L9h

read_r32:
	PUSH	%ebp
	MOVL	%esp, %ebp
	XORL	%ecx, %ecx
	PUSH	%ecx		# value

	#  Read the first character which must be an 'e'
	PUSH	8(%ebp)		# ifile
	CALL	getone
	CMPB	$0x65, %al	# 'e'
	JNE	error

	#  Read the second character
	CALL	getone
	MOVL	-4(%ebp), %ecx	# value
	CMPB	$0x61, %al	# 'a'
	JE	.L9d
	INCL	%ecx
	CMPB	$0x63, %al	# 'c'
	JE	.L9d
	INCL	%ecx
	CMPB	$0x64, %al	# 'd'
	JE	.L9d
	INCL	%ecx
	CMPB	$0x62, %al	# 'b'
	JE	.L9d
	INCL	%ecx
	CMPB	$0x73, %al	# 's'
	JE	.L9g
	JMP	error


####	#  Function:	int read_r8( ifile* in )
	#  We've already read a '%'.  Now read an 8-bit register name.  Return
	#    0 => %al,  1 => %cl,  2 => %dl,  3 => %bl
	#    4 => %ah,  5 => %ch,  6 => %dh,  7 => %bh
.L9c:
	MOVL	%ecx, %eax
	POP	%edx		# value
	POP	%edx		# ifile
	POP	%ebp
	RET

.L9b:
	#  Read and parse the second character
	MOVL	%ecx, -4(%ebp)	# value
	CALL	getone
	MOVL	-4(%ebp), %ecx	# value
	CMPB	$0x6C, %al	# 'l'
	JE	.L9c
	ADDL	$4, %ecx
	CMPB	$0x68, %al	# 'h'
	JE	.L9c
	JMP	error

read_r8:
	PUSH	%ebp
	MOVL	%esp, %ebp
	XORL	%ecx, %ecx
	PUSH	%ecx		# value

	#  Read the first character
	PUSH	8(%ebp)		# ifile
	CALL	getone
	MOVL	-4(%ebp), %ecx	# value

	#  Parse the first character
	CMPB	$0x61, %al	# 'a'
	JE	.L9b
	INCL	%ecx
	CMPB	$0x63, %al	# 'c'
	JE	.L9b
	INCL	%ecx
	CMPB	$0x64, %al	# 'd'
	JE	.L9b
	INCL	%ecx
	CMPB	$0x62, %al	# 'b'
	JE	.L9b
	JMP	error


####	#  Function:	int read_reg( int bits, ifile* in )
	#  Read a 8- or 32-bit register name, depending on BITS.  The
	#  leading '%' must already be read.
.L13:
	CALL	read_r8
.L13a:
	POP	%ecx
	POP	%ebp
	RET

read_reg:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	12(%ebp)	# ifile

	CMPL	$8, 8(%ebp)
	JE	.L13
	CALL	read_r32
	JMP	.L13a

####	#  Function:	int skiphws( ifile* );
	#  Skip horizontal whitespace until a non-whitespace character 
	#  which is unread and returned.
skiphws:
	#  The function entry point
	PUSH	%ebp
	MOVL	%esp, %ebp

	#  Read one byte 
	PUSH	8(%ebp)		# ifile
.L15:
	CALL	getone

	#  Is the byte horizontal white space?  If so, loop back
	PUSH	%eax
	CALL	ishws
	CMPL	$0, %eax
	POP	%eax
	JNE	.L15

	#  Unread the last character read
	PUSH	%eax
	CALL	unread
	POP	%eax

	#  Stack cleanup and exit
	POP	%edx
	POP	%ebp
	RET
	

####	#  Function:	int read_int( int bits, ifile* in );
	#  Read an integer: either decimal (which must not have a leading 0),
	#  or hex (which must have an 0x prefix).  Error on overflow.
.L17:
	#  Have we read any digits?  If not, it's an error
	CMPL	$0, -8(%ebp)
	JE	error

.L17a:
	#  Unread the last character read (now in %edx)
	PUSH	%edx
	CALL	unread
	POP	%edx		# char

.L17b:
	#  Stack cleanup and exit
	POP	%edx		# ifile
	POP	%edx		# count
	POP	%eax		# ret val
	POP	%ebp
	RET

.L17c:
	#  Have we an overflow?  Don't care if bits == 32
	CMPB	$32, 8(%ebp)
	JE	.L17

	#  This branch is only used for decimals and before applying the
	#  minus sign.   We only test whether it's >= 2^bits because 
	#  we don't know whether 200 is a valid unsigned 8-bit number or
	#  an invalid signed 8-bit number.
	MOVL	$1, %eax
	MOVL	8(%ebp), %ecx
	SALL	%eax		# by %cl
	CMPL	%eax, -4(%ebp)
	JAE	error

	JMP	.L17

.L16:
	#  We're reading decimal and the first digit is already in %eax
	PUSH	%eax
	CALL	dchr
	POP	%edx		# char
	CMPB	$-1, %al
	JE	.L17		# done
	
	#  Add the digit to the current value -- tedious as MUL only
	#  acts on the accumulator and cannot read immediates.  Test for
	#  overflow as we go along by using the fact that MUL and ADDL
	#  are both signed operations which set the OF flag on overflow.
	PUSH	%eax		# this digit
	MOVL	-4(%ebp), %eax	# value so far
	MOVL	$10, %ecx
	MULL	%ecx		# acts on %edx:%eax	val *= 10
	JO	error
	POP	%edx		# digit
	ADDL	%edx, %eax	# val += digit
	JO	error
	MOVL	%eax, -4(%ebp)	# store val
	INCL	-8(%ebp)	# inc counter

	#  Read another digit
	CALL	getone
	JMP	.L16		# loop

.L16b:
	#  This would be where we'd consider octal, if we cared about it.
	#  We don't, but we do care about the special case of zero, so we
	#  assume anything begining '0' but not continuing 'x' is a plain
	#  zero.  We'll get an error parsing the next token if we were wrong.
	CALL	getone
	CMPB	$0x78, %al	# 'x'
	MOVL	%eax, %edx
	JNE	.L17a

	#  Set number of hex digits (divide number of bits by 4)
	MOVB	$2, %cl
	SARL	8(%ebp)		# by %cl

.L18:
	#  We know we're going to have a hex number, so start reading
	CALL	getone
	PUSH	%eax
	CALL	xchr
	POP	%edx		# char
	CMPB	$-1, %al
	JE	.L17	# done

	#  Add the digit to the current value
	MOVB	$4, %cl
	SALL	-4(%ebp)	# by %cl
	ADDL	%eax, -4(%ebp)
	INCL	-8(%ebp)

	#  Test for overflow: 
	#  have we read more than the expected number of digits?
	MOVL	8(%ebp), %eax
	CMPL	%eax, -8(%ebp)
	JG	error
	JMP	.L18	# loop

.L18a:
	NEGL	%eax
	MOVL	%eax, -4(%ebp)	# save
	JMP	.L17b

read_int:
	#  The function entry point
	PUSH	%ebp
	MOVL	%esp, %ebp
	XORL	%ecx, %ecx
	PUSH	%ecx		# int val = 0;
	PUSH	%ecx		# int count = 0;

	#  Have as got an '0' followed by an 'x'?
	PUSH	12(%ebp)	# ifile
	CALL	getone
	CMPB	$0x30, %al	# '0'
	JE	.L16b		# hex / zero
	CMPB	$0x2D, %al	# '-'
	JNE	.L16		# deciml

	#  Recurse to read_int and negate the answer
	PUSH	12(%ebp)
	PUSH	8(%ebp)
	CALL	read_int
	POP	%edx
	POP	%edx

	#  Have we an overflow?  Don't care if bits == 32
	CMPB	$32, 8(%ebp)
	JE	.L18a

	#  This branch is only used for decimals and before applying the
	#  minus sign.   We only test whether it's >= 2^bits because 
	#  we don't know whether 200 is a valid unsigned 8-bit number or
	#  an invalid signed 8-bit number.
	MOVL	$1, %edx
	MOVL	8(%ebp), %ecx
	DECL	%ecx
	SALL	%edx		# by %cl
	CMPL	%edx, %eax
	JA	error
	JMP	.L18a


####	#  Function:	int read_rm( int bits, int* disp, ifile* in )
	#  Skip whitespace then read an r/m8 or r/m32 depending on BITS.  
	#  Returns the ModR/M byte and if a displacement is present, stores 
	#  it in disp.
.L15b:
	#  It's not a '(' or '%' so it must be the start of a number
	PUSH	%eax
	CALL	unread
	POP	%edx
	MOVL	$32, %eax	# bits
	PUSH	%eax
	CALL	read_int
	POP	%edx
	MOVL	12(%ebp), %ecx	# int* disp32
	MOVL	%eax, (%ecx)	# store disp
	
	#  We now must have a '('
	CALL	getone
	CMPB	$0x28, %al	# '('
	JNE	error

	#  Save 0x80 for |= later
	MOVL	$0x80, -8(%ebp)

.L15a:
	#  If we've jumped here directly, -8(%ebp) [or val] is zero.
	#  Have read a '('; now we must read a '%'.  (ifile already on stack.)
	CALL	getone
	CMPB	$0x25, %al	# '%'
	JNE	error

	CALL	read_r32
	MOVL	%eax, -4(%ebp)	# save

	#  Register code is in -4(%ebp).  Skip ')'
	CALL	getone
	CMPB	$0x29, %al	# ')'
	JNE	error

.L15c:
	#  Clean up stack and |= with or val in -8(%ebp)
	POP	%edx		# ifile
	POP	%edx		# or val
	POP	%eax		# ret val
	ORB	%dl, %al

	#  0x04 == (%esp), 0x05 == (%ebp) and 0x84 == DISP(%esp) are invalid
	CMPB	$0x04, %al
	JE	error
	CMPB	$0x05, %al
	JE	error
	CMPB	$0x84, %al
	JE	error

	POP	%ebp
	RET

read_rm:
	PUSH	%ebp
	MOVL	%esp, %ebp
	XORL	%ecx, %ecx
	PUSH	%ecx		# for ret val
	PUSH	%ecx		# for or val

	#  What sort of argument is it?  Direct memory accesses will begin '(',
	#  registers will start with an '%', and memory accesses with
	#  a displacement will start with a number?
	PUSH	16(%ebp)	# ifile
	CALL	skiphws
	CALL	getone
	CMPB	$0x28, %al	# '('
	JE	.L15a
	CMPB	$0x25, %al	# '%'
	JNE	.L15b

	#  Save 0xC0 for |= later
	MOVL	$0xC0, -8(%ebp)

	#  Read the 32-bit register name (ifile is still on stack)
	PUSH	8(%ebp)		# bits
	CALL	read_reg
	POP	%ecx
	MOVL	%eax, -4(%ebp)	# save
	JMP	.L15c

	
####	#  Function:	int get_label( int strlen, int bits, main_frame* p ):
	#  Lookup the text in p->buffer (which has known length strlen),
	#  Convert it to an offset relative to the current instruction
	#  (we're assumed to have written the opcode and any ModR/M bytes,
	#  so the end of instruction happens after writing the number of bits
	#  in the BITS parameter).  Check the offset can be expressed as a
	#  BITS-bit signed (2's-complement) integer.
.L11b:
	#  The label wasn't found.  Only allow this if we're not writing.
	CMPL	$0, -4208(%ebx)
	JG	error
	XORL	%eax, %eax

.L11a:
	POP	%edi
	POP	%esi
	POP	%ebx
	POP	%ebp
	RET

get_label:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%ebx
	PUSH	%esi
	PUSH	%edi

	MOVL	8(%ebp), %ecx	# strlen
	MOVL	16(%ebp), %ebx
	LEA	-96(%ebx), %esi
	LEA	-4208(%ebx), %edi

	#  Null terminate
	MOVL	%esi, %edx
	ADDL	%ecx, %edx
	XORL	%eax, %eax
	MOVL	%eax, (%edx)
	INCL	%ecx

	PUSH	%ecx
.L11:
	POP	%ecx

	ADDL	$16, %edi
	CMPL	-8(%ebx), %edi
	JGE	.L11b
	PUSH	%ecx
	PUSH	%esi
	PUSH	%edi
	REPE CMPSB
	POP	%edi
	POP	%esi
	JNE	.L11
	CMPL	$0, %ecx
	JNE	.L11
	POP	%ecx

	#  Found it.  Compute the value.
	MOVL	12(%ebp), %eax
	MOVB	$3, %cl
	SARL	%eax		# by %cl
	ADDL	-4204(%ebx), %eax #ofile->count
	SUBL	12(%edi), %eax
	NEGL	%eax

	#  %eax is the offset.  If bits == 32, don't care about overflow
	CMPL	$32, 12(%ebp)
	JE	.L11a
	
	#  Is it >= 2^(bits-1) ?
	MOVL	12(%ebp), %ecx
	DECL	%ecx
	MOVL	$1, %edx
	SALL	%edx		# by %cl
	CMPL	%eax, %edx
	JLE	error

	#  Or < 2^(bits-1)?
	NEGL	%edx
	CMPL	%eax, %edx
	JG	error
	JMP	.L11a


####	#  Function:	char* read_id( main_frame* p );
	#  The first byte of an identifier is already p->buf[0].  We 
	#  first confirm it isid1chr() and abort if not.  Then read 
	#  until we reach the end of the id and return a pointer to 
	#  the first non-id char.
read_id:
	PUSH	%ebp
	MOVL	%esp, %ebp

	#  Was the first byte a valid character for an identifier?
	MOVL	8(%ebp), %ecx
	PUSH	-96(%ecx)
	CALL	isid1chr
	POP	%ecx
	CMPL	$0, %eax
	JE	error

	#  Continue to read an identifier
	MOVL	8(%ebp), %eax
	LEA	-4200(%eax), %ecx
	PUSH	%ecx
	LEA	-96(%eax), %ecx
.L12:
	INCL	%ecx
	#  We need to check that we're not going to overflow the buffer.  
	#  But in fact, the instruction table limit is 12 characters (incl.
	#  null termination -- i.e. 11 without), so we use that limit.
	MOVL	8(%ebp), %eax
	LEA	-96(%eax), %edx
	SUBL	%ecx, %edx
	CMPL	$-11, %edx
	JL	error

	PUSH	%ecx
	CALL	readonex
	POP	%ecx
	PUSH	%ecx
	PUSH	(%ecx)
	CALL	isidchr
	POP	%edx
	POP	%ecx		# restore %ecx
	CMPL	$0, %eax
	JNE	.L12		# Loop
	POP	%edx

	#  (%ecx) is now something other than lchr.  Return it
	MOVL	%ecx, %eax
	POP	%ebp
	RET



####	#  Function:	int read_imm( int bits, main_frame* p );
	#  Skip whitespace and then read an immediate value ($0xXX or $NNN)
	#  Returns the value read or exits if unable to read.

.L16a:
	#  Put the byte into the main buffer and read a label
	MOVB	-4(%ebp), %al
	MOVL	12(%ebp), %ecx
	MOVB	%al, -96(%ecx)

	PUSH	12(%ebp)	# main_frame
	CALL	read_id
	POP	%ecx

	#  Get ready to unread the last byte back at the end of the function
	MOVL	(%eax), %ecx
	MOVL	%ecx, -4(%ebp)

	#  Calculate the string length
	MOVL	12(%ebp), %ecx
	LEA	-96(%ecx), %edx
	SUBL	%edx, %eax

	PUSH	12(%ebp)	# main_frame
	PUSH	8(%ebp)		# bits
	PUSH	%eax		# strlen
	CALL	get_label
	POP	%ecx
	POP	%ecx
	POP	%ecx
	MOVL	%eax, -8(%ebp)

	#  Unread the last character read
	POP	%ecx		# Overwrite 
	PUSH	-4(%ebp)	#   bufp
	CALL	unread

	#  Stack cleanup and exit
	MOVL	-8(%ebp), %eax	# return val
	LEAVE
	RET

read_imm:
	#  The function entry point
	PUSH	%ebp
	MOVL	%esp, %ebp
	XORL	%ecx, %ecx
	PUSH	%ecx		# char buf[4];
	PUSH	%ecx		# int val = 0;
	PUSH	%ecx		# int count = 0;

	#  Skip horizontal whitespace
	MOVL	12(%ebp), %ecx
	LEA	-4200(%ecx), %ecx
	PUSH	%ecx		# ifile
	CALL	skiphws

	#  Check it is a '$' marking the start of an immediate;
	#  if not, it must be an identifier
	LEA	-4(%ebp), %ecx
	PUSH	%ecx		# char* bufp
	CALL	readonex
	CMPB	$0x24, -4(%ebp)	# '$'
	JNE	.L16a

	#  It's an integer
	POP	%edx		# bufp
	PUSH	8(%ebp)		# bits
	CALL	read_int

	#  Stack clean-up and exit
	LEAVE
	RET


####	#  Function:	void write_oc12( int opcode_info, main_frame* )
	#  Uses the second byte in opcode_info to determine how many
	#  bytes to write (1 or 2) and then writes the third and perhaps
	#  fourth byte out.
.L19:
	POP	%ebx	# ofile
	POP	%ebx	# Restore %ebx
	POP	%ebp
	RET

write_oc12:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%ebx	# Save %ebx

	#  Find the ofile object
	MOVL	12(%ebp), %eax
	LEA	-4208(%eax), %ecx
	PUSH	%ecx

	#  Unconditionally write the first byte
	MOVL	8(%ebp), %edx
	MOVL	%edx, %ebx
	MOVB	$16, %cl
	SHRL	%ebx		# by %cl
	PUSH	%ebx
	CALL	writebyte
	POP	%ebx

	#  Is there a second byte?
	MOVL	8(%ebp), %edx
	CMPB	$1, %dh
	JLE	.L19
	MOVB	$8, %cl
	SHRL	%ebx		# by %cl
	PUSH	%ebx
	CALL	writebyte
	POP	%ebx
	JMP	.L19


####	#  Function:	void write_modrm( int rm, int reg, int disp, ofile* )
.L20:
	POP	%edx		# ofile
	POP	%ebp
	RET

write_modrm:
	PUSH	%ebp
	MOVL	%esp, %ebp

	#  Or reg (or opcode) bits <<3 with rm and put in %al
	MOVL	8(%ebp), %eax	# rm
	MOVL	12(%ebp), %edx	# reg
	MOVB	$3, %cl
	SHLL	%edx		# by %cl
	ORB	%dl, %al

	#  Write the ModR/M byte
	PUSH	20(%ebp)	# ofile
	PUSH	%eax		# byte
	CALL	writebyte
	POP	%eax

	#  Do we have a disp32?  NB: We don't support disp8s
	CMPB 	$0x80, %al
	JB	.L20
	CMPB	$0xC0, %al
	JAE	.L20

	#  Yes.   ofile already on stack
	PUSH	16(%ebp)		# disp
	CALL	writedword
	POP	%ebx
	JMP	.L20

	
####    #  The main function.
	#  Stack is arranged as follows:
	#
	#	-4(%ebp)	instrct* mnemonics
	#       -8(%ebp)	label* label_end
	#      -12(%ebp)	label* label_end_store
	#      -96(%ebp)	char buffer[80]
	#    -4192(%ebp)	label labels[64]
	#    -4200(%ebp)        ifile fin
	#    -4208(%ebp)	ofile fout
	#
	#  where label is a { char name[12]; int addr; },
	#  instrct is a { char name[12]; char type; char data[3]; },
	#  ifile is a { int fd; bool has_pback :8; char pback_char; int:16; }.
	#  and ofile is a { int fd; int count; }.

ret:
	#  This ret is labelled to allow various bits of main to
	#  jump up to it in order to effect a forwards jump.
	XORL	%eax, %eax
ret1:
	RET

	#  --- Test for a comment.
	#  If found, skip over comment line until we've read a LF
	#  At end of section, %eax=1 iff we read a comment.
	#  If %eax=0, all other registers are unaltered.
comment:
	CMPB    $0x23, -96(%ebp)        # '#'
	JNE	ret
	LEA	-4200(%ebp), %ecx	# ifile
	PUSH	%ecx
	LEA	-96(%ebp), %ecx
	PUSH	%ecx
.L10:
	CALL	readonex
	CMPB	$0x0A, -96(%ebp)	# '\n'
	JNE	.L10
	POP	%edx
	POP	%edx
	MOVL	$1, %eax
	RET	# to main loop

	#  Parts of the label section
labeldef:
	#  Check that we're not about to over run the label store,
	#  and then store the label
	LEA	-8(%ebp), %ebx
	MOVL	(%ebx), %edi
	MOVL	-12(%ebp), %eax
	CMPL	%eax, %edi
	JGE	error
	REP MOVSB
	MOVL	-4204(%ebp), %eax #ofile->count
	MOVL	(%ebx), %edi
	MOVL	%eax, 12(%edi)
	ADDL	$16, (%ebx)
	MOVL	$1, %eax
	RET	# to main loop

insn_end:
	MOVL	$2, %eax
	JMP	ret1

type_00:
	#  Write out a type 00 instruction.  These are easiest as they
	#  have no parameters.
	PUSH	%ebp		# main_frame
	PUSH	%edx	# opcode info
	CALL	write_oc12
	POP	%edx
	POP	%edx
	JMP	insn_end

type_01:
	#  Write the opcode byte(s).
	PUSH	%ebp		# main_frame
	PUSH	%edx	# opcode info
	CALL	write_oc12
	POP	%edx

	#  Skip ws and read immediate.
	PUSH	%ebx		# bits
	CALL	read_imm
	POP	%ecx
	POP	%ecx

	#  Byte is in %al (as part of return from read_imm): write it.
	LEA	-4208(%ebp), %ecx	# ofile
	PUSH	%ecx
	PUSH	%eax
	PUSH	%ebx
	CALL	writedata
	POP	%edx
	POP	%edx
	POP	%edx
	JMP	insn_end

type_03:
	#  Write the opcode byte(s).
	PUSH	%ebp		# main_frame
	PUSH	%edx	# opcode info
	CALL	write_oc12
	POP	%edx
	POP	%ecx

	#  Read the r/m
	PUSH	%edx	# store opcode info
	LEA	-4200(%ebp), %ecx	# ifile
	PUSH	%ecx
	LEA	-96(%ebp), %ecx		# disp
	PUSH	%ecx
	PUSH	%ebx		# bits
	CALL	read_rm
	ADDL	$12, %esp
	POP	%edx

.L21:
	#  Write the ModR/M byte and (if present) displacement.
	LEA	-4208(%ebp), %ecx	# ofile
	PUSH	%ecx
	PUSH	-96(%ebp)	# disp
	MOVB	$24, %cl
	SHRL	%edx		# by %cl
	PUSH	%edx		# reg bits
	PUSH	%eax		# modrm
	CALL	write_modrm
	ADDL	$16, %esp
	JMP	insn_end

type_05:
	#  Write the opcode byte(s).
	PUSH	%ebp		# main_frame
	PUSH	%edx	# opcode info
	CALL	write_oc12
	POP	%edx
	POP	%ecx

.L22:
	#  Read the r/m
	LEA	-4200(%ebp), %ecx	# ifile
	PUSH	%ecx
	LEA	-96(%ebp), %ecx		# disp
	PUSH	%ecx
	PUSH	%ebx			# bits
	CALL	read_rm
	POP	%ebx			# bits
	POP	%ecx
	POP	%ecx

	# Skip ws, read a comma, then skip more ws and read the '%' and reg
	PUSH	%eax	# store r/m
	LEA	-4200(%ebp), %ecx	# ifile
	PUSH	%ecx
	CALL	skiphws
	CALL	getone
	CMPB	$0x2C, %al	# ','
	JNE	error
	CALL	skiphws
	CALL	getone
	CMPB	$0x25, %al	# '%'
	JNE	error
	PUSH	%ebx		# bits
	CALL	read_reg
	POP	%edx	# bits
	POP	%edx	# ifile
	POP	%edx	# r/m

.L22a:
	#  Write the ModR/M byte and (if present) displacement.
	#  So: %edx is r/m, %eax is reg
	LEA	-4208(%ebp), %ecx	# ofile
	PUSH	%ecx
	PUSH	-96(%ebp)	# disp
	PUSH	%eax		# reg
	PUSH	%edx		# rm
	CALL	write_modrm
	ADDL	$16, %esp
	JMP	insn_end

type_06_rm:
	#  %edx contains the opcode info, so %dh+2 is the opcode.  Write it.
	LEA	-4208(%ebp), %ecx
	PUSH	%ecx
	MOVB	%dh, %al
	ADDL	$2, %eax
	PUSH	%eax
	CALL	writebyte
	POP	%ecx
	POP	%ecx

	JMP	.L22

type_06_reg:
	#  %edx contains the opcode info, so %dh is the opcode.  Write it.
	LEA	-4208(%ebp), %ecx
	PUSH	%ecx
	MOVB	%dh, %al
	PUSH	%eax
	CALL	writebyte
	POP	%ecx
	POP	%ecx

.L23:
	#  Read the register
	LEA	-4200(%ebp), %ecx	# ifile
	PUSH	%ecx
	CALL	skiphws
	CALL	getone
	CMPB	$0x25, %al	# '%'
	JNE	error
	PUSH	%ebx		# bits
	CALL	read_reg
	POP	%ebx			# bits
	POP	%ecx

.L23a:
	#  Skip ws, read a comma, then skip more ws and read the '%' and r/m
	#  We expect %eax to contain the reg bytes already read, and
	#  %ebx the number of bits.
	PUSH	%eax	# store reg
	LEA	-4200(%ebp), %ecx	# ifile
	PUSH	%ecx
	CALL	skiphws
	CALL	getone
	CMPB	$0x2C, %al	# ','
	JNE	error
	LEA	-96(%ebp), %ecx		# disp
	PUSH	%ecx
	PUSH	%ebx			# bits
	CALL	read_rm
	MOVL	%eax, %edx
	POP	%ebx		# bits
	POP	%ecx		# disp
	POP	%ecx		# ifile
	POP	%eax	# retrieve reg

	#  Write the ModR/M byte.
	JMP	.L22a

.L23b:
	#  This is the easy case: the immediate is a literal
	PUSH	%edx		# store op info
	PUSH	%ebp
	PUSH	%ebx		
	CALL	read_imm
	POP	%ecx
	POP	%ecx
	POP	%edx		# opinfo

	LEA	-4208(%ebp), %ecx
	PUSH	%ecx		# ofile
	PUSH	%eax		# data
	PUSH	%ebx		# bits

	#  Messy.  Call into the {.L23a, .L22a, insn_end, ret1} block.
	#  That isn't a real function.   It expects %al to contain reg bits
	MOVB	%dh, %al
	CALL	.L23a

	#  Now write the immediate.  The stack is already set up.
	CALL	writedata
	POP	%ebx
	POP	%ecx
	POP	%ecx
	JMP	insn_end
	

type_06_imm:
	#  Shift %edx so %dl is the opcode byte and %dh the extension bits
	MOVB	$16, %cl
	SHRL	%edx		# by %cl
	PUSH	%edx		# store op info

	#  Write the opcode byte from %dl
	LEA	-4208(%ebp), %ecx
	PUSH	%ecx
	MOVB	%dl, %al
	PUSH	%eax
	CALL	writebyte
	POP	%ecx
	POP	%ecx

	#  The AT&T naming convention makes this awkward.  We need to 
	#  write the ModR/M bytes *before* looking up the symbol in the 
	#  immediate.  Unfortunately, in AT&T syntax, the immediate is before 
	#  the r/m argument which means we don't know whether it's a got 
	#  a disp word.

	LEA	-4200(%ebp), %ecx	# ifile
	PUSH	%ecx
	CALL	skiphws
	POP	%ecx
	POP	%edx	# retrieve op info
	CMPB	$0x24, %al	# '$'
	JE	.L23b
	
	#  We've got a an intermediate label.  Get first char into buffer,
	#  and then read the label.
	PUSH	%edx		# store op info
	LEA	-4200(%ebp), %ecx	# ifile
	PUSH	%ecx
	CALL	getone
	MOVB	%al, -96(%ebp)
	PUSH	%ebp		# main_frame
	CALL	read_id
	POP	%ecx
	POP	%ecx		# ifile
	
	#  Write back the last byte
	PUSH	%eax		# save endp
	PUSH	%ecx		# ifile
	PUSH	(%eax)		# char
	CALL	unread
	POP	%ecx
	POP	%ecx		# ifile
	POP	%eax		# endp

	#  Calculate the string length
	LEA	-96(%ebp), %edx
	SUBL	%edx, %eax
	POP	%ecx	# retrieve op info
	PUSH	%ebp		# main_frame
	PUSH	%ebx		# bits
	PUSH	%eax		# strlen
	PUSH	(%edx)		# first dword

	#  Now call into the {.L23a, .L22a, insn_end, ret1} block to handle
	#  the ModR/M (incl. disp32).   Urgh.
	MOVB	%ch, %al
	CALL	.L23a

	#  Restore first dword of the label, and then look up the immediate
	#  label now that ModR/M is written
	POP	-96(%ebp)
	CALL	get_label
	POP	%ecx
	POP	%ebx		# bits
	POP	%ecx

	#  And write the immediate data
	LEA	-4208(%ebp), %ecx
	PUSH	%ecx
	PUSH	%eax		# byte(s)
	PUSH	%ebx		# bits
	CALL	writedata
	POP	%ecx
	POP	%ecx
	POP	%ecx

	JMP	insn_end

type_06:
	#  First, skip whitespace, leaving %eax containing the peek-ahead char
	PUSH	%edx	# opcode info
	LEA	-4200(%ebp), %ecx	# ifile
	PUSH	%ecx
	CALL	skiphws
	POP	%ecx	# ifile
	POP	%edx

	#  Need to determine which of the three types of instruction it is.
	CMPB	$0x25, %al	# '%'
	JE	type_06_reg
	CMPB	$0x28, %al	# '('
	JE	type_06_rm
	CMPB	$0x2D, %al	# '-'
	JE	type_06_rm
	PUSH	%eax
	CALL	dchr
	POP	%ecx
	CMPB	$-1, %al
	JNE	type_06_rm
	JMP	type_06_imm

hex_bytes:
	#  Skip horizontal whitespace and read a byte
	LEA	-4200(%ebp), %ecx	# ifile
	PUSH	%ecx
	CALL	skiphws
	POP	%ecx

	#  Is it a line ending?  If so, end parsing the directive.
	CMPB	$0x0A, %al	# '\n'
	JE	insn_end
	CMPB	$0x3B, %al	# ';'
	JE	insn_end
	CMPB	$0x23, %al	# '#'
	JE	insn_end

	PUSH	%ecx
	LEA	-96(%ebp), %ecx
	PUSH	%ecx
	CALL	readonex
	POP	%edx
	POP	%edx

	#  Start parsing an octet
	PUSH	-96(%ebp)
	CALL	xchr
	POP	%ebx
	CMPB	$-1, %al
	JE	error

	#  Read the next byte
	PUSH	%eax
	LEA	-4200(%ebp), %ecx	# ifile
	PUSH	%ecx
	LEA	-95(%ebp), %ecx
	PUSH	%ecx
	CALL	readonex
	POP	%edx
	POP	%edx
	POP	%ebx	# The first byte

	#  Process it
	PUSH	-95(%ebp)
	CALL	xchr
	POP	%edx
	CMPL	$-1, %eax
	JE	error
	MOVB	$4, %cl
	SALB	%bl		# by %cl
	ADDB	%bl, %al

	#  Byte is in %al; let's write it, and increment the address counter
	LEA	-4208(%ebp), %ecx	# ofile
	PUSH	%ecx
	PUSH	%eax
	CALL	writebyte
	POP	%ebx
	POP	%ebx
	JMP	hex_bytes


	#  --- Test for an identifier at top level in the source file.
	#  This might be a label, a mnemonic or a directive.
tl_ident:
	PUSH	%ebp
	CALL	read_id
	POP	%ecx
	MOVL	%eax, %ecx

	#  (%ecx) is now something other than lchr.  Is it a colon?
	#  Also, null terminate, load %esi with start of string, and
	#  %ecx with its length inc. NUL.
	CMPB	$0x3A, (%ecx)        # ':'
	PUSHF
	MOVL	(%ecx), %edx
	MOVB	$0, (%ecx)	# '\0' term.
	INCL	%ecx
	LEA	-96(%ebp), %esi
	SUBL	%esi, %ecx
	CMPL	$12, %ecx
	JG	error
	POPF
	JE	labeldef

	#  It must be either a directive or an instruction
	LEA	-4200(%ebp), %eax	# ifile
	PUSH	%ecx	# we care about this
	PUSH	%eax
	PUSH	%edx
	CALL	unread
	POP	%ecx
	POP	%ecx
	POP	%ecx

	#  Look up the mnemonic or directive
	#  %esi points to the null-terminated identifier name
	MOVL	-4(%ebp), %edi
	SUBL	$16, %edi
.L14a:
	ADDL	$16, %edi
	CMPL	$0, (%edi)
	JE	error
	PUSH	%ecx
	PUSH	%esi
	PUSH	%edi
	REPE CMPSB
	POP	%edi
	POP	%esi
	POP	%ecx
	JNE	.L14a

	#  Found the mnemonic or directive.  Get it's type (and parameters)
	#  and see whether it's a directive (which must be a .hex)
	MOVL	12(%edi), %edx
	CMPB	$0xFF, %dl
	JE	hex_bytes

	#  Instructions with 8-bit operands
	MOVL	$8, %ebx	# 8-bit mode
	CMPB	$0, %dl
	JE	type_00
	CMPB	$1, %dl
	JE	type_01
	CMPB	$3, %dl
	JE	type_03
	CMPB	$6, %dl
	JE	type_06

	#  Instructions with 32-bit operands
	MOVL	$32, %ebx	# 32-bit mode
	CMPB	$2, %dl
	JE	type_01		# 1 & 2 same
	CMPB	$4, %dl
	JE	type_03		# 3 & 4 same
	CMPB	$5, %dl
	JE	type_05
	CMPB	$7, %dl
	JE	type_06		# 6 & 7 same

	JMP	error
	RET	# to main loop

.L8:
	#  Read one byte (not with readonex because EOF is permitted)
	LEA	-4200(%ebp), %ecx
	PUSH	%ecx
	LEA	-96(%ebp), %ecx
	PUSH	%ecx
	CALL	readone
	POP	%edx
	POP	%edx
	CMPL	$0, %eax
	JL	error
	MOVL	%eax, %ebx  # zero exit status
	JE	ret1

	#  Is the byte white space?  If so, loop back
	MOVB	-96(%ebp), %al
	PUSH	%eax
	CALL	isws
	CMPL	$0, %eax
	POP	%edx
	JNE	.L8

	#  We have a byte.  What is it?
	CALL	comment
	CMPL	$0, %eax
	JNE	.L8

	#  Or perhaps a top-level identifier (label / mnemonic / directive);
	#  nothing else is permitted here.
	CALL	tl_ident
	CMPL	$0, %eax
	JE	error
	CMPL	$1, %eax
	JE	.L8

	#  We only get here if we've had an instruction or directive (as 
	#  opposed to a comment, label, or whitespace.)
	LEA	-4200(%ebp), %ecx	# ifile
	PUSH	%ecx
	CALL	skiphws
	POP	%ecx	# ifile

	#  Ordinarily, the next character should be either a '\n', '#' or ';',
	#  but we'll be permissive here to allow things like REPE CMPSB.  So
	#  we make the ';' between instructions on a single line optional.
	CMPB	$0x3B, %al	# ';'
	JNE	.L8	# Permissive

	#  Skip over the semicolon (statement separator)
	LEA	-4200(%ebp), %ecx	# ifile
	PUSH	%ecx
	CALL	getone
	POP	%ecx	# ifile

	#  Nothing else is permitted here.	
	JMP	.L8

	#  --- The main loop
main:
	MOVL	%esp, %ebp
	SUBL	$4212, %esp
	#  label* label_end = &labels[0];
	LEA	-4192(%ebp), %eax
	MOVL	%eax, -8(%ebp)
	#  label* label_end_store = &labels[256];  # 4096 == 256*sizeof(label)
	ADDL	$4096, %eax
	MOVL	%eax, -12(%ebp)

	#  Check we have a command line argument
	CMPL	$2, 0(%ebp)
	JNE	error

	#  Open the file
	XORL	%ecx, %ecx	# 0 == O_RDONLY
	MOVL	8(%ebp), %ebx
	MOVL	$5, %eax	# 5 == __NR_open
	INT	$0x80
	CMPL	$0, %eax
	JL	error

	#  memset( &fout, 0, sizeof(ofile) );
	#  No write on first pass, so fout.fd = -1
	MOVL	$-1, -4208(%ebp)
	MOVL	$0, -4204(%ebp)
	#  memset( &fin, 0, sizeof(ifile) );
	MOVL	%eax, -4200(%ebp) # 0 == stdin
	MOVL	$0, -4196(%ebp)

	#  Locate the mnemonics table:  instrct* mnemonics = &mnemonics;
	#  CALL next_line; next_line: POP %eax  simply effects  MOV %eip, %eax
	CALL	.L8a
.L8a:
	POP	%eax		# Currently assembled sub-optimally as 2 bytes
	ADDL	mnemonics, %eax	# Assembled as six bytes (op, modrm, imm32)

	#  And add the length of the previous two instructions
	ADDL	$8, %eax   # len of prev two instr
	MOVL	%eax, -4(%ebp)

	CALL	.L8

	#  Set fout.fd = 1 (for stdout) to initiate writing, and reset counter
	MOVL	$1, -4208(%ebp)
	MOVL	$0, -4204(%ebp)

	#  Seek to the beginning of the file for second pass
	XORL	%edx, %edx		# SEEK_SET=0
	XORL	%ecx, %ecx		# offset = 0
	MOVL	-4200(%ebp), %ebx	# fd
	MOVL	$19, %eax		# _NR_lseek
	INT	$0x80
	CMPL	$0, %eax
	JL	error

	CALL	.L8
	JMP	success


####    #  And finally, the entry point.
	#  Last per requirement for elfify.
	JMP     main

