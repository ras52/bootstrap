# as.s

# Copyright (C) 2011, 2012 Richard Smith <richard@ex-parrot.com>
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

.hex_bytes  4E 4F 50 00  00 00 00 00  00 00 00 00    00 01 90 00    # NOP
.hex_bytes  50 55 53 48  46 00 00 00  00 00 00 00    00 01 9C 00    # PUSHF
.hex_bytes  50 4F 50 46  00 00 00 00  00 00 00 00    00 01 9D 00    # POPF
.hex_bytes  52 45 50 00  00 00 00 00  00 00 00 00    00 01 F3 00    # REP
.hex_bytes  52 45 50 45  00 00 00 00  00 00 00 00    00 01 F3 00    # REPE
.hex_bytes  52 45 50 4E  45 00 00 00  00 00 00 00    00 01 F2 00    # REPNE
.hex_bytes  4D 4F 56 53  42 00 00 00  00 00 00 00    00 01 A4 00    # MOVSB
.hex_bytes  43 4D 50 53  42 00 00 00  00 00 00 00    00 01 A6 00    # CMPSB
.hex_bytes  53 43 41 53  42 00 00 00  00 00 00 00    00 01 AE 00    # SCASB
.hex_bytes  43 42 57 00  00 00 00 00  00 00 00 00    00 02 66 98    # CBW
.hex_bytes  43 57 44 45  00 00 00 00  00 00 00 00    00 01 98 00    # CWDE
.hex_bytes  43 44 51 00  00 00 00 00  00 00 00 00    00 01 99 00    # CDQ
.hex_bytes  52 45 54 00  00 00 00 00  00 00 00 00    00 01 C3 00    # RET
.hex_bytes  48 4C 54 00  00 00 00 00  00 00 00 00    00 01 F4 00    # HLT


# Type 01 instructions.   A single immediate 8-bit argument, e.g.
#
#   INT     imm8            CD
#
# The three parameters are: 
#   1) the number of bytes in the op-code (01 here),
#   2) the first op-code byte (CD), and 
#   3) the second op-code byte (n/a here).

.hex_bytes  49 4E 54 00  00 00 00 00  00 00 00 00    01 01 CD 00    # INT


# Type 02 instructions.   A single immediate 32-bit argument, e.g.
#
#   CALL    rel32           E8
#
# The three parameters are: 
#   1) the number of bytes in the op-code (01 here),
#   2) the first op-code byte (E8), and 
#   3) the second op-code byte (n/a here).

.hex_bytes  43 41 4C 4C  00 00 00 00  00 00 00 00    02 01 E8 00    # CALL
.hex_bytes  4A 4D 50 00  00 00 00 00  00 00 00 00    02 01 E9 00    # JMP
.hex_bytes  4A 45 00 00  00 00 00 00  00 00 00 00    02 02 0F 84    # JE
.hex_bytes  4A 47 00 00  00 00 00 00  00 00 00 00    02 02 0F 8F    # JG
.hex_bytes  4A 47 45 00  00 00 00 00  00 00 00 00    02 02 0F 8D    # JGE
.hex_bytes  4A 41 00 00  00 00 00 00  00 00 00 00    02 02 0F 87    # JA
.hex_bytes  4A 41 45 00  00 00 00 00  00 00 00 00    02 02 0F 83    # JAE
.hex_bytes  4A 4C 00 00  00 00 00 00  00 00 00 00    02 02 0F 8C    # JL
.hex_bytes  4A 4C 45 00  00 00 00 00  00 00 00 00    02 02 0F 8E    # JLE
.hex_bytes  4A 42 00 00  00 00 00 00  00 00 00 00    02 02 0F 82    # JB
.hex_bytes  4A 42 45 00  00 00 00 00  00 00 00 00    02 02 0F 86    # JBE
.hex_bytes  4A 4E 45 00  00 00 00 00  00 00 00 00    02 02 0F 85    # JNE
.hex_bytes  4A 43 00 00  00 00 00 00  00 00 00 00    02 02 0F 82    # JC == JB
.hex_bytes  4A 4F 00 00  00 00 00 00  00 00 00 00    02 02 0F 80    # JO


# Type 03 instructions.   A single r/m8 operand, e.g.
#
#   INCB    r/m8            FE /0
#
# The three parameters are:
#   1) the number 1 (to allow it to be treated as the opcode length)
#   2) the op-code byte (FE), and 
#   3) the three-bit reg field for the ModR/M byte (0 here).

.hex_bytes  49 4E 43 42  00 00 00 00  00 00 00 00    03 01 FE 00    # INCB
.hex_bytes  44 45 43 42  00 00 00 00  00 00 00 00    03 01 FE 01    # DECB
.hex_bytes  4E 45 47 42  00 00 00 00  00 00 00 00    03 01 F6 03    # NEGB
.hex_bytes  53 41 4C 42  00 00 00 00  00 00 00 00    03 01 D2 04    # SALB
.hex_bytes  53 48 4C 42  00 00 00 00  00 00 00 00    03 01 D2 04    # SHLB
.hex_bytes  53 41 52 42  00 00 00 00  00 00 00 00    03 01 D2 07    # SARB
.hex_bytes  53 48 52 42  00 00 00 00  00 00 00 00    03 01 D2 05    # SHRB
.hex_bytes  4D 55 4C 42  00 00 00 00  00 00 00 00    03 01 F6 04    # MULB
.hex_bytes  49 4D 55 4C  42 00 00 00  00 00 00 00    03 01 F6 05    # IMULB
.hex_bytes  44 49 56 42  00 00 00 00  00 00 00 00    03 01 F6 06    # DIVB
.hex_bytes  49 44 49 56  42 00 00 00  00 00 00 00    03 01 F6 07    # IDIVB


# Type 04 instructions.   A single r/m32 operand, e.g.
#
#   INCL    r/m32           FF /0
#
# The three parameters are:
#   1) the number 1 (to allow it to be treated as the opcode length)
#   2) the op-code byte (FF), and 
#   3) the three-bit reg field for the ModR/M byte (0 here).

.hex_bytes  49 4E 43 4C  00 00 00 00  00 00 00 00    04 01 FF 00    # INCL
.hex_bytes  44 45 43 4C  00 00 00 00  00 00 00 00    04 01 FF 01    # DECL
.hex_bytes  4E 45 47 4C  00 00 00 00  00 00 00 00    04 01 F7 03    # NEGL
.hex_bytes  50 55 53 48  00 00 00 00  00 00 00 00    04 01 FF 06    # PUSH
.hex_bytes  50 4F 50 00  00 00 00 00  00 00 00 00    04 01 8F 00    # POP
.hex_bytes  53 41 4C 4C  00 00 00 00  00 00 00 00    04 01 D3 04    # SALL
.hex_bytes  53 48 4C 4C  00 00 00 00  00 00 00 00    04 01 D3 04    # SHLL
.hex_bytes  53 41 52 4C  00 00 00 00  00 00 00 00    04 01 D3 07    # SARL
.hex_bytes  53 48 52 4C  00 00 00 00  00 00 00 00    04 01 D3 05    # SHRL
.hex_bytes  4D 55 4C 4C  00 00 00 00  00 00 00 00    04 01 F7 04    # MULL
.hex_bytes  49 4D 55 4C  4C 00 00 00  00 00 00 00    04 01 F7 05    # IMULL
.hex_bytes  44 49 56 4C  00 00 00 00  00 00 00 00    04 01 F7 06    # DIVL
.hex_bytes  49 44 49 56  4C 00 00 00  00 00 00 00    04 01 F7 07    # IDIVL


# Type 05 instructions.   An r/m32 operand followed by a r32 operand:
#
#   LEA     r/m32, r32      8D
#
# The three parameters are: 
#   1) the number of bytes in the op-code (01 here),
#   2) the first op-code byte (8D), and 
#   3) the second op-code byte (n/a here).

.hex_bytes  4C 45 41 00  00 00 00 00  00 00 00 00    05 01 8D 00    # LEA


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

.hex_bytes  4D 4F 56 42  00 00 00 00  00 00 00 00    06 88 C6 00    # MOVB
.hex_bytes  41 44 44 42  00 00 00 00  00 00 00 00    06 00 80 00    # ADDB
.hex_bytes  53 55 42 42  00 00 00 00  00 00 00 00    06 28 80 05    # SUBB
.hex_bytes  43 4D 50 42  00 00 00 00  00 00 00 00    06 38 80 07    # CMPB
.hex_bytes  41 4E 44 42  00 00 00 00  00 00 00 00    06 20 80 04    # ANDB
.hex_bytes  4F 52 42 00  00 00 00 00  00 00 00 00    06 08 80 01    # ORB
.hex_bytes  58 4F 52 42  00 00 00 00  00 00 00 00    06 30 80 06    # XORB


# Type 07 instructions.   These represent a large family of op-codes, e.g.
#
#   MOVL    r32,r/m32       89         } These two opcodes are constrained
#   MOVL    r/m32,r32       8B         } to $N and $N+2.
#   MOVL    imm32,r/m32     C7 /0
#
# The three parameters are the register op code (88 here), the 
# immediate op code (C6 here), and the three op-code extension bits
# that are in the reg part of the mod-r/m byte (0 here).

.hex_bytes  4D 4F 56 4C  00 00 00 00  00 00 00 00    07 89 C7 00    # MOVL
.hex_bytes  41 44 44 4C  00 00 00 00  00 00 00 00    07 01 81 00    # ADDL
.hex_bytes  53 55 42 4C  00 00 00 00  00 00 00 00    07 29 81 05    # SUBL
.hex_bytes  43 4D 50 4C  00 00 00 00  00 00 00 00    07 39 81 07    # CMPL
.hex_bytes  41 4E 44 4C  00 00 00 00  00 00 00 00    07 21 81 04    # ANDL
.hex_bytes  4F 52 4C 00  00 00 00 00  00 00 00 00    07 09 81 01    # ORL
.hex_bytes  58 4F 52 4C  00 00 00 00  00 00 00 00    07 31 81 06    # XORL


# Type FF 'instructions'.  These are actually directives.    
#
#   .hex_bytes
#
# The sole parameter is an unique code used internally to represent the 
# directive.  Each directive is special-cased in the code.

.hex_bytes  2E 68 65 78  5F 62 79 74  65 73 00 00    FF 00 00 00    # .hex_bytes
.hex_bytes  2E 74 65 78  74 00 00 00  00 00 00 00    FF 01 00 00    # .text
.hex_bytes  2E 64 61 74  61 00 00 00  00 00 00 00    FF 01 01 00    # .data

zeros:
# End of table marker -- first byte of name is NULL.
.hex_bytes  00 00 00 00  00 00 00 00  00 00 00 00    00 00 00 00 
# ########################################################################



elf_hdr:

### The ELF header.      start: 0x00; length: 0x34
# 
#                        32-bit, LSB ('Intel') byte order, ELF v.1
#           magic-----\  |      /--------------------------------/
.hex_bytes  7F 45 4C 46  01 01 01 00    00 00 00 00  00 00 00 00
#           .o--\ 386-\  ELF-v1----\    entry-----\  phdr-off--\
.hex_bytes  01 00 03 00  01 00 00 00    00 00 00 00  00 00 00 00
#           shdr-off--\  flags-----\    ehsz\
.hex_bytes  34 00 00 00  00 00 00 00    34 00
#           phsz\ phn-\  shsz\ shn-\    shst\
.hex_bytes  00 00 00 00  28 00 08 00    02 00 

# #0  Null section header.   start: 0x34; length: 0x28
# c.f. Fig 4.10 in gABI 4.1
.hex_bytes  00 00 00 00  00 00 00 00    00 00 00 00  00 00 00 00
.hex_bytes  00 00 00 00  00 00 00 00    00 00 00 00  00 00 00 00
.hex_bytes  00 00 00 00  00 00 00 00

# #1  Text section header.   start: 0x5C; length: 0x28
# (Offset: 0x70 requires text section size.)	# TODO:  Done
#
#           name-str--\  PROGBITS--\    EXEC|ALLOC\  load-addr-\
.hex_bytes  1F 00 00 00  01 00 00 00    06 00 00 00  00 00 00 00
#           offset----\  **size**--\    /-- Fig 4.12 in gABI --\   <-- 0x1A4
.hex_bytes  A4 01 00 00  00 00 00 00    00 00 00 00  00 00 00 00
#           align-----\  entry-sz--\
.hex_bytes  04 00 00 00  00 00 00 00

# #2  Shstrtab section hdr.  start: 0x84; length: 0x28
#
#           name-str--\  STRTAB----\    no-flags--\  load-addr-\ 
.hex_bytes  01 00 00 00  03 00 00 00    00 00 00 00  00 00 00 00
#           offset----\  size------\    /-- Fig 4.12 in gABI --\
.hex_bytes  74 01 00 00  30 00 00 00    00 00 00 00  00 00 00 00
#           align-----\  entry-sz--\
.hex_bytes  01 00 00 00  00 00 00 00

# #3  Symbol table.          start: 0xAC; length: 0x28
# (Offsets 0xBC, 0xC0 need setting)	# TODO ???
#
#           name-str--\  SYMTAB----\    no-flags--\  load-addr-\ 
.hex_bytes  0B 00 00 00  02 00 00 00    00 00 00 00  00 00 00 00
#           **offset**\  **size**--\    .strtab---\  **LOCAL**-\ 
.hex_bytes  00 00 00 00  00 00 00 00    04 00 00 00  00 00 00 00
#           align-----\  entry-sz--\
.hex_bytes  04 00 00 00  10 00 00 00

# #4  String table.          start: 0xD4; length: 0x28
# (Offsets 0xE4, 0xE8 need setting)	# TODO ???
#
#           name-str--\  STRTAB----\    no-flags--\  load-addr-\
.hex_bytes  13 00 00 00  03 00 00 00    00 00 00 00  00 00 00 00
#           **offset**\  **size**--\    /-- Fig 4.12 in gABI --\
.hex_bytes  00 00 00 00  00 00 00 00    00 00 00 00  00 00 00 00
#           align-----\  entry-sz--\
.hex_bytes  01 00 00 00  00 00 00 00

# #5 Text relocations.      start: 0xFC; length: 0x28
# (Offsets 0x10C, 0x110 need setting)
#
#           name-str--\  SHT_REL---\    no-flags--\  load-addr-\
.hex_bytes  1B 00 00 00  09 00 00 00    00 00 00 00  00 00 00 00
#           **offset**\  **size**--\    .symtab---\  .text-----\
.hex_bytes  00 00 00 00  00 00 00 00    03 00 00 00  01 00 00 00
#           align-----\  entry-sz--\
.hex_bytes  04 00 00 00  08 00 00 00

# #6  Data section header.  start: 0x124; length: 0x28
# (Offsets 0x134, 0x138 need setting)   # TODO 0x138 done; 0x134 not done
#
#           name-str--\  PROGBITS--\    WRIT|ALLOC\  load-addr-\
.hex_bytes  2A 00 00 00  01 00 00 00    03 00 00 00  00 00 00 00
#           **offset**\  **size**--\    /-- Fig 4.12 in gABI --\ 
.hex_bytes  00 00 00 00  00 00 00 00    00 00 00 00  00 00 00 00
#           align-----\  entry-sz--\
.hex_bytes  04 00 00 00  00 00 00 00

# #7  Data relocations.     start: 0x14C; length: 0x28
#
#           name-str--\  SHT_REL--\     no-flags\    load-addr-\
.hex_bytes  25 00 00 00  09 00 00 00    00 00 00 00  00 00 00 00
#           **offset**\  **size**--\    .symtab---\  .data-----\
.hex_bytes  00 00 00 00  00 00 00 00    03 00 00 00  06 00 00 00
#           align-----\  entry-sz--\
.hex_bytes  04 00 00 00  08 00 00 00

# The shared string table itself.  start 0x174; length: 0x30
#
.hex_bytes  00                                # NULL        Offset 0x00
.hex_bytes  2E 73 68 73 74 72 74 61 62 00     # .shstrtab   Offset 0x01
.hex_bytes  2E 73 79 6D 74 61 62 00           # .symtab     Offset 0x0B
.hex_bytes  2E 73 74 72 74 61 62 00           # .strtab     Offset 0x13
.hex_bytes  2E 72 65 6C 2E 74 65 78 74 00     # .rel.text   Offset 0x1B (0x1F)
.hex_bytes  2E 64 61 74 61 2E 64 61 74 61 00  # .data.text  Offset 0x25 (0x2A)

# End of headers.   offset 0x1A4


# ########################################################################

####	#  Function: void* malloc(size_t sz)
	#  Crude dynamic memory allocation, by punting directly to kernel 
malloc:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%ebx

	#  How many bytes do we need?
	MOVL	8(%ebp), %ecx		# sz
	ADDL	$0x4, %ecx		# header containing size

	#  Punt off to mmap(MAP_ANON).  Highly suboptimal, but simple to code.
	XORL	%eax, %eax		# 0 offset
	PUSH	%eax
	DECL	%eax
	PUSH	%eax			# fd -1 for MAP_ANON
	MOVL	$0x22, %eax		# MAP_ANON (0x20) | MAP_PRIVATE (0x2)
	PUSH	%eax
	MOVL	$0x3, %eax		# PROT_READ (0x1) | PROT_WRITE (0x2)
	PUSH	%eax
	PUSH	%ecx			# size
	XORL	%eax, %eax		# NULL 
	PUSH	%eax
	MOVL	%esp, %ebx
	MOVL	$90, %eax		# 90 == __NR_mmap
	INT	$0x80
	CMPL	$-4096, %eax		# -4095 <= %eax < 0 for errno
	JA	error			# unsigned comparison handles above
	MOVL	-24(%ebp), %ecx		# restore %ecx
	ADDL	$0x18, %esp		# 6x POP
	
	#  Write size into malloc header
	MOVL	%ecx, (%eax)
	ADDL	$4, %eax

	#  Cleanup
	POP	%ebx
	POP	%ebp
	RET


####	#  Function: void* realloc(void* ptr, size_t sz)
	#  Grows memory allocated by malloc, above
realloc:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%ebx
	PUSH	%esi

	#  Leave space for header (4 bytes)
	MOVL	8(%ebp), %ebx		# ptr
	SUBL	$4, %ebx
	MOVL	(%ebx), %ecx		# old size
	MOVL	12(%ebp), %edx		# size
	ADDL	$4, %edx
	PUSH	%ecx

	#  Get kernel to mremap the block
	MOVL	$1, %esi		# 1 == MREMAP_MAYMOVE
	MOVL	$163, %eax		# 163 == __NR_mremap
	INT	$0x80
	CMPL	$-4096, %eax		# -4095 <= %eax < 0 for errno
	JA	error			# unsigned comparison handles above

	#  Write header
	POP	%ecx
	MOVL	%ecx, (%eax)
	ADDL	$4, %eax

	#  Cleanup
	POP	%esi
	POP	%ebx
	POP	%ebp
	RET
	

####	#  Function: size_t strlen(char const* str)
	#  Finds the length of str
strlen:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%edi
	MOVL	8(%ebp), %edi
	XORL	%eax, %eax
	XORL	%ecx, %ecx
	DECL	%ecx
	REPNE SCASB
	SUBL	8(%ebp), %edi
	DECL	%edi
	MOVL	%edi, %eax
	POP	%edi
	POP	%ebp
	RET


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
	MOVL	$1, %ebx
success:
	MOVL	$1, %eax   		# 1 == __NR_exit
	INT	$0x80


####	#  Function:    void writedptr( int bytes, void* data, ofile* of )
	#  A thin wrapper around write(2)
writedptr:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%ebx

	MOVL	8(%ebp), %edx		# bytes to write

	#  Increment counter
	MOVL	16(%ebp), %ebx
	ADDL	%edx, 4(%ebx)		# inc counter

	#  If the fd is -ve, we're not actually writing
	MOVL	(%ebx), %ebx		# fd from ofile
	CMPL	$0, %ebx
	JL	.L5d

        PUSH	%edx
	MOVL	12(%ebp), %ecx		# data
	MOVL	$4, %eax   		# 4 == __NR_write
        INT	$0x80
        POP	%edx
        CMPL	%edx, %eax
        JNE	error

.L5d:
	POP	%ebx
	POP	%ebp
	RET


####	#  Function:	void writebyte( int c, ofile* of )
	#  Write out character c
writebyte:
	PUSH	%ebp
	MOVL	%esp, %ebp

	PUSH	12(%ebp)	# ofile*
	LEA	8(%ebp), %eax	# &c
	PUSH	%eax
	MOVL	$1, %eax	# Write one byte
	PUSH	%eax
	CALL	writedptr
	POP	%eax
	POP	%eax
	POP	%eax

	POP	%ebp
	RET


####	#  Function:	void writedword( int dw, ofile* of )
	#  Write out dword dw
writedword:
	PUSH	%ebp
	MOVL	%esp, %ebp

	PUSH	12(%ebp)	# ofile*
	LEA	8(%ebp), %eax	# &c
	PUSH	%eax
	MOVL	$4, %eax	# Write one byte
	PUSH	%eax
	CALL	writedptr
	POP	%eax
	POP	%eax
	POP	%eax

	POP	%ebp
	RET


####	#  Function:	void writedata( int bits, int data, ofile* of )
	#  A wrapper around writedword / writebyte.  BITS must be 8 or 32.
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
	POP	%edx
	POP	%edx
	POP	%ebp
	RET


####	#  Function:	void writedwat( int dw, int offset, ofile* of )
	#  Seeks to offset and writes a dword
writedwat:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%ebx

	#  Seek to offset
	XORL	%edx, %edx		# SEEK_SET=0
	MOVL	12(%ebp), %ecx		# offset
	MOVL	16(%ebp), %ebx		
	MOVL	(%ebx), %ebx		# fd from ofile
	MOVL	$19, %eax		# 19 == __NR_lseek
	INT	$0x80
	CMPL	$0, %eax
	JL	error

	#  Write the dword
	PUSH	16(%ebp)
	PUSH	8(%ebp)
	CALL	writedword
	POP	%ebx
	POP	%ebx

	#  Don't count this the in byte counter
	SUBL	$4, 4(%ebx)

	#  And seek back to the end
	MOVL	$2, %edx		# SEEK_END=2
	XORL	%ecx, %ecx		# offset = 0 (end)
	MOVL	(%ebx), %ebx		# fd
	MOVL	$19, %eax		# 19 == __NR_lseek
	INT	$0x80
	CMPL	$0, %eax
	JL	error

	POP	%ebx
	POP	%ebp
	RET


####	#  Function:	void writepad( int align, ofile* o )
	#  Pad to an ALIGN byte boundary
writepad:
	PUSH	%ebp
	MOVL	%esp, %ebp

	#  Pad to a 16-byte boundary
	XORL	%edx, %edx
	MOVL	12(%ebp), %eax
	MOVL	4(%eax), %eax		# ofile->count
	MOVL	8(%ebp), %ecx
	DIVL	%ecx			# acts on %edx:%eax
	SUBL	%edx, %ecx
	CMPL	8(%ebp), %ecx		# Are we already aligned?
	JE	.L8b2

	#  Generate 16 bytes of zeros on the stack
	XORL	%eax, %eax
	PUSH	%eax
	PUSH	%eax
	PUSH	%eax
	PUSH	%eax

	#  Write padding
	MOVL	%esp, %edx		# pointer to padding
	PUSH	12(%ebp)		# ofile
	PUSH	%edx			# data
	PUSH	%ecx			# count
	CALL	writedptr
	POP	%eax			# Return count
	ADDL	$0x18, %esp

.L8b2:
	POP	%ebp
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
	MOVL	(%ebx), %ebx		# fd from ifile
	MOVL	$3, %eax		# 3 == __NR_read
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

	ADDL	$12, %esp
	POP	%ebp
	RET


####	#  Function:	int read_r32( ifile* in )
	#  We've already read a '%'.  Now read a 32-bit register name.  Return
	#    0 => %eax, 1 => %ecx, 2 => %edx, 3 => %ebx
	#    4 => %esp, 5 => %ebp, 6 => %esi, 7 => %edi
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

.L9d:	
	#  Have parsed two characters of %eax or %ecx, check for 'x'
	MOVL	%ecx, -4(%ebp)	# value
	CALL	getone
	MOVL	-4(%ebp), %ecx	# value
	CMPB	$0x78, %al	# 'x'
	JE	.L9h

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
	
.L9f:
	#  Is it %edi?
	CMPB	$0x69, %al	# 'i'
	JNE	error
	MOVL	$7, %ecx	# == %edi
	JMP	.L9h
	
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

.L9h:
	MOVL	%ecx, %eax
	POP	%edx		# value
	POP	%edx		# ifile
	POP	%ebp
	RET


####	#  Function:	int read_r8( ifile* in )
	#  We've already read a '%'.  Now read an 8-bit register name.  Return
	#    0 => %al,  1 => %cl,  2 => %dl,  3 => %bl
	#    4 => %ah,  5 => %ch,  6 => %dh,  7 => %bh
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
	JNE	error

.L9b:
	#  Read and parse the second character
	MOVL	%ecx, -4(%ebp)	# value
	CALL	getone
	MOVL	-4(%ebp), %ecx	# value
	CMPB	$0x6C, %al	# 'l'
	JE	.L9c
	ADDL	$4, %ecx
	CMPB	$0x68, %al	# 'h'
	JNE	error

.L9c:
	MOVL	%ecx, %eax
	POP	%edx		# value
	POP	%edx		# ifile
	POP	%ebp
	RET


####	#  Function:	int read_reg( int bits, ifile* in )
	#  Read a 8- or 32-bit register name, depending on BITS.  The
	#  leading '%' must already be read.
read_reg:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	12(%ebp)	# ifile

	CMPL	$8, 8(%ebp)
	JE	.L13
	CALL	read_r32
	JMP	.L13a
.L13:
	CALL	read_r8
.L13a:
	POP	%ecx
	POP	%ebp
	RET


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
read_int:
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

	#  Don't care about overflow if bits == 32
	CMPB	$32, 8(%ebp)
	JE	.L18a

	#  Test for overflow.
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
.L18a:
	NEGL	%eax
	MOVL	%eax, -4(%ebp)	# save
	JMP	.L17b

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


####	#  Function:	int read_rm( int bits, int* disp, ifile* in )
	#  Skip whitespace then read an r/m8 or r/m32 depending on BITS.  
	#  Returns the ModR/M byte and if a displacement is present, stores 
	#  it in disp.
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


####	#  Function:	int getlabel( int* slot, int strlen, main_frame* p ):
	#  Lookup the text in p->buffer (which has known length strlen),
	#  and return an offset relative to the start of the .text section,
	#  or -1 if it cannot be found.
getlabel:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%ebx
	PUSH	%esi
	PUSH	%edi

	MOVL	12(%ebp), %ecx	# strlen
	MOVL	16(%ebp), %ebx
	LEA	-96(%ebx), %esi
	MOVL	-4(%ebx), %edi
	SUBL	$16, %edi

	#  Null terminate
	MOVL	%esi, %edx
	ADDL	%ecx, %edx
	XORL	%eax, %eax
	MOVL	%eax, (%edx)
	INCL	%ecx

	#  Counter
	XORL	%edx, %edx
	DECL	%edx

	PUSH	%ecx
.L11:
	POP	%ecx

	INCL	%edx
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

	#  Found it: get it's value
	MOVL	12(%edi), %eax
	
	#  Set the counter
	CMPL	$0, 8(%ebp)
	JE	.L11a
	MOVL	8(%ebp), %ecx
	MOVL	%edx, (%ecx)
	JMP	.L11a

.L11b:
	#  Not found: return -1 
	XORL	%eax, %eax
	DECL	%eax

.L11a:
	POP	%edi
	POP	%esi
	POP	%ebx
	POP	%ebp
	RET


####	#  Function:	void savelabel( int val, int strlen, main_frame* p);
	#  Save a label with value VAL.  The special value -1 will result
	#  in a symbol table entry, but no label.  This is used for 
	#  undefined symbols.
savelabel:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%ebx
	PUSH	%esi
	PUSH	%edi

	MOVL	16(%ebp), %ebx

	#  Check that we're not about to over run the label store,
	MOVL	-12(%ebx), %eax
	CMPL	%eax, -8(%ebx)
	JL	.L10c

	#  Grow storage
	XORL	%edx, %edx
	MOVL	-12(%ebx), %eax
	SUBL	-4(%ebx), %eax
	MOVL	$2, %ecx
	MULL	%ecx			# acts on %edx:%eax
	PUSH	%eax			# new size (twice old)
	PUSH	-4(%ebx)		# ptr
	CALL	realloc

	#  Store pointers
	MOVL	%eax, -4(%ebx)		# store new pointer
	POP	%ecx
	SUBL	%ecx, -8(%ebx)
	ADDL	%eax, -8(%ebx)		# new end data
	POP	%ecx
	ADDL	%eax, %ecx
	MOVL	%ecx, -12(%ebx)		# new end storage

.L10c:
	#  And then store the label
	LEA	-96(%ebx), %esi
	MOVL	-8(%ebx), %edi
	MOVL	12(%ebp), %ecx		# strlen
	REP MOVSB
	MOVL	8(%ebp), %eax 		# value
	MOVL	-8(%ebx), %edi
	MOVL	%eax, 12(%edi)
	ADDL	$16, -8(%ebx)

	POP	%edi
	POP	%esi
	POP	%ebx
	POP	%ebp
	RET


####	#  Function:	void saverel( int symbol, main_frame* p);
	#  Save a relocation at the current ofile offset.  
saverel:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%ebx

	MOVL	12(%ebp), %ebx

	#  Check that we're not about to over run the rel store
	MOVL	-128(%ebx), %eax	# end_store
	CMPL	%eax, -124(%ebx)
	JL	.L24

	#  Grow storage
	XORL	%edx, %edx
	MOVL	-128(%ebx), %eax
	SUBL	-120(%ebx), %eax
	MOVL	$2, %ecx
	MULL	%ecx			# acts on %edx:%eax
	PUSH	%eax			# new size (twice old)
	PUSH	-120(%ebx)		# ptr
	CALL	realloc

	#  Store pointers
	MOVL	%eax, -120(%ebx)	# store new pointer
	POP	%ecx
	SUBL	%ecx, -124(%ebx)
	ADDL	%eax, -124(%ebx)	# new end data
	POP	%ecx
	ADDL	%eax, %ecx
	MOVL	%ecx, -128(%ebx)	# new end storage

.L24:
	#  Save the relocation position
	MOVL	-124(%ebx), %edx
	MOVL	-108(%ebx), %eax	# ofile->count
	MOVL	%eax, (%edx)
	MOVL	8(%ebp), %eax		# The symbol slot index
	MOVL	%eax, 4(%edx)
	ADDL	$8, -124(%ebx)

	POP	%ebx
	POP	%ebp
	RET

	
####	#  Function:	int labelref( int strlen, int bits, main_frame* p ):
	#  Lookup the text in p->buffer (which has known length strlen),
	#  Convert it to an offset relative to the current instruction
	#  (we're assumed to have written the opcode and any ModR/M bytes,
	#  so the end of instruction happens after writing the number of bits
	#  in the BITS parameter).  Check the offset can be expressed as a
	#  BITS-bit signed (2's-complement) integer.
labelref:
	PUSH	%ebp
	MOVL	%esp, %ebp

	#  If we're in pass #1, do nothing
	MOVL	16(%ebp), %ecx
	CMPL	$0, -112(%ecx)
	JLE	.L11e

	#  Make a slot for the index of the undefined symbol
	XORL	%eax, %eax
	DECL	%eax
	PUSH	%eax

	PUSH	16(%ebp)
	PUSH	8(%ebp)
	LEA	-4(%ebp), %eax		# the symbol table slot, above
	PUSH	%eax
	CALL	getlabel
	POP	%ecx
	POP	%ecx
	POP	%ecx

	CMPL	$-1, %eax
	JE	.L11d
	MOVL	%eax, %edx

	POP	%eax			# symbol slot

	#  Found it.  Compute the value.
	MOVL	12(%ebp), %eax
	MOVB	$3, %cl
	SARL	%eax			# by %cl  -- %eax is now num bytes
	MOVL	16(%ebp), %ecx
	ADDL	-108(%ecx), %eax	# Add ofile->count
	SUBL	%edx, %eax		# Subtract position
	NEGL	%eax

	#  %eax is the offset.  If bits == 32, don't care about overflow
	CMPL	$32, 12(%ebp)
	JE	.L11c
	
	#  Is it >= 2^(bits-1) ?
	MOVL	12(%ebp), %ecx
	DECL	%ecx
	MOVL	$1, %edx
	SALL	%edx			# by %cl
	CMPL	%eax, %edx
	JLE	error

	#  Or < 2^(bits-1)?
	NEGL	%edx
	CMPL	%eax, %edx
	JG	error
	JMP	.L11c

.L11d:
	#  Only support 32-bit relocations
	CMPL	$32, 12(%ebp)
	JNE	error

	#  Head of stack now has symbol table slot
	POP	%edx
	CMPL	$-1, %edx
	JNE	.L11f

	#  Create a symbol table entry for it
	PUSH	16(%ebp)		# main frame bp
	PUSH	8(%ebp)			# strlen
	PUSH	%edx			# %edx == -1 
	CALL	savelabel
	POP	%ecx
	POP	%ecx
	POP	%ecx

	#  Find the number of the new symbol
	MOVL	16(%ebp), %edx		# main frame
	MOVL	-8(%edx), %eax		# label_end
	SUBL	-4(%edx), %eax		# - label_start
	XORL	%edx, %edx
	MOVL	$16, %ecx
	DIVL	%ecx			# acts on %edx:%eax
	DECL	%eax			# want an index (0 based)
	MOVL	%eax, %edx

.L11f:
	#  Save the relocation
	PUSH	16(%ebp)		# main frame bp
	PUSH	%edx			# symbol
	CALL	saverel
	POP	%eax
	POP	%eax
	MOVL	$-4, %eax		# The relocation addend
	JMP	.L11c

.L11e:
	XORL	%eax, %eax
	DECL	%eax			# Use -1 as error value

.L11c:
	POP	%ebp
	RET


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
	LEA	-104(%eax), %ecx
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
	LEA	-104(%ecx), %ecx
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
	JMP	.L16c

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
	CALL	labelref
	POP	%ecx
	POP	%ecx
	POP	%ecx
	MOVL	%eax, -8(%ebp)

	#  Unread the last character read
	POP	%ecx		# Overwrite 
	PUSH	-4(%ebp)	#   bufp
	CALL	unread

	MOVL	-8(%ebp), %eax	# return val
.L16c:
	#  Stack clean-up and exit
	ADDL	$20, %esp
	POP	%ebp
	RET


####	#  Function:	void write_oc12( int opcode_info, main_frame* )
	#  Uses the second byte in opcode_info to determine how many
	#  bytes to write (1 or 2) and then writes the third and perhaps
	#  fourth byte out.
write_oc12:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%ebx	# Save %ebx

	#  Find the ofile object
	MOVL	12(%ebp), %eax
	LEA	-112(%eax), %ecx
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
.L19:
	POP	%ebx	# ofile
	POP	%ebx	# Restore %ebx
	POP	%ebp
	RET


####	#  Function:	void write_mrm( int rm, int reg, int disp, ofile* )
write_mrm:
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
.L20:
	POP	%edx		# ofile
	POP	%ebp
	RET

	
####    #  The main function.
	#  Stack is arranged as follows:
	#
	#       -4(%ebp)	label* label_start
	#       -8(%ebp)	label* label_end
	#      -12(%ebp)	label* label_end_store
	#      -96(%ebp)	char buffer[80]
	#     -104(%ebp)	ifile fin
        #     -112(%ebp)	ofile fout
	#     -116(%ebp)	instrct* mnemonics
	#     -120(%ebp)	rel* rel_start
	#     -124(%ebp)	rel* rel_end
	#     -128(%ebp)	rel* rel_end_store
	#     -132(%ebp)	int parse_sect;
	#     -140(%ebp)	int sect_len[2];   # 0 .text; 1 .data
	#     -148(%ebp)	int sect_fd[2];
	#
	#  where label is a { char name[12]; int addr; },
	#  rel is a { int offset; int sym_no; },
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
	LEA	-104(%ebp), %ecx	# ifile
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
	#  Have we already seen this label?
	PUSH	%ebp
	PUSH	%esi
	CALL	strlen
	POP	%esi
	PUSH	%eax
	XORL	%eax, %eax
	PUSH	%eax		# NULL slot
	CALL	getlabel
	POP	%ecx
	POP	%ecx		# strlen return
	POP	%edx

	#  Is it new?	
	#  (-1 from an undefined symbol cannot happen.  If we're in pass #1
	#  we've not recorded any undefined symbols yet, and if we're in 
	#  pass #2, we must have already seen this label once already.)
	CMPL	$-1, %eax
	JE	.L10a

	#  Is it identical to what we already have?
	CMPL	-108(%ebp), %eax
	JE	.L10b
	JMP	error

.L10a:
	PUSH	%ebp
	PUSH	%ecx		# strlen
	PUSH	-108(%ebp)	# ofile->count
	CALL	savelabel
	ADDL	$12, %esp

.L10b:
	MOVL	$1, %eax
	RET	# to main loop

insn_end:
	MOVL	$2, %eax
	JMP	ret1

type_00:
	#  Write out a type 00 instruction.  These are easiest as they
	#  have no parameters.
	PUSH	%ebp		# main_frame
	PUSH	%edx		# opcode info
	CALL	write_oc12
	POP	%edx
	POP	%edx
	JMP	insn_end

type_01:
	#  Write the opcode byte(s).
	PUSH	%ebp		# main_frame
	PUSH	%edx		# opcode info
	CALL	write_oc12
	POP	%edx

	#  Skip ws and read immediate.
	PUSH	%ebx		# bits
	CALL	read_imm
	POP	%ecx
	POP	%ecx

	#  Byte is in %al (as part of return from read_imm): write it.
	LEA	-112(%ebp), %ecx	# ofile
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
	PUSH	%edx		# opcode info
	CALL	write_oc12
	POP	%edx
	POP	%ecx

	#  Read the r/m
	PUSH	%edx			# store opcode info
	LEA	-104(%ebp), %ecx	# ifile
	PUSH	%ecx
	LEA	-96(%ebp), %ecx		# disp
	PUSH	%ecx
	PUSH	%ebx			# bits
	CALL	read_rm
	ADDL	$12, %esp
	POP	%edx

.L21:
	#  Write the ModR/M byte and (if present) displacement.
	LEA	-112(%ebp), %ecx	# ofile
	PUSH	%ecx
	PUSH	-96(%ebp)	# disp
	MOVB	$24, %cl
	SHRL	%edx		# by %cl
	PUSH	%edx		# reg bits
	PUSH	%eax		# modrm
	CALL	write_mrm
	ADDL	$16, %esp
	JMP	insn_end

type_05:
	#  Write the opcode byte(s).
	PUSH	%ebp		# main_frame
	PUSH	%edx		# opcode info
	CALL	write_oc12
	POP	%edx
	POP	%ecx

.L22:
	#  Read the r/m
	LEA	-104(%ebp), %ecx	# ifile
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
	LEA	-104(%ebp), %ecx	# ifile
	PUSH	%ecx
	CALL	skiphws
	CALL	getone
	CMPB	$0x2C, %al		# ','
	JNE	error
	CALL	skiphws
	CALL	getone
	CMPB	$0x25, %al	# '%'
	JNE	error
	PUSH	%ebx		# bits
	CALL	read_reg
	POP	%edx		# bits
	POP	%edx		# ifile
	POP	%edx		# r/m

.L22a:
	#  Write the ModR/M byte and (if present) displacement.
	#  So: %edx is r/m, %eax is reg
	LEA	-112(%ebp), %ecx	# ofile
	PUSH	%ecx
	PUSH	-96(%ebp)		# disp
	PUSH	%eax			# reg
	PUSH	%edx			# rm
	CALL	write_mrm
	ADDL	$16, %esp
	JMP	insn_end

type_06_rm:
	#  %edx contains the opcode info, so %dh+2 is the opcode.  Write it.
	LEA	-112(%ebp), %ecx
	PUSH	%ecx
	MOVB	%dh, %al
	ADDL	$2, %eax
	PUSH	%eax
	CALL	writebyte
	POP	%ecx
	POP	%ecx

	JMP	.L22

type_06_rg:
	#  %edx contains the opcode info, so %dh is the opcode.  Write it.
	LEA	-112(%ebp), %ecx
	PUSH	%ecx
	MOVB	%dh, %al
	PUSH	%eax
	CALL	writebyte
	POP	%ecx
	POP	%ecx

.L23:
	#  Read the register
	LEA	-104(%ebp), %ecx	# ifile
	PUSH	%ecx
	CALL	skiphws
	CALL	getone
	CMPB	$0x25, %al		# '%'
	JNE	error
	PUSH	%ebx			# bits
	CALL	read_reg
	POP	%ebx			# bits
	POP	%ecx

.L23a:
	#  Skip ws, read a comma, then skip more ws and read the '%' and r/m
	#  We expect %eax to contain the reg bytes already read, and
	#  %ebx the number of bits.
	PUSH	%eax	# store reg
	LEA	-104(%ebp), %ecx	# ifile
	PUSH	%ecx
	CALL	skiphws
	CALL	getone
	CMPB	$0x2C, %al		# ','
	JNE	error
	LEA	-96(%ebp), %ecx		# disp
	PUSH	%ecx
	PUSH	%ebx			# bits
	CALL	read_rm
	MOVL	%eax, %edx
	POP	%ebx		# bits
	POP	%ecx		# disp
	POP	%ecx		# ifile
	POP	%eax		# retrieve reg

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

	LEA	-112(%ebp), %ecx
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
	

type_06_im:
	#  Shift %edx so %dl is the opcode byte and %dh the extension bits
	MOVB	$16, %cl
	SHRL	%edx		# by %cl
	PUSH	%edx		# store op info

	#  Write the opcode byte from %dl
	LEA	-112(%ebp), %ecx
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

	LEA	-104(%ebp), %ecx	# ifile
	PUSH	%ecx
	CALL	skiphws
	POP	%ecx
	POP	%edx			# retrieve op info
	CMPB	$0x24, %al		# '$'
	JE	.L23b
	
	#  We've got a an intermediate label.  Get first char into buffer,
	#  and then read the label.
	PUSH	%edx			# store op info
	LEA	-104(%ebp), %ecx	# ifile
	PUSH	%ecx
	CALL	getone
	MOVB	%al, -96(%ebp)
	PUSH	%ebp			# main_frame
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
	POP	%ecx		# retrieve op info
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
	CALL	labelref
	POP	%ecx
	POP	%ebx		# bits
	POP	%ecx

	#  And write the immediate data
	LEA	-112(%ebp), %ecx
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
	LEA	-104(%ebp), %ecx	# ifile
	PUSH	%ecx
	CALL	skiphws
	POP	%ecx	# ifile
	POP	%edx

	#  Need to determine which of the three types of instruction it is.
	CMPB	$0x25, %al	# '%'
	JE	type_06_rg
	CMPB	$0x28, %al	# '('
	JE	type_06_rm
	CMPB	$0x2D, %al	# '-'
	JE	type_06_rm
	PUSH	%eax
	CALL	dchr
	POP	%ecx
	CMPB	$-1, %al
	JNE	type_06_rm
	JMP	type_06_im

set_sect:
	#  When we jumped here, %edx is the new section number
	PUSH	%edx			# Store across MUL
	MOVL	-132(%ebp), %eax	# current parse_sect
	MOVL	$4, %ecx
	MULL	%ecx
	ADDL	%ebp, %eax
	MOVL	-108(%ebp), %ecx	# current offset
	MOVL	%ecx, -140(%eax)	# and store it

	POP	%eax
	MOVL	%eax, -132(%ebp)	# set parse_sect
	MOVL	$4, %ecx
	MULL	%ecx
	ADDL	%ebp, %eax
	MOVL	-140(%eax), %ecx	# current offset
	MOVL	%ecx, -108(%ebp)	# and load it
	MOVL	-148(%eax), %ecx	# current fd
	MOVL	%ecx, -112(%ebp)	# and load it

	JMP	insn_end

hex_bytes:
	#  Skip horizontal whitespace and read a byte
	LEA	-104(%ebp), %ecx	# ifile
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
	LEA	-104(%ebp), %ecx	# ifile
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
	LEA	-112(%ebp), %ecx	# ofile
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
	LEA	-104(%ebp), %eax	# ifile
	PUSH	%ecx	# we care about this
	PUSH	%eax
	PUSH	%edx
	CALL	unread
	POP	%ecx
	POP	%ecx
	POP	%ecx

	#  Look up the mnemonic or directive
	#  %esi points to the null-terminated identifier name
	MOVL	-116(%ebp), %edi
	SUBL	$16, %edi
.L14a:
	ADDL	$16, %edi		# sizeof(mnemonic)
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
	#  and see whether it's a directive (which must be a .hex_bytes)
	MOVL	12(%edi), %edx
	CMPB	$0xFF, %dl
	JNE	.L14b
	
	#  Directives
	CMPB	$0x00, %dh
	JE	hex_bytes
	MOVB	$0x10, %cl
	SHRL	%edx
	JMP	set_sect

.L14b:
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

	#  --- The main parsing loop.
	#  We CALL it and RET from it, but continue to use %ebp from 
	#  _start (the caller) so it's not a proper function.
.L8:
	#  Read one byte (not with readonex because EOF is permitted)
	LEA	-104(%ebp), %ecx
	PUSH	%ecx
	LEA	-96(%ebp), %ecx
	PUSH	%ecx
	CALL	readone
	POP	%edx
	POP	%edx
	CMPL	$0, %eax
	JL	error
	MOVL	%eax, %ebx  	# zero exit status
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
	LEA	-104(%ebp), %ecx	# ifile
	PUSH	%ecx
	CALL	skiphws
	POP	%ecx	# ifile

	#  Ordinarily, the next character should be either a '\n', '#' or ';',
	#  but we'll be permissive here to allow things like REPE CMPSB.  So
	#  we make the ';' between instructions on a single line optional.
	CMPB	$0x3B, %al	# ';'
	JNE	.L8	# Permissive

	#  Skip over the semicolon (statement separator)
	LEA	-104(%ebp), %ecx	# ifile
	PUSH	%ecx
	CALL	getone
	POP	%ecx	# ifile

	#  Nothing else is permitted here.	
	JMP	.L8

	#  --- The main loop
_start:
	MOVL	%esp, %ebp
	SUBL	$152, %esp

	#  Check we have a command line argument
	CMPL	$2, 0(%ebp)
	JNE	error

	#  Open the file
	XORL	%ecx, %ecx		# 0 == O_RDONLY
	MOVL	8(%ebp), %ebx
	MOVL	$5, %eax		# 5 == __NR_open
	INT	$0x80
	CMPL	$0, %eax
	JL	error

	#  Set up the stream objects
	MOVL	$-1, -112(%ebp)		# fout.fd = -1: no write on pass #1
	MOVL	$0, -108(%ebp)		# zero the count 
	MOVL	%eax, -104(%ebp) 	# fin.fd = fd returned by open
	MOVL	$0, -100(%ebp)		# putback boolean and data

	#  Set up the label vector
	MOVL	$1024, %ecx
	PUSH	%ecx
	CALL	malloc
	POP	%ecx
	MOVL	%eax, -4(%ebp)
	MOVL	%eax, -8(%ebp)
	ADDL	%ecx, %eax		 # %ecx still has size
	MOVL	%eax, -12(%ebp)

	#  Set up the rel vector
	MOVL	$512, %ecx
	PUSH	%ecx
	CALL	malloc
	POP	%ecx
	MOVL	%eax, -120(%ebp)
	MOVL	%eax, -124(%ebp)
	ADDL	%ecx, %eax		 # %ecx still has size
	MOVL	%eax, -128(%ebp)

	#  Locate the mnemonics table:  instrct* mnemonics = &mnemonics;
	#  CALL next_line; next_line: POP %eax  simply effects  MOV %eip, %eax
	CALL	.L8a
.L8a:
	POP	%eax		# Currently assembled sub-optimally as 2 bytes
	ADDL	mnemonics, %eax	# Assembled as six bytes (op, modrm, imm32)

	#  And add the length of the previous two instructions
	ADDL	$8, %eax   	# len of prev two instr
	MOVL	%eax, -116(%ebp)

	#  Default to the .text section
	MOVL	$0, -132(%ebp)
	MOVL	$0, -136(%ebp)	# size of .text
	MOVL	$0, -140(%ebp)	# size of .data 
	MOVL	$-1, -144(%ebp)	# set fds to -1
	MOVL	$-1, -148(%ebp)

	#  Pass #1
	CALL	.L8

	#  Change back to the .text section
	XORL	%edx, %edx
	CALL	set_sect

	#  Determine the output filename
	PUSH	8(%ebp)
	CALL	strlen
	POP	%ecx
	ADDL	8(%ebp), %eax
	CMPB	$0x73, -1(%eax)		# 's'
	JNE	error
	CMPB	$0x2E, -2(%eax)		# '.'
	JNE	error
	MOVB	$0x6F, -1(%eax)		# 'o'

	#  And create the output file
	MOVL	$0x1A4, %ecx		# 0x1A4 == 0644 (file creation mode)
	MOVL	8(%ebp), %ebx
	MOVL	$8, %eax		# 8 == __NR_creat
	INT	$0x80
	CMPL	$0, %eax
	JL	error
	MOVL	%eax, -112(%ebp)	# fout.fd = return from creat

	#  Locate the ELF header
	#  CALL next_line; next_line: POP %eax  simply effects  MOV %eip, %eax
	CALL	.L8b
.L8b:
	POP	%ecx		# Currently assembled sub-optimally as 2 bytes
	ADDL	elf_hdr, %ecx	# Assembled as six bytes (op, modrm, imm32)
	ADDL	$8, %ecx	# len of prev two instr

	#  Write the ELF header
	#  Don't use writedptr because we don't want to update ofile->count
	MOVL	%eax, %ebx
	MOVL	$0x1A4, %edx		# Length of ELF header
	MOVL	$4, %eax		# 4 == __NR_write
	INT	$0x80
	CMPL	$0, %eax
	JL	error

	#  Offset 0x70 needs .text section size
	LEA	-112(%ebp), %eax
	PUSH	%eax			# ofile
	MOVL	$0x70, %ecx
	PUSH	%ecx
	MOVL	-140(%ebp), %eax	# .text size
	PUSH	%eax
	CALL	writedwat
	POP	%eax
	POP	%ecx

	#  And offset 0x138 needs .data section size
	MOVL	$0x138, %ecx
	PUSH	%ecx
	MOVL	-136(%ebp), %eax	# .data size
	PUSH	%eax
	CALL	writedwat
	POP	%eax
	POP	%ecx
	POP	%eax	# ofile

	#  Reset write counter
	MOVL	$0, -108(%ebp)
	MOVL	$0, -140(%ebp)
	MOVL	$0, -136(%ebp)

	#  Seek to the beginning of input for second pass, and reset counter
	XORL	%edx, %edx		# SEEK_SET=0
	XORL	%ecx, %ecx		# offset = 0
	MOVL	-104(%ebp), %ebx	# fd
	MOVL	$19, %eax		# 19 == __NR_lseek
	INT	$0x80
	CMPL	$0, %eax
	JL	error

	#  Pass #2 -- write .text
	MOVL	-112(%ebp), %eax	# fout.fd
	MOVL	%eax, -148(%ebp)	# sect_fd[.text]
	CALL	.L8

	#  Change back to the .text section
	XORL	%edx, %edx
	CALL	set_sect

	#  Pad to a 4-byte boundary
	LEA	-112(%ebp), %eax
	PUSH	%eax			# ofile
	MOVL	$4, %ecx
	PUSH	%ecx
	CALL	writepad
	POP	%ecx
	POP	%ecx			# ofile

	#  Offset 0x134 needs offset of .data
	LEA	-112(%ebp), %eax
	PUSH	%eax			# ofile
	MOVL	$0x134, %ecx
	PUSH	%ecx
	MOVL	-108(%ebp), %eax	# .text size + padding
	ADDL	$0x1A4, %eax		# size of ELF headers
	PUSH	%eax
	CALL	writedwat
	POP	%eax
	POP	%ecx
	POP	%eax

	#  Reset write counter
	PUSH	-108(%ebp)		# store .text + padding
	MOVL	$0, -108(%ebp)
	MOVL	$0, -140(%ebp)
	MOVL	$0, -136(%ebp)

	#  Seek to the beginning of input for third pass, and reset counter
	XORL	%edx, %edx		# SEEK_SET=0
	XORL	%ecx, %ecx		# offset = 0
	MOVL	-104(%ebp), %ebx	# fd
	MOVL	$19, %eax		# 19 == __NR_lseek
	INT	$0x80
	CMPL	$0, %eax
	JL	error

	#  Pass #3 -- write .data
	MOVL	-148(%ebp), %eax	# sect_fd[.text]
	MOVL	%eax, -144(%ebp)	# sect_fd[.data]
	MOVL	$-1, -148(%ebp)
	CALL	.L8

	#  Add .text and .data sizes together
	POP	%eax			# pop .text + padding
	ADDL	%eax, -108(%ebp)	# add .data + padding

	#  Pad to a 4-byte boundary
	LEA	-112(%ebp), %eax
	PUSH	%eax			# ofile -- left on stack for ages **
	MOVL	$4, %ecx
	PUSH	%ecx
	CALL	writepad
	POP	%ecx

	#  Fix up ELF header to point to symbol table
	#  Offset 0xBC needs offset to symbol table
	MOVL	$0xBC, %ecx
	PUSH	%ecx
	MOVL	-108(%ebp), %eax	# .text size + padding
	ADDL	$0x1A4, %eax		# ELF header size
	PUSH	%eax
	CALL	writedwat
	POP	%eax
	POP	%ecx

	#  Store the current file offset so we can determine section size
	#  %ebx is the global symbol number
	MOVL	-108(%ebp), %esi
	XORL	%ebx, %ebx

	#  Write Null symbol (per gABI-4.1, Fig 4-18)
	XORL	%eax, %eax	# 0 -- placeholder for string
	PUSH	%eax
	CALL	writedword
	CALL	writedword	
	CALL	writedword	
	CALL	writedword	
	POP	%eax
	INCL	%ebx

	#  Write .symtab section
	MOVL	-4(%ebp), %edi
	SUBL	$16, %edi		# labels - 1 

.L8c:	
	#  Loop: Generate the symbol table, initially without string pointers
	ADDL	$16, %edi		# sizeof(label)
	CMPL	-8(%ebp), %edi
	JGE	.L8d			# finished

	CMPB	$0x2E, (%edi)		# '.'
	JNE	.L8e
	CMPB	$0x4C, 1(%edi)		# 'L'
	JE	.L8c
.L8e:
	#  We have a non-local symbol
	XORL	%eax, %eax	# 0 -- placeholder for string
	PUSH	%eax
	CALL	writedword	
	POP	%eax

	#  Use 0 as the address of an undefined symbol
	MOVL	0xC(%edi), %eax		# offset from start of .text
	CMPL	$-1, %eax
	JNE	.L8e2
	XORL	%eax, %eax
.L8e2:
	PUSH	%eax
	CALL	writedword	
	POP	%eax

	XORL	%eax, %eax		# 0 -- symbol size is not known
	PUSH	%eax
	CALL	writedword	
	POP	%eax
	
	MOVL	$0x00010010, %eax	# STB_GLOBAL, STT_NOTYPE, .text
	MOVL	0xC(%edi), %ecx		# offset from start of .text
	CMPL	$-1, %ecx
	JNE	.L8e3
	MOVL	$0x00000010, %eax	# STB_GLOBAL, STT_NOTYPE, SHN_UNDEF
.L8e3:
	PUSH	%eax
	CALL	writedword	
	POP	%eax

	#  The symbol value is not needed any more.  
	#  Instead store the symbol table number.
	MOVL	%ebx, 0xC(%edi)
	INCL	%ebx
	JMP	.L8c
.L8d:
	#  Fix up ELF header to point to string table
	#  Offset 0xE4 needs offset to string table
	MOVL	$0xE4, %ecx
	PUSH	%ecx
	MOVL	-108(%ebp), %eax	# .text + .rel.text + padding + .symtab
	ADDL	$0x1A4, %eax		# ELF header size
	PUSH	%eax
	CALL	writedwat
	POP	%eax
	POP	%ecx

	# And offset 0xC0 needs the size of the symbol table
	# %esi is counter at start of .symtab
	MOVL	$0xC0, %ecx
	PUSH	%ecx
	MOVL	-108(%ebp), %eax	# .text + .rel.text + padding + .symtab
	SUBL	%esi, %eax		# .text + .rel.text + padding
	PUSH	%eax
	CALL	writedwat
	POP	%eax
	POP	%ecx
	
	# %edi is the label pointer; %esi is the offset for the next symbol
	# table entry; %ebx is the start of the string section
	MOVL	-4(%ebp), %edi
	SUBL	$16, %edi		# labels - 1   (sizeof(label))
	ADDL	$0x1B4, %esi		# ELF header size (0x1A4) + null symbol
	MOVL    -108(%ebp), %ebx

	# Null symbol's name
	XORL	%eax, %eax
	PUSH	%eax
	CALL	writebyte
	POP	%eax
.L8f:
	# Loop writing strings and fixing up references to symbol names
	ADDL	$16, %edi		# sizeof(label)
	CMPL	-8(%ebp), %edi
	JGE	.L8i			# finished

	CMPB	$0x2E, (%edi)		# '.'
	JNE	.L8h
	CMPB	$0x4C, 1(%edi)		# 'L'
	JE	.L8f
.L8h:
	#  We have a non-local symbol
	PUSH	%esi			# Symbol table entry (address to write)
	MOVL	-108(%ebp), %eax
	SUBL	%ebx, %eax		# Offset into string table
	PUSH	%eax
	CALL	writedwat
	POP	%eax
	POP	%eax
	ADDL	$16, %esi		# sizeof(label)

	#  Write its symbol
	PUSH	%edi
	CALL	strlen
	INCL	%eax		# Write '\0' too
	PUSH	%eax
	CALL	writedptr
	POP	%eax
	POP	%ecx
	JMP	.L8f
.L8i:
	#  Fix up ELF header for string table
	#  Offset 0xE8 needs size of string table
	MOVL	$0xE8, %eax
	PUSH	%eax
	MOVL	-108(%ebp), %eax
	SUBL	%ebx, %eax		# Offset into string table
	PUSH	%eax
	CALL	writedwat
	POP	%eax
	POP	%eax
	
	#  Pad to an 8-byte boundary
	MOVL	$8, %ecx
	PUSH	%ecx
	CALL	writepad
	POP	%ecx

	#  Offset 0x10C needs offset to start of .rel.text
	MOVL	$0x10C, %eax
	PUSH	%eax
	MOVL	-108(%ebp), %eax 	# ofile->count
	ADDL	$0x1A4, %eax		# Length of ELF header
	PUSH	%eax
	CALL	writedwat
	POP	%eax
	POP	%eax

	#  Store the current file offset so we can determine section size
	MOVL	-108(%ebp), %esi

	#  Write the .rel.text section, initially without any .symtab links
	MOVL	-120(%ebp), %edi

.L25:
	CMPL	-124(%ebp), %edi
	JGE	.L26			# finished

	#  Relocation offset
	PUSH	(%edi)			# r_offset
	CALL	writedword
	POP	%eax

	#  Get the symbol number
	MOVL	4(%edi), %eax		# in-memory symbol number
	XORL	%edx, %edx
	MOVL	$16, %ecx
	MULL	%ecx			# acts on %edx:%eax
	ADDL	-4(%ebp), %eax		# label_start
	MOVL	0xC(%eax), %eax		# look up the value for the global no.

	#  Compose and write the r_info field
	MOVB 	$8, %cl
	SHLL	%eax			# by %cl
	ADDL	$2, %eax		# R_386_PC32
	PUSH	%eax
	CALL	writedword
	POP	%eax

	ADDL	$8, %edi
	JMP	.L25

.L26:
	# Offset 0x110 needs the size of the .rel.text table
	# %esi is counter at start of .rel.text
	MOVL	$0x110, %ecx
	PUSH	%ecx
	MOVL	-108(%ebp), %eax	# .text + padding + .rel.text
	SUBL	%esi, %eax		# .text + padding
	PUSH	%eax
	CALL	writedwat
	POP	%eax
	POP	%ecx

POP	%ecx			# ofile -- from way above **

	XORL	%ebx, %ebx
	JMP	success

####    #  And finally, the entry point.
	#  Last per requirement for elfify.
	JMP    _start 
