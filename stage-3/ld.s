# ld.s

# Copyright (C) 2011, 2012 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

# ########################################################################

elf_hdr:

### The ELF header.      start: 0x00; length: 0x34
# (Offset 0x20 requires offset to section headers;
# and offset 0x18 requires the entry point address.)
# 
#                        32-bit, LSB ('Intel') byte order, ELF v.1
#	magic-----\  |      /--------------------------------/
.hex	7F 45 4C 46  01 01 01 00    00 00 00 00  00 00 00 00
#	exe-\ 386-\  ELF-v1----\    **entry**---\  phdr-off--\
.hex	02 00 03 00  01 00 00 00    00 00 00 00  34 00 00 00
#	**shdroff**  flags-----\    ehsz\
.hex	00 00 00 00  00 00 00 00    34 00
#	phsz\ phn-\  shsz\ shn-\    shstr\
.hex	20 00 02 00  28 00 04 00    03 00 

# Note that ELF requires the offset of a loadable segment to be
# equal to its virtual address modulo its alignment.  

# Text segment program header.  start: 0x34; length: 0x20
# (Offsets 0x44 and 0x48 require .text size)
#
#	PT_LOAD---\  offset----\    vaddr-----\  paddr-----\
.hex	01 00 00 00  74 00 00 00    74 80 04 08  74 80 04 08 
#	**filesz**\  **memsz**-\    PF_R|PF_X-\  align-----\
.hex	00 00 00 00  00 00 00 00    05 00 00 00  00 10 00 00

# Data segment program header.  start: 0x54; length: 0x20
# (Offsets 0x5C and 0x60 require load address)
# (Offsets 0x64 and 0x68 require .data size)
#
#	PT_LOAD---\  offset----\    vaddr-----\  paddr-----\
.hex	01 00 00 00  74 00 00 00    74 90 04 08  74 90 04 08
#	**filesz**\  **memsz**-\    PF_R|PF_W-\  align-----\
.hex	00 00 00 00  00 00 00 00    06 00 00 00  00 10 00 00

# end of ELF + Program headers -- position: 0x74

sect_hdr:

# #0  Null section header.   start: +0x00; length: 0x28
# c.f. Fig 4.10 in gABI 4.1
.hex	00 00 00 00  00 00 00 00    00 00 00 00  00 00 00 00
.hex	00 00 00 00  00 00 00 00    00 00 00 00  00 00 00 00
.hex	00 00 00 00  00 00 00 00

# #1  Text section header.   start: +0x28; length: 0x28
# (Offset +0x3C requires text section size.)
#
#	name-str--\  PROGBITS--\    EXEC|ALLOC\  load-addr-\
.hex	01 00 00 00  01 00 00 00    06 00 00 00  74 80 04 08
#	offset----\  **size**--\    /-- Fig 4.12 in gABI --\
.hex	74 00 00 00  00 00 00 00    00 00 00 00  00 00 00 00
#	align-----\  entry-sz--\
.hex	04 00 00 00  00 00 00 00

# #2  Data section header.   start: +0x50; length: 0x28
# (Offset +0x60, +0x64 requires data section offset and size.)
#
#	name-str--\  PROGBITS--\    WRITE|ALLOC\ **load**--\
.hex	07 00 00 00  01 00 00 00    03 00 00 00  74 90 04 08
#	**offset**\  **size**--\    /-- Fig 4.12 in gABI --\
.hex	74 00 00 00  00 00 00 00    00 00 00 00  00 00 00 00
#	align-----\  entry-sz--\
.hex	04 00 00 00  00 00 00 00

# #3  Shstrtab section hdr.  start: +0x78; length: 0x28
# (Offset +0x88 requires offset to shstrtab.)
#
#	name-str--\  STRTAB----\    no-flags--\  load-addr-\ 
.hex	0D 00 00 00  03 00 00 00    00 00 00 00  00 00 00 00
#	**offset**\  size------\    /-- Fig 4.12 in gABI --\
.hex	00 00 00 00  16 00 00 00    00 00 00 00  00 00 00 00
#	align-----\  entry-sz--\
.hex	01 00 00 00  00 00 00 00

# The shared string table itself.  start +0xA0; length: 0x16
#
.hex	00                             # NULL.      Offset 0x00
.hex	2E 74 65 78 74 00              # .text      Offset 0x01
.hex	2E 64 61 74 61 00              # .data      Offset 0x07
.hex	2E 73 68 73 74 72 74 61 62 00  # .shstrtab  Offset 0x0D

# End of end headers.   offset 0xB6


# ########################################################################

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


	####	#  Function:	bool strneq( char* s1, char* s2, int n )
strneq:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%esi
	PUSH	%edi

	XORL	%eax, %eax
	MOVL	8(%ebp), %edi
	MOVL	12(%ebp), %esi
	MOVL	16(%ebp), %ecx
	REP CMPSB
	JNE	.L3
	INCL	%eax
.L3:
	POP	%edi
	POP	%esi
	POP	%ebp
	RET


####	#  Function:  int findsect( void* elf, void** ptr, int strlen, 
	#                           char* name )
findsect:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%ebx
	PUSH	%edi
	PUSH	%esi
	MOVL	8(%ebp), %ebx

	#  Find shoff (at 0x20)
	MOVL	0x20(%ebx), %eax
	ADDL	%ebx, %eax
	PUSH	%eax			# shoff		-16(%ebp)

	#  Find shentsz (at 0x2E)
	MOVL	0x2E(%ebx), %eax
	ANDL	$0x0000FFFF, %eax
	PUSH	%eax			# shentsz 	-20(%ebp)
	
	#  Find endshstr from shnum (at 0x30)
	MOVL	0x30(%ebx), %eax
	ANDL	$0x0000FFFF, %eax
	MULL	-20(%ebp)		# acts on %eax: result in %edx:%eax
	ADDL	-16(%ebp), %eax
	PUSH	%eax			# endshstr	-24(%ebp)

	#  Find .shstrtab section
	MOVL	0x32(%ebx), %eax
	ANDL	$0x0000FFFF, %eax
	MULL	-20(%ebp)		# acts on %eax: result in %edx:%eax
	ADDL	-16(%ebp), %eax		# beginning of shstrtab section
	MOVL	0x10(%eax), %eax	# sh_offset member
	ADDL	%ebx, %eax
	PUSH	%eax			# shstrstr	-28(%ebp)

	#  Set up stack ready for call to strneq
	MOVL	16(%ebp), %eax		# strlen
	INCL	%eax			# +1 for '\0'
	PUSH	%eax
	PUSH	20(%ebp)		# Pointer to NAME

	#  Iterate through section looking for one with name NAME
	XORL	%esi, %esi
	MOVL	-16(%ebp), %edi
.L1:
	CMPL	-24(%ebp), %edi
	JGE	error

	#  The section name
	MOVL	(%edi), %eax
	ADDL	-28(%ebp), %eax
	PUSH	%eax
	CALL	strneq
	POP	%ecx
	CMPL	$0, %eax
	JNE	.L2

	ADDL	-20(%ebp), %edi
	INCL	%esi
	JMP	.L1

.L2:
	ADDL	$0x18, %esp		# POP x6

	#  Write the pointer to PTR
	MOVL	12(%ebp), %eax
	CMPL	$0, %eax
	JE	.L2a
	MOVL	%edi, (%eax)

.L2a:
	#  Return the section number
	MOVL	%esi, %eax

	POP	%esi
	POP	%edi
	POP	%ebx
	POP	%ebp
	RET


####	#  Function:  void* readsyms( void* elf, size_t offset, 
	#                             size_t strlen, char* name,
	#                             vec<label>* syms, vec<label>* relocs,
	#                             int sectid )
	#
	#  Populate the SYM vector with data from .symtab / .strtab for
	#  the NAME section (which has length STRLEN) of ELF, 
	#  adding OFFSET to each symbol.  Return 
readsyms:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%ebx
	PUSH	%edi
	PUSH	%esi

	#  Slot for pointer to section NAME header **
	PUSH	%eax			# -16(%ebp)
	MOVL	%esp, %ecx		# ptr to NAME scratch, above

	#  Find the NAME section
	PUSH	20(%ebp)		# NAME
	PUSH	16(%ebp)		# STRLEN
	PUSH	%ecx
	PUSH	8(%ebp)			# The elf file image
	CALL	findsect
	ADDL	$16, %esp		# POP x4
	PUSH	%eax			# NAME header index -20(%ebp) **

	#  The string literal ".symtab"
	PUSH	%eax			# scratch
	MOVL	%esp, %ecx		# ptr to scratch
	MOVL	$0x626174, %eax		# 'tab'
	PUSH	%eax
	MOVL	$0x6D79732E, %eax	# '.sym'
	PUSH	%eax
	PUSH	%esp			# ptr to string
	MOVL	$7, %eax		# strlen(".symtab")
	PUSH	%eax
	PUSH	%ecx
	PUSH	8(%ebp)			# The elf file image
	CALL	findsect
	ADDL	$24, %esp		# POP x6
	POP	%ebx			# Pointer to section header <= scratch

	#  TODO: should use st_link from the symbol table to find this.
	#  The string literal ".strtab"
	PUSH	%eax			# scratch
	MOVL	%esp, %ecx		# ptr to scratch
	MOVL	$0x626174, %eax
	PUSH	%eax
	MOVL	$0x7274732E, %eax
	PUSH	%eax
	PUSH	%esp			# ptr to string
	MOVL	$7, %eax
	PUSH	%eax			# strlen(".symtab")
	PUSH	%ecx
	PUSH	8(%ebp)			# The elf file image
	CALL	findsect
	ADDL	$24, %esp		# POP x6
	POP	%eax

	#  Find pointer to .symtab section => %edi; .strtab section => %esi
	MOVL	0x10(%ebx), %edi	
	ADDL	8(%ebp), %edi
	PUSH	%edi			# elf **  -24(%ebp)
	MOVL	0x10(%eax), %esi
	ADDL	8(%ebp), %esi

	#  Loop over symbol table
.L10:
	#  Check whether we're overrunning the object file's symbol table
	MOVL	0x10(%ebx), %ecx	# offset of start of section
	ADDL	0x14(%ebx), %ecx	# + size of section
	ADDL	8(%ebp), %ecx		# => ptr to end of section
	CMPL	%ecx, %edi
	JGE	.L11			# Done

	#  Check the symbol is in the correct section
	#  NB  If the symbol is undefined, the section will be SHN_UNDEF==0
	MOVL	0xC(%edi), %eax
	MOVB	$16, %cl
	SHRL	%eax 			# %eax is now st_shndx
	CMPL	-20(%ebp), %eax
	JNE	.L12			# continue

	#  Check we're not about to overrun the label vector
	MOVL	24(%ebp), %edx
	MOVL	4(%edx), %eax		# vec.end
	CMPL	8(%edx), %eax		# vec.end_store
	JL	.L13

	#  Grow storage
	MOVL	8(%edx), %eax
	SUBL	(%edx), %eax
	XORL	%edx, %edx
	MOVL	$2, %ecx
	MULL	%ecx			# acts on %edx:%eax
	PUSH	%eax			# new size (twice old)
	MOVL	24(%ebp), %edx
	PUSH	(%edx)			# ptr
	CALL	realloc

	#  Store pointers
	MOVL	24(%ebp), %edx
	MOVL	%eax, (%edx)		# store new pointer
	POP	%ecx
	SUBL	%ecx, 4(%edx)
	ADDL	%eax, 4(%edx)		# new vec->end
	POP	%ecx
	ADDL	%eax, %ecx
	MOVL	%ecx, 8(%edx)		# new vec->end_store

.L13:
	#  Do a strcpy
	PUSH	%esi
	ADDL	(%edi), %esi		# source = string table + st_name 
	PUSH	%esi
	CALL	strlen
	POP	%esi
	CMPL	$12, %eax
	JGE	error			# Buffer overrun
	MOVL	%eax, %ecx
	INCL	%ecx			# +1 for NUL char
	PUSH	%edi
	MOVL	24(%ebp), %eax
	MOVL	4(%eax), %edi		# dest = vec.end
	REP MOVSB
	POP	%edi
	POP	%esi

	#  Copy the symbol address
	MOVL	0x4(%edi), %ecx		# st_value
	ADDL	12(%ebp), %ecx		# + offset
	MOVL	24(%ebp), %edx		# vec
	MOVL	4(%edx), %eax		# dest = vec->end
	MOVL	%ecx, 12(%eax)

	#  Store the section id in label->sect.
	MOVL	32(%ebp), %ecx
	MOVL	%ecx, 16(%eax)

	ADDL	$24, 4(%edx)		# ++vec->end;  sizeof(label)
.L12:
	ADDL	0x24(%ebx), %edi	# Add sh_entsize
	JMP	.L10

.L11:
	PUSH	%ebx			# -28(%ebp) == ptr to .symtab

	#  Search for .relNAME section header
	PUSH	%eax			# -32(%ebp)
	MOVL	%esp, %ecx

	#  Copy 12 bytes from NAME
	MOVL	20(%ebp), %eax
	PUSH	8(%eax)
	PUSH	4(%eax)
	PUSH	(%eax)
	MOVL	$0x6C65722E, %eax	# '.rel'
	PUSH	%eax

	#  Find the .relNAME section
	PUSH	%esp			# name
	MOVL	16(%ebp), %eax
	ADDL	$4, %eax		# 4 == strlen(".rel")
	PUSH	%eax			# strlen
	PUSH	%ecx
	PUSH	8(%ebp)			# The elf file image
	CALL	findsect
	ADDL	$32, %esp		# POP x8
	POP	%ebx			# -32(%ebp): start of .relNAME section

	#  Find pointer to .relNAME section
	MOVL	0x10(%ebx), %edi
	ADDL	8(%ebp), %edi

	#  Loop over relocation table
.L18:
	MOVL	0x10(%ebx), %ecx	# offset of start of section
	ADDL	0x14(%ebx), %ecx	# + size of section
	ADDL	8(%ebp), %ecx		# => ptr to end of section
	CMPL	%ecx, %edi
	JGE	.L19			# Done

	#  Check we're not about to overrun the relocation vector
	MOVL	28(%ebp), %edx
	MOVL	4(%edx), %eax		# vec.end
	CMPL	8(%edx), %eax		# vec.end_store
	JL	.L20

	#  Grow storage
	MOVL	8(%edx), %eax
	SUBL	(%edx), %eax
	XORL	%edx, %edx
	MOVL	$2, %ecx
	MULL	%ecx			# acts on %edx:%eax
	PUSH	%eax			# new size (twice old)
	MOVL	28(%ebp), %edx
	PUSH	(%edx)			# ptr
	CALL	realloc

	#  Store pointers
	MOVL	28(%ebp), %edx
	MOVL	%eax, (%edx)		# store new pointer
	POP	%ecx
	SUBL	%ecx, 4(%edx)
	ADDL	%eax, 4(%edx)		# new vec->end
	POP	%ecx
	ADDL	%eax, %ecx
	MOVL	%ecx, 8(%edx)		# new vec->end_store

.L20:
	#  (%edi) is the r_offset; 4(%edi) is the r_info field
	MOVL	4(%edi), %eax		# r_info
	MOVB	$8, %cl
	SHRL	%eax			# %eax is now r_sym from r_info

	#  Look up symbol
	XORL	%edx, %edx
	MOVL	$16, %ecx
	MULL	%ecx			# %eax is now offset into .symtab
	MOVL	-28(%ebp), %edx		# ptr to .symtab header
	ADDL	16(%edx), %eax		# offset to .symtab section
	ADDL	8(%ebp), %eax		# %eax is now a symbol 
	#MOVL	12(%eax), %eax		# st_info, st_other, st_shndx
	#MOVB	$16, %cl
	#SHRL	%eax			# st_shndx

	#  Do a strcpy
	PUSH	%esi
	ADDL	(%eax), %esi		# source = string table + st_name
	PUSH	%esi
	CALL	strlen
	POP	%esi
	CMPL	$12, %eax
	JGE	error
	MOVL	%eax, %ecx
	INCL	%ecx			# +1 for NUL char
	PUSH	%edi
	MOVL	28(%ebp), %eax
	MOVL	4(%eax), %edi		# dest = vec.end
	REP MOVSB
	POP	%edi
	POP	%esi
	
	#  Store relocation offset
	MOVL	(%edi), %ecx		# r_offset
	ADDL	12(%ebp), %ecx		# + offset
	MOVL	28(%ebp), %edx		# vec
	MOVL	4(%edx), %eax		# dest = vec->end
	MOVL	%ecx, 12(%eax)
	MOVL	32(%ebp), %ecx		# sectid
	MOVL	%ecx, 16(%eax)		# rel->sect

	#  We only understand two types of relocation
	MOVL	4(%edi), %eax		# %cl is r_type from r_info
	CMPB	$2, %al			# 2 == R_386_PC32
	JE	.L20a
	CMPB	$1, %al			# 1 == R_386_32
	JNE	error
.L20a:
	#  Store the relocation type in label->type
	XORL	%ecx, %ecx
	MOVB	%al, %cl
	MOVL	4(%edx), %eax		# dest = vec->end
	MOVL	%ecx, 20(%eax)

	ADDL	$24, 4(%edx)		# ++vec->end;  sizeof(label)

	ADDL	0x24(%ebx), %edi	# Add sh_entsize
	JMP	.L18
.L19:
	ADDL	$12, %esp		# POP x3: -28(%ebp) ... -20(%ebp)
	POP	%eax			# return pointer to NAME header **

	POP	%esi
	POP	%edi
	POP	%ebx
	POP	%ebp
	RET


####	#  Function:  label* findsym( size_t strlen, char* name,
	#                             vec<label>* syms )
	#
	#  Return the symbol NAME (with length STRLEN), in the 
	#  symbol table SYMS.
findsym:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%edi

	PUSH	8(%ebp)			# strlen
	PUSH	12(%ebp)		# name

	#  Scan the labels looking for NAME  
	MOVL	16(%ebp), %edx
	MOVL	(%edx), %edi		# syms[0].start
.L14:
	MOVL	16(%ebp), %edx
	CMPL	4(%edx), %edi		# sym->end
	JGE	error			# Not found

	PUSH	%edi
	CALL	strneq
	POP	%ecx
	CMPL	$0, %eax
	JNE	.L15			# Found it

	ADDL	$24, %edi		# sizeof(label)
	JMP	.L14
.L15:
	MOVL	%edi, %eax

	POP	%ecx
	POP	%ecx
	
	POP	%edi
	POP	%ebp
	RET


	#  The NAME contain the section name directly on the stack
####    #  Not a proper function.
	#  Exits program
error:
	MOVL	$1, %ebx
success:
	MOVL	$1, %eax   		# 1 == __NR_exit
	INT     $0x80


####	#  Function: int section( int fd_out, char* filename, 
	#                         char* sectname, int sectid, int offset,
	#                         vector<label>* syms, vector<label>* relocs,
	#                         int pad_chars );
section:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%eax		#  -4(%ebp)	void* in
	PUSH	%eax		#  -8(%ebp)	int fd_in
	PUSH	24(%ebp)	# -12(%ebp)	int offset 
	PUSH	%ebx
	PUSH	%edi

	#  Calculate padding required for 4-byte alignment
	XORL	%edx, %edx
	MOVL	-12(%ebp), %eax		# bytes written
	MOVL	$4, %ecx
	DIVL	%ecx			# acts on %edx:%eax
	SUBL	%edx, %ecx		# 4 - (bytes % 4)
	CMPL	$4, %ecx
	JE	.L23

	#  And write the padding
	PUSH	%ecx
	PUSH	36(%ebp)
	MOVL	%ecx, %edx
	MOVL	%esp, %ecx
	MOVL	8(%ebp), %ebx		# fd
	MOVL	$4, %eax   		# 4 == __NR_write
	INT	$0x80
	POP	%ecx
	POP	%ecx
	CMPL	%ecx, %eax
	JNE	error
	ADDL	%ecx, -12(%ebp)

.L23:
	#  Open the input file
	XORL	%ecx, %ecx		# 0 == O_RDONLY
	MOVL	12(%ebp), %ebx		# filename
	MOVL	$5, %eax		# 5 == __NR_open
	INT	$0x80
	CMPL	$0, %eax
	JL	error
	MOVL	%eax, -8(%ebp) 		# fd_in

	#  Find the size of the object file
	SUBL	$0x58, %esp		# sizeof(struct stat) per [new] fstat
	MOVL	%esp, %ecx
	MOVL	-8(%ebp), %ebx		# fd_in
	MOVL	$108, %eax		# 108 == __NR_fstat
	INT	$0x80
	CMPL	$0, %eax
	JL	error
	ADDL	$0x14, %esp		# move to file size member }
	POP	%edx			#                          } ADDL $0x58,
	ADDL	$0x40, %esp		# $0x58 - 0x14 - 4         }   %esp

	PUSH	%edx			# stay on stack for munmap **

	#  Map the input object file
	XORL	%eax, %eax
	PUSH	%eax			# 0 offset
	PUSH	-8(%ebp)		# fd_in
	MOVL	$2, %ecx
	PUSH	%ecx			# MAP_PRIVATE
	MOVL	$1, %ecx
	PUSH	%ecx			# PROT_READ
	PUSH	%edx			# file len
	PUSH	%eax			# NULL
	MOVL	%esp, %ebx
	MOVL	$90, %eax		# 90 == __NR_mmap
	INT	$0x80
	CMPL	$-4096, %eax		# -4095 <= %eax < 0 for errno
	JA	error			# unsigned comparison handles above
	MOVL	%eax, -4(%ebp)		# Save memory location
	ADDL	$0x18, %esp		# 6x POP

	#  Read the symbol table
	PUSH	20(%ebp)		# sectid
	PUSH	32(%ebp)		# relocs
	PUSH	28(%ebp)		# syms
	PUSH	16(%ebp)		# sectname
	CALL	strlen
	PUSH	%eax			# strlen(sectname)
	PUSH	-12(%ebp)		# previous .texts written 
	PUSH	-4(%ebp)
	CALL	readsyms
	ADDL	$0x1C, %esp		# POP x7
	MOVL	%eax, %edi		# Ptr to .text sect header

	#  Write out the section 
	MOVL	0x14(%edi), %edx	# sh_size member
	MOVL	0x10(%edi), %ecx	# sh_offset member
	ADDL	-4(%ebp), %ecx		# data
	MOVL	8(%ebp), %ebx		# fd
	MOVL	$4, %eax   		# 4 == __NR_write
	INT	$0x80
	CMPL	0x14(%edi), %eax
	JNE	error
	ADDL	%eax, -12(%ebp)		# Bytes written in .text

	#  Unmap and close the input file
	POP	%ecx			# file len from way above **
	MOVL	-4(%ebp), %ebx
	MOVL	$91, %eax		# __NR_munmap == 91
	INT	$0x80
	MOVL	-8(%ebp), %ebx		# fd_in
	MOVL	$6, %eax		# __NR_close == 6
	INT	$0x80

	MOVL	-12(%ebp), %eax		# return bytes written

	POP	%edi
	POP	%ebx
	ADDL	$12, %esp	
	POP	%ebp
	RET


####    #  The main function.
	#  Stack is arranged as follows:
	#
	#	-4(%ebp)	void* in    # mmapped file data
	#	-8(%ebp)        int32 fd_in
	#      -12(%ebp)	int32 bytes_written
	#      -16(%ebp)        int32 fd_out
	#      -28(%ebp)	vector<label> syms
	#      -40(%ebp)	vector<label> relocs
	#      -48(%ebp)	int32 sect_size[2]
	# 
	#  where vector<T> is a { T* start, end, end_store; },
	#  and label is a { char name[12]; int32 val, sect, type; }.
	#  In the SYMS vector, type is ignored; in the RELOCS vector, 
	#  type is the relocation type.

	#  --- The main loop
_start:
	MOVL	%esp, %ebp
	SUBL	$48, %esp	

	XORL	%eax, %eax
	MOVL	%eax, -12(%ebp)

	#  Check we have at least three command line argument (-o out in.o ...)
	CMPL	$4, 0(%ebp)	# three args => argc == 4
	JL	error

	#  Check argv[1] is '-o'
	MOVL	8(%ebp), %eax
	CMPB	$0x2D, (%eax)		# '-'
	JNE	error
	CMPB	$0x6F, 1(%eax)		# 'o'
	JNE	error
	CMPB	$0x0, 2(%eax)		# NUL
	JNE	error

	#  Open the output file
	MOVL	$0x1FD, %edx		# 0755
	MOVL	$0x242, %ecx		# O_RDWR=2|O_CREAT=0x40|O_TRUNC=0x200
	MOVL	12(%ebp), %ebx		# argv[2]
	MOVL	$5, %eax		# 5 == __NR_open
	INT	$0x80
	CMPL	$0, %eax
	JL	error
	MOVL	%eax, -16(%ebp)		# fout.fd

	#  Locate the ELF header
	#  CALL next_line; next_line: POP %eax  simply effects  MOV %eip, %eax
	CALL	.L4
.L4:
	POP	%ecx		# Currently assembled sub-optimally as 2 bytes
	ADDL	elf_hdr, %ecx	# Assembled as six bytes (op, modrm, imm32)
	ADDL	$8, %ecx	# len of prev two instr

	#  Write the ELF & program header
	MOVL	$0x74, %edx		# Length of ELF & program headers
	MOVL	-16(%ebp), %ebx		# fd
	MOVL	$4, %eax		# 4 == __NR_write
	INT	$0x80
	CMPL	$0x74, %eax
	JL	error

	#  Allocate the symbol vector
	MOVL	$1536, %ecx		# 64 * sizeof(label)
	PUSH	%ecx
	CALL	malloc
	POP	%ecx
	MOVL	%eax, -28(%ebp)
	MOVL	%eax, -24(%ebp)
	ADDL	%ecx, %eax
	MOVL	%eax, -20(%ebp)

	#  Allocate the undef vector
	MOVL	$1536, %ecx		# 64 * sizeof(label)
	PUSH	%ecx
	CALL	malloc
	POP	%ecx
	MOVL	%eax, -40(%ebp)
	MOVL	%eax, -36(%ebp)
	ADDL	%ecx, %eax
	MOVL	%eax, -32(%ebp)

	#  Loop over input files for .text
	MOVL	$3, %eax		# argn: about to read argv[argn]
	PUSH	%eax			# -52(%ebp)
.L16:
	POP	%eax
	CMPL	0(%ebp), %eax
	JGE	.L17
	INCL	%eax
	PUSH	%eax
	
	#  The string literal ".text"
	MOVL	$0x74, %eax		# 't'
	PUSH	%eax
	MOVL	$0x7865742E, %eax	# '.tex'
	PUSH	%eax
	MOVL	%esp, %ecx

	#  Read the symbol table
	MOVL	$0x90909090, %eax	# 0x90 is NOP; pad .text with this
	PUSH	%eax
	LEA	-40(%ebp), %eax
	PUSH	%eax			# &relocs
	LEA	-28(%ebp), %eax
	PUSH	%eax			# &syms
	PUSH	-12(%ebp)		# offset
	XORL	%edx, %edx
	PUSH	%edx			# sectid (0 for .text)
	PUSH	%ecx			# ptr to ".text"

	#  Stage 2/3 as doesn't support  PUSH (%ebp,%eax,4)
	MOVL	-52(%ebp), %eax
	MOVL	$4, %ecx
	MULL	%ecx
	ADDL	%ebp, %eax
	PUSH	(%eax)			# filename

	PUSH	-16(%ebp)
	CALL	section
	MOVL	%eax, -12(%ebp)
	ADDL	$40, %esp		# POP x10

	JMP	.L16			# End loop over input files
.L17:
	MOVL	-12(%ebp), %eax
	MOVL	%eax, -48(%ebp)
	MOVL	$0, -12(%ebp)

	#  Loop over input files for .data
	MOVL	$3, %eax		# argn: about to read argv[argn]
	PUSH	%eax			# -52(%ebp)
.L16a:
	POP	%eax
	CMPL	0(%ebp), %eax
	JGE	.L17a
	INCL	%eax
	PUSH	%eax
	
	#  The string literal ".data"
	MOVL	$0x61, %eax		# 'a'
	PUSH	%eax
	MOVL	$0x7461642E, %eax	# '.dat'
	PUSH	%eax
	MOVL	%esp, %ecx

	#  Read the symbol table
	XORL	%eax, %eax		# Pad with '\0' in .data
	PUSH	%eax
	LEA	-40(%ebp), %eax
	PUSH	%eax			# &relocs
	LEA	-28(%ebp), %eax
	PUSH	%eax			# &syms
	PUSH	-12(%ebp)		# offset
	XORL	%edx, %edx
	INCL	%edx
	PUSH	%edx			# sectid (1 for .data)
	PUSH	%ecx			# ptr to ".data"

	#  Stage 2/3 as doesn't support  PUSH (%ebp,%eax,4)
	MOVL	-52(%ebp), %eax
	MOVL	$4, %ecx
	MULL	%ecx
	ADDL	%ebp, %eax
	PUSH	(%eax)			# filename

	PUSH	-16(%ebp)
	CALL	section
	MOVL	%eax, -12(%ebp)
	ADDL	$40, %esp		# POP x10

	JMP	.L16a			# End loop over input files
.L17a:
	MOVL	-12(%ebp), %eax
	MOVL	%eax, -44(%ebp)
	MOVL	-48(%ebp), %eax
	ADDL	%eax, -12(%ebp)

	#  Locate the section headers & .shstrtab
	#  CALL next_line; next_line: POP %eax  simply effects  MOV %eip, %eax
	CALL	.L5
.L5:
	POP	%ecx		# Currently assembled sub-optimally as 2 bytes
	ADDL	sect_hdr, %ecx	# Assembled as six bytes (op, modrm, imm32)
	ADDL	$8, %ecx	# len of prev two instr

	#  Write the section headers & .shstrtab
	MOVL	$0xB6, %edx		# Length of headers
	MOVL	-16(%ebp), %ebx		# fd
	MOVL	$4, %eax		# 4 == __NR_write
	INT	$0x80
	CMPL	$0xB6, %eax
	JL	error

	#  The output file is completely written. 
	#  mmap it to fix up relocations
	XORL	%eax, %eax
	PUSH	%eax			# 0 offset
	PUSH	-16(%ebp)		# fd_out
	MOVL	$1, %ecx
	PUSH	%ecx			# MAP_SHARED
	MOVL	$3, %ecx
	PUSH	%ecx			# PROT_READ|PROT_WRITE
	MOVL	-12(%ebp), %ecx		# .text written
	ADDL	$0x74, %ecx		# Add ELF, etc.
	ADDL	$0xB6, %ecx		# Section hdrs, etc.
	PUSH	%ecx			# size
	XORL	%eax, %eax		# NULL 
	PUSH	%eax
	MOVL	%esp, %ebx
	MOVL	$90, %eax		# 90 == __NR_mmap
	INT	$0x80
	CMPL	$-4096, %eax		# -4095 <= %eax < 0 for errno
	JA	error			# unsigned comparison handles above
	MOVL	%eax, %ebx	

	#  Tell the program header the .text size
	MOVL	-48(%ebp), %ecx		# .text sh_size
	MOVL	%ecx, 0x44(%ebx)	# ELF p_filesz
	MOVL	%ecx, 0x48(%ebx)	# ELF p_memsz

	#  .data offset and size
	ADDL	%ecx, 0x58(%ebx)	# ELF p_offset
	ADDL	%ecx, 0x5C(%ebx)	# ELF p_vaddr
	ADDL	%ecx, 0x60(%ebx)	# ELF p_paddr

	MOVL	-44(%ebp), %ecx		# .data sh_size
	MOVL	%ecx, 0x64(%ebx)	# ELF p_filesz
	MOVL	%ecx, 0x68(%ebx)	# ELF p_memsz


	#  Link ELF header to the section headers
	MOVL	-12(%ebp), %ecx		# total .text + .data
	ADDL	$0x74, %ecx		# for ELF
	MOVL	%ecx, 0x20(%ebx)	# ELF shdroff

	#  .text section header needs .text section size
	MOVL	-12(%ebp), %ecx		# total .text + .data
	ADDL	$0x74, %ecx		# for ELF
	ADDL	$0x3C, %ecx		# size field in second section header
	ADDL	%ebx, %ecx
	MOVL	-48(%ebp), %eax		# .text size
	MOVL	%eax, (%ecx)

	#  .data offset & size
	ADDL	$0x20, %ecx		# sizeof(Shdr) minus 4 bytes 
	ADDL	%eax, (%ecx)		# Write .data load addr in section hdr
	ADDL	$4, %ecx
	ADDL	%eax, (%ecx)		# Write .data offset in section hdr
	MOVL	-44(%ebp), %eax		# .data size
	ADDL	$4, %ecx
	ADDL	%eax, (%ecx)		# Write .data size in section hdr

	#  .shstrtab section header needs link to table
	MOVL	-12(%ebp), %ecx		# .text sh_size
	ADDL	$0x74, %ecx		# for ELF
	ADDL	$0x88, %ecx		# offset field in third section header
	MOVL	%ecx, %eax
	ADDL	%ebx, %ecx
	ADDL	$0x18, %eax		# .shstrtab data
	MOVL	%eax, (%ecx)	

	#  The string literal "_start"
	MOVL	$0x7472, %eax		# 'rt'
	PUSH	%eax	
	MOVL	$0x6174735F, %eax	# '_sta'
	PUSH	%eax

	#  Find _start
	LEA	-28(%ebp), %ecx		# syms
	PUSH	%ecx
	MOVL	%esp, %eax		# }
	ADDL	$4, %eax		# }     LEA	4(%esp), %eax
	PUSH	%eax
	MOVL	$7, %eax		# strlen("_start") + 1 /* for NUL */
	PUSH	%eax
	CALL	findsym
	ADDL	$0x14, %esp		# POP x5

	#  Write the entry point address
	MOVL	12(%eax), %eax		# sym->value
	ADDL	$0x74, %eax		# for ELF
	ADDL	$0x08048000, %eax	# Load address 0x08048000
	MOVL	%eax, 0x18(%ebx)	# e_entry

	#  Iterate over the relocation table
	LEA	-28(%ebp), %ecx		# syms
	PUSH	%ecx
	MOVL	-40(%ebp), %edi		# relocs.start
.L21:
	CMPL	-36(%ebp), %edi		# relocs.end
	JGE	.L22			# No more relocations

	#  Find the symbol value
	PUSH	%edi			# rel->name
	CALL	strlen
	PUSH	%eax
	CALL	findsym		
	POP	%ecx
	POP	%ecx
	PUSH	%eax

	#  Do the relocation
	MOVL	12(%edi), %ecx		# rel->value (address to be reloc'd)
	MOVL	%ecx, %edx
	ADDL	$0x74, %ecx		# for ELF
	CMPL	$0, 16(%edi)		# Is the reloc in the .data section?
	JE	.L21b
	ADDL	-48(%ebp), %ecx		# Yes... so add .text size

.L21b:
	#  Add the symbol value  (+S per Fig 4.4 in ABI386 v4)
	ADDL	%ebx, %ecx
	MOVL	12(%eax), %eax		# sym->value
	ADDL	%eax, (%ecx)
	ADDL	$0x08048074, (%ecx)	# .text load address
	POP	%eax			# restore sym
	CMPL	$0, 16(%eax)		# Is the symbol in the .data section?
	JE	.L21c
	MOVL	-48(%ebp), %eax		# + .text size
	ADDL	%eax, (%ecx)
	ADDL	$0x1000, (%ecx)		# extra page for .data (0x08049074)

.L21c:
	#  If it's a PC-relative relocation, subtract the PC (%edx)
	CMPL	$2, 20(%edi)		# rel->type
	JNE	.L21a
	SUBL	%edx, (%ecx)		# -P per Fig 4.4 in ABI386 v4
	SUBL	$0x08048074, (%ecx)	# .text load address

.L21a:
	ADDL	$24, %edi		# ++rel; sizeof(label)
	JMP	.L21
.L22:
	POP	%ecx			# syms

	#  Unmap and close the output
	MOVL	-12(%ebp), %ecx		# .text written
	ADDL	$0x74, %ecx		# Add ELF, etc.
	ADDL	$0x89, %ecx		# Section hdrs, etc.
	PUSH	%ecx			# size
	MOVL	$91, %eax		# __NR_munmap == 91
	INT	$0x80
	MOVL	-16(%ebp), %ebx		# fd_out
	MOVL	$6, %eax		# __NR_close == 6
	INT	$0x80

	XORL	%ebx, %ebx
	JMP	success


####    #  And finally, the entry point.
	#  Last per requirement for elfify.
	JMP	_start 
