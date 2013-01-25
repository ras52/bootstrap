# ld.s

# Copyright (C) 2011, 2012, 2013 Richard Smith <richard@ex-parrot.com>
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
.hex	20 00 02 00  28 00 06 00    03 00 

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
.hex	05 00 00 00  01 00 00 00    06 00 00 00  74 80 04 08
#	offset----\  **size**--\    /-- Fig 4.12 in gABI --\
.hex	74 00 00 00  00 00 00 00    00 00 00 00  00 00 00 00
#	align-----\  entry-sz--\
.hex	04 00 00 00  00 00 00 00

# #2  Data section header.   start: +0x50; length: 0x28
# (Offset +0x60, +0x64 requires data section offset and size.)
#
#	name-str--\  PROGBITS--\    WRITE|ALLOC\ **load**--\
.hex	0F 00 00 00  01 00 00 00    03 00 00 00  74 90 04 08
#	**offset**\  **size**--\    /-- Fig 4.12 in gABI --\
.hex	74 00 00 00  00 00 00 00    00 00 00 00  00 00 00 00
#	align-----\  entry-sz--\
.hex	04 00 00 00  00 00 00 00

# #3  Shstrtab section hdr.  start: +0x78; length: 0x28
# (Offset +0x88 requires offset to shstrtab.)
#
#	name-str--\  STRTAB----\    no-flags--\  load-addr-\ 
.hex	15 00 00 00  03 00 00 00    00 00 00 00  00 00 00 00
#	**offset**\  size------\    /-- Fig 4.12 in gABI --\
.hex	00 00 00 00  2F 00 00 00    00 00 00 00  00 00 00 00
#	align-----\  entry-sz--\
.hex	01 00 00 00  00 00 00 00

# #4  Symbol table.          start: 0xA0; length: 0x28
# (Offsets 0xB0, 0xB4 need setting) Y Y
#
#	name-str--\  SYMTAB----\    no-flags--\  load-addr-\ 
.hex	1F 00 00 00  02 00 00 00    00 00 00 00  00 00 00 00
#	**offset**\  **size**--\    .strtab---\  **LOCAL**-\ 
.hex	00 00 00 00  00 00 00 00    05 00 00 00  00 00 00 00
#	align-----\  entry-sz--\
.hex	04 00 00 00  10 00 00 00

# #5  String table.          start: 0xC8; length: 0x28
# (Offsets 0xD8, 0xDC need setting) Y Y
#
#	name-str--\  STRTAB----\    no-flags--\  load-addr-\
.hex	27 00 00 00  03 00 00 00    00 00 00 00  00 00 00 00
#	**offset**\  **size**--\    /-- Fig 4.12 in gABI --\
.hex	00 00 00 00  00 00 00 00    00 00 00 00  00 00 00 00
#	align-----\  entry-sz--\
.hex	01 00 00 00  00 00 00 00

# end of sections (exc .rel sections) 0xF0

# #6 Text relocations.      start: 0xF0; length: 0x28
# (Offsets 0x100, 0x104 need setting)
#
#	name-str--\  SHT_REL---\    no-flags--\  load-addr-\
.hex	01 00 00 00  09 00 00 00    00 00 00 00  00 00 00 00
#	**offset**\  **size**--\    .symtab---\  .text-----\
.hex	00 00 00 00  00 00 00 00    04 00 00 00  01 00 00 00
#	align-----\  entry-sz--\
.hex	04 00 00 00  08 00 00 00

# #7 Text relocations.      start: 0x118; length: 0x28
# (Offsets 0x128, 0x12C need setting)
#
#	name-str--\  SHT_REL---\    no-flags--\  load-addr-\
.hex	0B 00 00 00  09 00 00 00    00 00 00 00  00 00 00 00
#	**offset**\  **size**--\    .symtab---\  .text-----\
.hex	00 00 00 00  00 00 00 00    04 00 00 00  02 00 00 00
#	align-----\  entry-sz--\
.hex	04 00 00 00  08 00 00 00

# end of sections inc .rel.XXX sections 0x140 (or 0xF0 without .rel.XXX sects)

shstrtab:
# The shared string table itself.  start +0xF0; length: 0x30
#
.hex	00                             # NULL.      Offset 0x00
.hex	2E 72 65 6C 2E 74 65 78 74 00  # .rel.text  Offset 0x01 0x05
.hex	2E 72 65 6C 2E 64 61 74 61 00  # .rel.data  Offset 0x0B 0x0F
.hex	2E 73 68 73 74 72 74 61 62 00  # .shstrtab  Offset 0x15
.hex	2E 73 79 6D 74 61 62 00        # .symtab    Offset 0x1F
.hex	2E 73 74 72 74 61 62 00        # .strtab    Offset 0x27
.hex	00			       # padding to 4-bit boundary

# End of end headers.   offset 0x120 (0x170)


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


####	#  Function: void* memset(void* s, int c, size_t n);
	#  Set N bytes of memory pointed to by S to the C byte
memset:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%edi

	MOVL	8(%ebp), %edi
	MOVB	12(%ebp), %al
	MOVL	16(%ebp), %ecx
	REP STOSB
	MOVL	8(%ebp), %eax

	POP	%edi
	POP	%ebp
	RET


####	#  Function:	void strcpy12(char* dest, char const* src);
	#  Copy at most 12 bytes (incl. null termination) from SRC to DEST.
strcpy12:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%edi
	PUSH	%esi

	PUSH	12(%ebp)
	CALL	strlen
	POP	%esi
	CMPL	$12, %eax
	JGE	error

	MOVL	%eax, %ecx
	INCL	%ecx			# +1 for NUL char
	MOVL	8(%ebp), %edi
	REP MOVSB

	POP	%esi
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


####	#  Function: void free(void* ptr)
	#  Returns memory allocated with malloc or realloc
free:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%ebx

	MOVL	8(%ebp), %ebx		# ptr
	SUBL	$4, %ebx
	MOVL	(%ebx), %ecx		# old size
	MOVL	$91, %eax		# 91 == __NR_munmap
	INT	$0x80
	CMPL	$-4096, %eax		# -4095 <= %eax < 0 for errno
	JA	error			# unsigned comparison handles above

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
	PUSH	%edx

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
	#  Return 1 if the strings are equal in N bytes, and 0 otherwise
strneq:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%esi
	PUSH	%edi

	XORL	%eax, %eax
	MOVL	8(%ebp), %edi
	MOVL	12(%ebp), %esi
	MOVL	16(%ebp), %ecx
	REPE CMPSB
	JNE	.L3
	TESTL	%ecx, %ecx
	JNZ	.L3
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
	JGE	.L1a

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

.L1a:
	#  Not found
	XORL	%edi, %edi
	XORL	%esi, %esi
	DECL	%esi
#	JMP	.L2a

.L2:
	#  Write the pointer to PTR
	MOVL	12(%ebp), %eax
	CMPL	$0, %eax
	JE	.L2a
	MOVL	%edi, (%eax)

.L2a:
	#  Return the section number
	MOVL	%esi, %eax

	ADDL	$0x18, %esp		# POP x6

	POP	%esi
	POP	%edi
	POP	%ebx
	POP	%ebp
	RET


####	#  Function:  void growvec( vec<label>* v );
	#  Check there is at least one space in the given vector
growvec:
	PUSH	%ebp
	MOVL	%esp, %ebp

	MOVL	8(%ebp), %edx
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
	MOVL	8(%ebp), %edx
	PUSH	(%edx)			# vec.start
	CALL	realloc

	#  Store pointers
	MOVL	8(%ebp), %edx
	MOVL	%eax, (%edx)		# store new pointer
	POP	%ecx
	SUBL	%ecx, 4(%edx)
	ADDL	%eax, 4(%edx)		# new vec->end
	POP	%ecx
	ADDL	%eax, %ecx
	MOVL	%ecx, 8(%edx)		# new vec->end_store

.L13:
	POP	%ebp
	RET


####	#  Function:  void* readsyms( elffile* file, size_t* offsets, 
	#                             int unused, int unused,
	#                             vec<label>* syms, vec<label>* relocs,
	#                             int unused )
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

	PUSH	%eax			# Unused return value -16(%ebp) **
	PUSH	%eax			# Unused              -20(%ebp) **

	#  Set pointers to .symtab header => %ebx, .symtab section => %edi,
	#  and .strtab section => %esi
	MOVL	8(%ebp), %ecx
	MOVL	28(%ecx), %ebx		# file->symhdr in %ebx
	MOVL	0x10(%ebx), %edi	# .symtab sh_offset
	ADDL	4(%ecx), %edi		# += file->ptr
	PUSH	%edi			#                     -24(%ebp) **
	MOVL	32(%ecx), %eax		# file->strhdr in %eax
	MOVL	0x10(%eax), %esi	# .strtab sh_offset
	ADDL	4(%ecx), %esi		# += file->ptr

	#  Allocate memory for a vector to store a mapping from object file
	#  symbol number to in-memory symbol number.
	MOVL	0x14(%ebx), %eax	# sh_size
	XORL	%edx, %edx
	MOVL	0x24(%ebx), %ecx	# /= sh_entsize
	DIVL	%ecx			# acts on %edx:%eax
	MOVB	$2, %cl
	SHLL	%eax			# *= 4 (sizeof(int))
	PUSH	%eax
	CALL	malloc

	#  Initialise the array to (int)(-1); size still on stack
	XORL	%ecx, %ecx
	DECL	%ecx
	PUSH	%ecx
	PUSH	%eax
	CALL	memset
	ADDL	$12, %esp		# POP x3
	PUSH	%eax			# -28(%ebp) **

	#  Loop over symbol table -- %edi already set
.L10:
	#  Check whether we're overrunning the object file's symbol table
	MOVL	0x10(%ebx), %ecx	# offset of start of section
	ADDL	0x14(%ebx), %ecx	# + size of section
	MOVL	8(%ebp), %eax
	ADDL	4(%eax), %ecx		# => ptr to end of section
	CMPL	%ecx, %edi
	JGE	.L11			# Done

	#  Check the symbol is in the correct section
	#  NB  If the symbol is undefined, the section will be SHN_UNDEF==0
	MOVL	0xC(%edi), %eax
	MOVB	$16, %cl
	SHRL	%eax 			# %eax is now st_shndx
	XORL	%edx, %edx		# sectid 0
	MOVL	8(%ebp), %ecx
	CMPL	36(%ecx), %eax		# Is it in data?
	JE	.L12a
	INCL	%edx			# sectid 1
	CMPL	40(%ecx), %eax
	JE	.L12a
	JMP	.L12			# continue

.L12a:	# We're in .text or .data, and %edx sectid
	PUSH	%edx

	#  Check we're not about to overrun the symbol vector
	PUSH	24(%ebp)
	CALL	growvec
	POP	%eax

	#  Store the string
	MOVL	%esi, %ecx
	ADDL	(%edi), %ecx		# source = string table + st_name 
	PUSH	%ecx
	MOVL	24(%ebp), %eax
	PUSH	4(%eax)			# dest = vec.end
	CALL	strcpy12
	ADDL	$8, %esp		# POP x2

	#  Store the section id in label->sect.
	MOVL	24(%ebp), %edx		# vec
	MOVL	4(%edx), %eax		# dest = vec->end
	POP	%edx
	MOVL	%edx, 16(%eax)

	#  Calculate file offset: offsets[sectid]
	MOVL	12(%ebp), %eax
	MOVB	$2, %cl
	SHLL	%edx
	ADDL	%edx, %eax

	#  Copy the symbol address
	MOVL	0x4(%edi), %ecx		# st_value
	ADDL	(%eax), %ecx		# + file offset
	MOVL	24(%ebp), %edx		# vec
	MOVL	4(%edx), %eax		# dest = vec->end
	MOVL	%ecx, 12(%eax)

	#  Store st_info in label->type
	XORL	%ecx, %ecx
	MOVB	0xC(%edi), %cl		# st_info
	MOVL	%ecx, 20(%eax)

	#  Fill in our temporary array.
	MOVL	%edi, %eax		# ptr to current symbol
	SUBL	0x10(%ebx), %eax	# -= offset of start of sectiona
	MOVL	8(%ebp), %ecx
	SUBL	4(%ecx), %eax		# -= file->ptr
	XORL	%edx, %edx
	MOVL	0x24(%ebx), %ecx	# /= sh_entsize
	DIVL	%ecx			# %eax is now the .o symbol number
	MOVB	$2, %cl
	SHLL	%eax			# dword offset into temporary array
	ADDL	-28(%ebp), %eax		# offset into array

	MOVL	24(%ebp), %edx		# vec
	MOVL	4(%edx), %ecx		# vec->end
	SUBL	(%edx), %ecx		# - vec->start => byte offset 
	MOVL	%ecx, (%eax)

	ADDL	$28, 4(%edx)		# ++vec->end;  sizeof(label)
.L12:
	ADDL	0x24(%ebx), %edi	# Add sh_entsize
	JMP	.L10

.L11:
	PUSH	%ebx			# -32(%ebp) == ptr to .symtab **

	#  Loop over the two sections { .text, .data }
	XORL	%eax, %eax
	PUSH	%eax			# -36(%ebp) == sectid
.L18a:
	#  Find the .relNAME section
	MOVL	8(%ebp), %eax
	MOVL	-36(%ebp), %edx
	MOVB	$2, %cl
	SHLL	%edx
	ADDL	%edx, %eax
	MOVL	20(%eax), %ebx		# file->relshdr[sectid]  .relNAME
	TESTL	%ebx, %ebx
	JZ	.L19			# It's okay if .relNAME is missing

	#  Find pointer to .relNAME section
	MOVL	0x10(%ebx), %edi
	MOVL	8(%ebp), %eax
	ADDL	4(%eax), %edi		# file->ptr

	#  Loop over relocation table
.L18:
	MOVL	0x10(%ebx), %ecx	# offset of start of section
	ADDL	0x14(%ebx), %ecx	# + size of section
	MOVL	8(%ebp), %eax
	ADDL	4(%eax), %ecx		# => ptr to end of section
	CMPL	%ecx, %edi
	JGE	.L19			# Done

	#  Check we're not about to overrun the relocation vector
	PUSH	28(%ebp)
	CALL	growvec
	POP	%eax

	#  (%edi) is the r_offset; 4(%edi) is the r_info field
	MOVL	4(%edi), %eax		# r_info
	MOVB	$8, %cl
	SHRL	%eax			# %eax is now r_sym from r_info
	PUSH	%eax			# save symbol number

	#  Use our temporary map to reference the actual symbol.
	MOVB	$2, %cl
	SHLL	%eax			# dword offset into temporary array
	ADDL	-28(%ebp), %eax		# offset into array
	MOVL	(%eax), %ecx
	MOVL	28(%ebp), %edx		# vec
	MOVL	4(%edx), %eax		# dest = vec->end
	MOVL	%ecx, 24(%eax)		# rel->strno

	#  Look up symbol in input object file
	POP	%eax			# restore symbol number
	XORL	%edx, %edx
	MOVL	$16, %ecx
	MULL	%ecx			# %eax is now offset into .symtab
	MOVL	-32(%ebp), %edx		# ptr to .symtab header
	ADDL	16(%edx), %eax		# offset to .symtab section
	MOVL	8(%ebp), %ecx
	ADDL	4(%ecx), %eax		# += file->ptr  => %eax is now a symbol 

	#  Store the string
	MOVL	%esi, %ecx
	ADDL	(%eax), %ecx		# source = string table + st_name 
	PUSH	%ecx
	MOVL	28(%ebp), %eax
	PUSH	4(%eax)			# dest = vec.end
	CALL	strcpy12
	ADDL	$8, %esp		# POP x2

	#  Calculate file offset: offsets[sectid]
	MOVL	12(%ebp), %eax
	MOVL	-36(%ebp), %edx
	MOVB	$2, %cl
	SHLL	%edx
	ADDL	%edx, %eax

	#  Store relocation offset
	MOVL	(%edi), %ecx		# r_offset
	ADDL	(%eax), %ecx		# + file offset
	MOVL	28(%ebp), %edx		# vec
	MOVL	4(%edx), %eax		# dest = vec->end
	MOVL	%ecx, 12(%eax)		# rel->type
	MOVL	-36(%ebp), %ecx		# sectid
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

	ADDL	$28, 4(%edx)		# ++vec->end;  sizeof(label)

	ADDL	0x24(%ebx), %edi	# Add sh_entsize
	JMP	.L18

.L19:	# Do we need to loop back to do the next .rel.XXX section
	POP	%eax			# sectid, -36(%ebp), above **
	TESTL	%eax, %eax
	JNZ	.L19a
	INCL	%eax
	PUSH	%eax
	JMP	.L18a

.L19a:
	POP	%eax			# -32(%ebp), above **
	CALL	free			# free array in -28(%ebp)
	ADDL	$12, %esp		# POP x3: -28(%ebp) ... -20(%ebp)
	POP	%eax			# return pointer to NAME header **

	POP	%esi
	POP	%edi
	POP	%ebx
	POP	%ebp
	RET


####	#  Function:  void doreadsyms( elffile* file, size_t* offsets, 
	#                              vec<label>* syms, vec<label>* relocs );
doreadsyms:
	PUSH	%ebp
	MOVL	%esp, %ebp

	#  Read the symbols
	PUSH	20(%ebp)		# relocs
	PUSH	16(%ebp)		# syms
	PUSH	%ecx			# (unused)
	PUSH	%ecx			# (unused)
	MOVL	12(%ebp), %edx
	PUSH	%edx			# offsets
	PUSH	8(%ebp)			# file
	CALL	readsyms
	ADDL	$24, %esp		# POP x7

	#  Calculate the size of the section incl. padding
	MOVL	8(%ebp), %edx
	MOVL	12(%edx), %eax		# file->shdr[0]
	MOVL	0x14(%eax), %ecx	# sh_size member
	PUSH	%ecx
	CALL	calcpad4
	POP	%ecx
	ADDL	%eax, %ecx
	MOVL	12(%ebp), %edx
	ADDL	%ecx, (%edx)		# offsets[0]

	#  Calculate the size of the section incl. padding
	MOVL	8(%ebp), %edx
	MOVL	16(%edx), %eax		# &file->shdr[1]
	MOVL	0x14(%eax), %ecx	# sh_size member
	PUSH	%ecx
	CALL	calcpad4
	POP	%ecx
	ADDL	%eax, %ecx
	MOVL	12(%ebp), %edx
	ADDL	%ecx, 4(%edx)		# offsets[1]

	POP	%ebp
	RET


####	#  Function:  int calcpad4( int written );
	#  Returns the number of padding bytes needed to make WRITTEN 
	#  4-byte aligned.
calcpad4:
	PUSH	%ebp
	MOVL	%esp, %ebp

	XORL	%edx, %edx
	MOVL	8(%ebp), %eax		# bytes written
	MOVL	$4, %ecx
	DIVL	%ecx			# acts on %edx:%eax
	XORL	%eax, %eax		# ready for return
	SUBL	%edx, %ecx		# 4 - (bytes % 4)
	CMPL	$4, %ecx
	JE	.L23a
	MOVL	%ecx, %eax
.L23a:
	POP	%ebp
	RET


####	#  Function:  int writepad4( int written, int pad_chars, int fd_out )
	#  Write bytes from PAD_CHARS until WRITTEN is 4-byte aligned, and
	#  return the number of padding bytes written.
writepad4:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%ebx

	PUSH	8(%ebp)
	CALL	calcpad4
	POP	%ecx
	TESTL	%eax, %eax
	JZ	.L23

	#  And write the padding
	PUSH	%eax			# count
	PUSH	12(%ebp)		# pad_chars
	MOVL	%eax, %edx
	MOVL	%esp, %ecx
	MOVL	16(%ebp), %ebx		# fd
	MOVL	$4, %eax   		# 4 == __NR_write
	INT	$0x80
	POP	%ecx
	POP	%ecx
	CMPL	%ecx, %eax
	JNE	error

.L23:
	POP	%ebx
	POP	%ebp
	RET


####	#  Function:  void writestr( char* str, int fd_out )
	#  Write out null-terminated string, STR
writestr:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%ebx

	PUSH	8(%ebp)
	CALL	strlen
	POP	%ecx
	INCL	%eax
	PUSH	%eax
	MOVL	%eax, %edx		# len
	MOVL	8(%ebp), %ecx
	MOVL	12(%ebp), %ebx		# fd_out
	MOVL	$4, %eax		# 4 == __NR_write
	INT	$0x80
	POP	%ecx
	CMPL	%ecx, %eax
	JNE	error

	POP	%ebx
	POP	%ebp
	RET

####	#  Function:  void writedword( int dw, int fd_out )
	#  Write out dword, DW
writedword:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%ebx

	MOVL	$4, %edx
	LEA	8(%ebp), %ecx		# &dw
	MOVL	12(%ebp), %ebx		# fd_out
	MOVL	$4, %eax		# 4 == __NR_write
	INT	$0x80
	CMPL	$4, %eax
	JNE	error

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

	MOVL	8(%ebp), %eax		# strlen
	INCL	%eax			# add one to compare '\0#
	PUSH	%eax
	PUSH	12(%ebp)		# name

	#  Scan the labels looking for NAME  
	MOVL	16(%ebp), %edx
	MOVL	(%edx), %edi		# syms[0].start
.L14:
	MOVL	16(%ebp), %edx
	CMPL	4(%edx), %edi		# sym->end
	JL	.L14a

	#  Reached end of vector without finding symbol.  Report error.
	MOVL	$0x203A646C, %eax	# 'ld: '
	PUSH	%eax
	MOVL	$2, %ebx		# STDERR_FILENO
	MOVL	%esp, %ecx
	MOVL	$4, %edx
	MOVL	$4, %eax   		# 4 == __NR_write
	INT	$0x80
	MOVL	$2, %ebx		# STDERR_FILENO
	MOVL	12(%ebp), %ecx		# name
	MOVL	8(%ebp), %edx		# strlen
	MOVL	$4, %eax   		# 4 == __NR_write
	INT	$0x80
	POP	%eax
	MOVL	$0x0A, %eax
	PUSH	%eax
	MOVL	$2, %ebx		# STDERR_FILENO
	MOVL	%esp, %ecx
	MOVL	$1, %edx
	MOVL	$4, %eax   		# 4 == __NR_write
	INT	$0x80
	POP	%eax
	JMP	error			# Not found

.L14a:
	#  Does it have the right name?
	PUSH	%edi
	CALL	strneq
	POP	%ecx
	CMPL	$0, %eax
	JE	.L14b

	#  Is it something other than STB_LOCAL (which has value 0)?
	MOVL	20(%edi), %eax		# sym->type  (from st_info)
	ANDB	$0xF0, %al		# Just look at the ST_BIND bits
	CMPB	$0, %al
	JE	.L14b

	JMP	.L15			# Found it
.L14b:
	ADDL	$28, %edi		# sizeof(label)
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


####	#  Function:  void openelf( char* filename, elffile* file );
	#  where elffile is an { int fd; void* ptr; size_t sz; void* shdr[2];
	#                        void* relshdr[2]; void* symhdr; void* strhdr;
	#			 ind sidx[2]; }
openelf:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%ebx

	#  Open the input file
	XORL	%ecx, %ecx		# 0 == O_RDONLY
	MOVL	8(%ebp), %ebx		# filename
	MOVL	$5, %eax		# 5 == __NR_open
	INT	$0x80
	CMPL	$0, %eax
	JL	error
	MOVL	12(%ebp), %edx
	MOVL	%eax, (%edx) 		# Set file->fd

	#  Find the size of the object file
	SUBL	$0x58, %esp		# sizeof(struct stat) per [new] fstat
	MOVL	%esp, %ecx
	MOVL	(%edx), %ebx		# file->fd
	MOVL	$108, %eax		# 108 == __NR_fstat
	INT	$0x80
	CMPL	$0, %eax
	JL	error
	ADDL	$0x14, %esp		# move to file size member }
	POP	%eax			#                          } ADDL $0x58,
	ADDL	$0x40, %esp		# $0x58 - 0x14 - 4         }   %esp
	MOVL	12(%ebp), %edx
	MOVL	%eax, 8(%edx) 		# Set file->sz

	#  Map the input object file
	XORL	%eax, %eax
	PUSH	%eax			# 0 offset
	PUSH	(%edx)			# file->fd
	MOVL	$2, %ecx
	PUSH	%ecx			# MAP_PRIVATE
	MOVL	$1, %ecx
	PUSH	%ecx			# PROT_READ
	PUSH	8(%edx)			# file->sz
	PUSH	%eax			# NULL
	MOVL	%esp, %ebx
	MOVL	$90, %eax		# 90 == __NR_mmap
	INT	$0x80
	CMPL	$-4096, %eax		# -4095 <= %eax < 0 for errno
	JA	error			# unsigned comparison handles above
	ADDL	$0x18, %esp		# 6x POP
	MOVL	12(%ebp), %edx
	MOVL	%eax, 4(%edx)		# Set file->ptr

	#  Locate the .text section header
	MOVL	$0x74, %eax		# 't'
	PUSH	%eax
	MOVL	$0x7865742E, %eax	# '.tex'
	PUSH	%eax
	PUSH	%esp
	MOVL	$5, %eax		# strlen(".text")
	PUSH	%eax
	LEA	12(%edx), %eax
	PUSH	%eax			# &file->shdr[0]
	PUSH	4(%edx)			# file->ptr
	CALL	findsect
	ADDL	$16, %esp		# POP x4
	MOVL	12(%ebp), %edx
	MOVL	%eax, 36(%edx)		# file->sidx[0]

	#  Locate the .rel.text section header
	MOVL	$0x6C65722E, %eax	# '.rel'
	PUSH	%eax
	PUSH	%esp
	MOVL	$9, %eax		# strlen(".rel.text")
	PUSH	%eax
	LEA	20(%edx), %eax
	PUSH	%eax			# &file->relshdr[0]
	PUSH	4(%edx)			# file->ptr
	CALL	findsect
	ADDL	$28, %esp		# POP x7

	#  Locate the .data section header
	MOVL	$0x61, %eax		# 'a'
	PUSH	%eax
	MOVL	$0x7461642E, %eax	# '.dat'
	PUSH	%eax
	PUSH	%esp
	MOVL	$5, %eax		# strlen(".data")
	PUSH	%eax
	MOVL	12(%ebp), %edx
	LEA	16(%edx), %eax
	PUSH	%eax			# &file->shdr[1]
	PUSH	4(%edx)			# file->ptr
	CALL	findsect
	ADDL	$16, %esp		# POP x4
	MOVL	12(%ebp), %edx
	MOVL	%eax, 40(%edx)		# file->sidx[1]

	#  Locate the .rel.data section header
	MOVL	$0x6C65722E, %eax	# '.rel'
	PUSH	%eax
	PUSH	%esp
	MOVL	$9, %eax		# strlen(".rel.data")
	PUSH	%eax
	LEA	24(%edx), %eax
	PUSH	%eax			# &file->relshdr[1]
	PUSH	4(%edx)			# file->ptr
	CALL	findsect
	ADDL	$28, %esp		# POP x7

	#  Locate the .symtab section header
	MOVL	$0x626174, %eax		# 'tab'
	PUSH	%eax
	MOVL	$0x6D79732E, %eax	# '.sym'
	PUSH	%eax
	PUSH	%esp			# ptr to string
	MOVL	$7, %eax		# strlen(".symtab")
	PUSH	%eax
	MOVL	12(%ebp), %edx
	LEA	28(%edx), %eax
	PUSH	%eax			# file->symhdr
	PUSH	4(%edx)			# file->ptr
	CALL	findsect
	ADDL	$20, %esp		# POP x5

	#  Locate the .strtab section header
	MOVL	$0x7274732E, %eax	# '.str'
	PUSH	%eax
	PUSH	%esp			# ptr to string
	MOVL	$7, %eax		# strlen(".symtab")
	PUSH	%eax
	MOVL	12(%ebp), %edx
	LEA	32(%edx), %eax
	PUSH	%eax			# file->strhdr
	PUSH	4(%edx)			# file->ptr
	CALL	findsect
	ADDL	$24, %esp		# POP x6

	POP	%ebx
	POP	%ebp
	RET
	

####	#  Function:  void closeelf( elffile* file );
closeelf:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%ebx

	#  Unmap and close the input file
	MOVL	8(%ebp), %edx
	MOVL	8(%edx), %ecx		# file->sz
	MOVL	4(%edx), %ebx		# file->ptr
	MOVL	$91, %eax		# 91 == __NR_munmap
	INT	$0x80

	MOVL	8(%ebp), %edx
	MOVL	(%edx), %ebx		# file->fd
	MOVL	$6, %eax		# 6 == __NR_close
	INT	$0x80

	POP	%ebx
	POP	%ebp
	RET


####	#  Function: void writesect( int sectid, int pad_char, int nfiles,
	#                            elffile* files, int fd_out );
writesect:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%ebx

	#  Loop over input files
	XORL	%eax, %eax
	PUSH	%eax			# loop variable	-8(%ebp)
.L16:
	MOVL	-8(%ebp), %eax
	CMPL	16(%ebp), %eax
	JGE	.L17

	#  Find elffiles[argn]
	MOVL	$44, %ecx		# %eax *= sizeof(elffile)
	MULL	%ecx
	ADDL	20(%ebp), %eax		# += &elffiles[0]  =>  %eax
	PUSH	%eax			# -12(%ebp)

	#  Locate the correct header
	MOVL	8(%ebp), %edx
	MOVB	$2, %cl
	SHLL	%edx			# 4*sectid
	ADDL	%eax, %edx		# + file
	MOVL	12(%edx), %eax		# &file->shdr[sectid]  => %eax

	#  Write the section
	MOVL	0x14(%eax), %edx	# sh_size member	=> size
	MOVL	0x10(%eax), %ecx	# sh_offset member
	POP	%eax			# retrieve file
	PUSH	%edx			# save size
	ADDL	4(%eax), %ecx		# + elf	(file->ptr)	=> ptr
	MOVL	24(%ebp), %ebx		# fd_out		=> fd
	MOVL	$4, %eax   		# 4 == __NR_write
	INT	$0x80
	POP	%ecx
	CMPL	%ecx, %eax
	JNE	error

	#  And pad it
	PUSH	24(%ebp)		# fd_out
	PUSH	12(%ebp)		# padding
	PUSH	%eax
	CALL	writepad4
	ADDL	$12, %esp		# POP x3

	INCL	-8(%ebp)
	JMP	.L16
.L17:
	POP	%eax
	POP	%ebx
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
	#      -52(%ebp)	int32 shdr_size   # -r ? 0x140 : 0xF0
	#      -56(%ebp)	int32 shdr_plus_shstr_size # -r ? 0x170 : 0x120
	#      -60(%ebp)	elffile* files
	#
	#  ... later on, 
	#      -60(%ebp)	int32 strtabsz
	# 
	#  where vector<T> is a { T* start, end, end_store; },
	#  and label is a { char name[12]; int32 val, sect, type, strno; }.
	#  In the SYMS vector, type is the st_info (i.e. binding and type) of 
	#  the symbol; in the RELOCS vector, type is the relocation type.
	#  In the SYMS vector, strno is set to the string offset in the output
	#  file; in the RELOCS vector, strno is the byte offset into SYMS for
	#  the symbol.

	#  --- The main loop
_start:
	MOVL	%esp, %ebp
	SUBL	$60, %esp	

	#  Non-partial linking
	MOVL	$0xF0, -52(%ebp)	# sizeof shdr
	MOVL	$0x120, -56(%ebp)	# sizeof shdr + shstrtab

	#  Check we have at least three command line argument (-o out in.o ...)
	CMPL	$4, 0(%ebp)	# three args => argc == 4
	JL	error

	MOVL	$3, %eax		# argn of first input file
	PUSH	%eax			# -64(%ebp)

	#  Check argv[1] is '-o' or '-r'
	LEA	8(%ebp), %esi
	MOVL	(%esi), %eax
	CMPB	$0x2D, (%eax)		# '-'
	JNE	error
	CMPB	$0x6F, 1(%eax)		# 'o'
	JE	.L4a

	CMPB	$0x72, 1(%eax)		# 'r'
	JNE	error
	CMPB	$0x0, 2(%eax)		# '\0'
	JNE	error

	MOVL	$0x140, -52(%ebp)	# sizeof shdr
	MOVL	$0x170, -56(%ebp)	# sizeof shdr + shstrtab
	INCL	-64(%ebp)

	#  Check argv[2] is '-o'
	LEA	12(%ebp), %esi
	MOVL	(%esi), %eax
	CMPB	$0x2D, (%eax)		# '-'
	JNE	error
	CMPB	$0x6F, 1(%eax)		# 'o'
	JNE	error
.L4a:
	CMPB	$0x0, 2(%eax)		# '\0'
	JNE	error

	#  Open the output file
	MOVL	$0x1A4, %edx		# 0644 (we chmod later if necessary)
	MOVL	$0x242, %ecx		# O_RDWR=2|O_CREAT=0x40|O_TRUNC=0x200
	MOVL	4(%esi), %ebx		# argv[2]
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
	MOVL	$1792, %ecx		# 64 * sizeof(label)
	PUSH	%ecx
	CALL	malloc
	POP	%ecx
	MOVL	%eax, -28(%ebp)
	MOVL	%eax, -24(%ebp)
	ADDL	%ecx, %eax
	MOVL	%eax, -20(%ebp)

	#  Allocate the undef vector
	MOVL	$1792, %ecx		# 64 * sizeof(label)
	PUSH	%ecx
	CALL	malloc
	POP	%ecx
	MOVL	%eax, -40(%ebp)
	MOVL	%eax, -36(%ebp)
	ADDL	%ecx, %eax
	MOVL	%eax, -32(%ebp)

	#  Allocate array of elffiles for input files
	MOVL	0(%ebp), %eax
	SUBL	$3, %eax		# => number .o files
	MOVL	$44, %ecx		# sizeof(elffile)
	PUSH	%eax
	CALL	malloc
	POP	%ecx
	MOVL	%eax, -60(%ebp)

	#  Loop opening input files and reading symbols
	XORL	%eax, %eax
	MOVL	%eax, -48(%ebp)		# set write counter to zero
	PUSH	-64(%ebp)		# copy of the starting argn: -68(%ebp)
.L16a:
	POP	%eax
	CMPL	0(%ebp), %eax
	JGE	.L17a
	PUSH	%eax

	#  Find elffiles[argn]
	MOVL	-68(%ebp), %eax		# argn
	SUBL	-64(%ebp), %eax		# first argn
	MOVL	$44, %ecx		# *= sizeof(elffile)
	MULL	%ecx
	ADDL	-60(%ebp), %eax		# += &elffiles[0]
	MOVL	%eax, %edx
	PUSH	%edx

	#  Open the input file	
	MOVL	-68(%ebp), %eax
	INCL	%eax
	MOVL	$4, %ecx		# }  Stage 2/3 does not support
	MULL	%ecx			# }  PUSH (%ebp,%eax,4)
	ADDL	%ebp, %eax		# }
	PUSH	(%eax)			# } filename
	CALL	openelf
	POP	%eax

	#  Read the symbols
	POP	%edx
	LEA	-40(%ebp), %eax
	PUSH	%eax			# &relocs
	LEA	-28(%ebp), %eax
	PUSH	%eax			# &syms
	LEA	-48(%ebp), %eax
	PUSH	%eax			# &offsets[0]
	PUSH	%edx
	CALL	doreadsyms
	ADDL	$16, %esp		# POP x4

	INCL	-68(%ebp)
	JMP	.L16a			# End loop over input files
.L17a:
	MOVL	$0x74, %eax		# size of Elf header + PHdrs
	ADDL	-44(%ebp), %eax		# .data size
	ADDL	-48(%ebp), %eax		# .text size
	MOVL	%eax, -12(%ebp)

	#  Prepare to write the .text and .data sections
	PUSH	-16(%ebp)		# fd_out
	PUSH	-60(%ebp)		# &elffiles[0]
	MOVL	0(%ebp), %eax
	SUBL	-64(%ebp), %eax
	PUSH	%eax			# number of input files

	#  Write .text
	MOVL	$0x90909090, %eax	# 0x90 is NOP; pad .text with this
	PUSH	%eax
	XORL	%eax, %eax		# sectid for .text is 0
	PUSH	%eax
	CALL	writesect
	POP	%eax
	POP	%eax

	#  Write .data
	XORL	%eax, %eax		# Pad with '\0' in .data
	PUSH	%eax
	INCL	%eax			# sectid for .data is 1
	PUSH	%eax
	CALL	writesect
	ADDL	$20, %esp		# POP x5

	#  Loop closing input files
	PUSH	-64(%ebp)		# copy of the starting argn: -68(%ebp)
.L16c:
	POP	%eax
	CMPL	0(%ebp), %eax
	JGE	.L17c
	PUSH	%eax
	
	#  Find elffiles[argn]
	MOVL	-68(%ebp), %eax		# argn
	SUBL	-64(%ebp), %eax		# first argn
	MOVL	$44, %ecx		# *= sizeof(elffile)
	MULL	%ecx
	ADDL	-60(%ebp), %eax		# += &elffiles[0]

	PUSH	%eax
	CALL	closeelf
	POP	%eax

	INCL	-68(%ebp)
	JMP	.L16c
.L17c:
	POP	%eax			# remove -64(%ebp)
	CALL	free			# unallocate elffiles
	POP	%eax			# remove -60(%ebp)

	#  Locate the section headers we want to write
	#  CALL next_line; next_line: POP %eax  simply effects  MOV %eip, %eax
	CALL	.L5
.L5:
	POP	%ecx		# Currently assembled sub-optimally as 2 bytes
	ADDL	sect_hdr, %ecx	# Assembled as six bytes (op, modrm, imm32)
	ADDL	$8, %ecx	# len of prev two instr

	#  Write the section headers
	MOVL	-52(%ebp), %edx		# Length of headers
	MOVL	-16(%ebp), %ebx		# fd
	MOVL	$4, %eax		# 4 == __NR_write
	INT	$0x80
	CMPL	-52(%ebp), %eax
	JL	error

	#  And similarly for the .shstrtab
	CALL	.L5a
.L5a:
	POP	%ecx		# Currently assembled sub-optimally as 2 bytes
	ADDL	shstrtab, %ecx	# Assembled as six bytes (op, modrm, imm32)
	ADDL	$8, %ecx	# len of prev two instr

	#  Write the shstrtab
	MOVL	$0x30, %edx		# Length of table
	MOVL	-16(%ebp), %ebx		# fd
	MOVL	$4, %eax		# 4 == __NR_write
	INT	$0x80
	CMPL	$0x30, %eax
	JL	error

	#  Add the section headers & shstrtab to the output counter
	MOVL	-56(%ebp), %eax
	ADDL	%eax, -12(%ebp)

	#  Set up some new counters
	XORL	%eax, %eax
	PUSH	%eax			# -60(%ebp), for .strtab sz
	PUSH	%eax			# -64(%ebp), for .strtab sz
	PUSH	%eax			# -68(%ebp), for .rel.text sz
	PUSH	%eax			# -72(%ebp), fro .rel.data sz

	#  Write the string table, starting with a null character
	MOVL	%esp, %eax		# ptr to '\0'
	PUSH	-16(%ebp)		# fd_out  **
	PUSH	%eax
	CALL	writestr
	POP	%ecx
	ADDL	%eax, -60(%ebp)		# strtabsz += return of writestr()

	MOVL	-28(%ebp), %edi		# syms.start
	SUBL	$28, %edi		# start at labels - 1;  sizeof(label)
.L28:
	#  Loop: Generate the string table
	ADDL	$28, %edi		# sizeof(label)
	CMPL	-24(%ebp), %edi		# syms.end
	JGE	.L29			# finished

	#  Write string
	MOVL	-60(%ebp), %ecx
	MOVL	%ecx, 24(%edi)		# set label->strno
	PUSH	%edi
	CALL	writestr
	POP	%ecx
	ADDL	%eax, -60(%ebp)
	JMP	.L28

.L29:
	#  And pad to the dword boundary
	XORL	%eax, %eax
	PUSH	%eax			# pad with '\0'
	PUSH	-60(%ebp)
	CALL	writepad4
	POP	%ecx
	POP	%ecx
	ADDL	%eax, -60(%ebp)
	MOVL	-60(%ebp), %eax
	ADDL	%eax, -12(%ebp)


	#  Write Null symbol (per gABI-4.1, Fig 4-18)
	PUSH	%eax
	CALL	writedword
	CALL	writedword	
	CALL	writedword	
	CALL	writedword	
	POP	%eax
	ADDL	$16, -64(%ebp)

	#  Write .symtab section
	MOVL	-28(%ebp), %edi		# syms.start
	SUBL	$28, %edi		# start at labels - 1;  sizeof(label)
.L24:
	#  Loop: Generate the symbol table
	ADDL	$28, %edi		# sizeof(label)
	CMPL	-24(%ebp), %edi		# syms.end
	JGE	.L25			# finished

	MOVL	24(%edi), %eax		# label->strno
	PUSH	%eax
	CALL	writedword		# st_name
	POP	%eax

	#  Use 0 as the address of an undefined symbol
	MOVL	12(%edi), %eax		# offset from start of .text
	CMPL	$-1, %eax
	JNE	.L26
	XORL	%eax, %eax
	JMP	.L26a
.L26:
	CMPL	$0xF0, -52(%ebp)	# Are we partial linking?
	JNE	.L26a			# If yes, don't fix up offsets
	ADDL	$0x8048074, %eax
	CMPL	$0, 16(%edi)		# Is the symbol in the .data section?
	JE	.L26a
	ADDL	-48(%ebp), %eax		# Yes... so add .text size
	ADDL	$0x1000, %eax		# extra page for .data
.L26a:
	PUSH	%eax
	CALL	writedword		# st_value
	POP	%eax

	XORL	%eax, %eax		# 0 -- symbol size is not known
	PUSH	%eax
	CALL	writedword		# st_size
	POP	%eax

	MOVL	16(%edi), %eax		# label->section
	INCL	%eax			# .text is 1
	MOVB	$16, %cl
	SHLL	%eax			# shift into st_shndx
	MOVB	20(%edi), %al		# add sh_info
	MOVL	0xC(%edi), %ecx		# offset from start of .text
	CMPL	$-1, %ecx
	JNE	.L27
	MOVL	20(%edi), %eax		# add sh_info
.L27:
	PUSH	%eax
	CALL	writedword		# st_info, st_other, st_shndx
	POP	%eax

	ADDL	$16, -64(%ebp)
	JMP	.L24
.L25:
	MOVL	-64(%ebp), %eax
	ADDL	%eax, -12(%ebp)

	CMPL	$0xF0, -52(%ebp)	# Are we partial linking?
	JE	.L32

	#  Write .rel.text section
	MOVL	-40(%ebp), %edi		# relocs.start
	SUBL	$28, %edi		# start at labels - 1;  sizeof(label)
.L33:
	#  Loop: Generate the relocation table
	ADDL	$28, %edi		# sizeof(label)
	CMPL	-36(%ebp), %edi		# syms.end
	JGE	.L32a			# finished

	#  Continue unless it's a .text (sectid 0) relocation
	CMPL	$0, 16(%edi)		# rel->sect
	JNE	.L33

	#  Write r_offset
	PUSH	12(%edi)		# rel->val
	CALL	writedword
	POP	%eax

	#  Has the symbol already been resolved?
	MOVL	24(%edi), %eax		# rel->strno
	CMPL	$-1, %eax
	JNE	.L34

	#  No.  So look up the symbol.
	LEA	-28(%ebp), %ecx		# syms
	PUSH	%ecx
	PUSH	%edi			# rel->name
	CALL	strlen
	PUSH	%eax
	CALL	findsym
	ADDL	$12, %esp
	SUBL	-28(%ebp), %eax		# - syms.start => byte offset into syms
	MOVL	%eax, 24(%edi)		# save in rel->strno

.L34:
	#  Calculate and write the symbol offset.  rel->strno is in %eax
	XORL	%edx, %edx
	MOVL	$28, %ecx		# sizeof(label)
	DIVL	%ecx			# acts on %edx:%eax
	INCL	%eax			# for initial null symbol
	MOVB	$8, %cl
	SHLL	%eax			# <<= 8 (to construct r_info)
	ADDL	20(%edi), %eax		# += rel->type
	PUSH	%eax
	CALL	writedword
	POP	%eax

	ADDL	$8, -68(%ebp)		# .rel.text sz += sizeof(Elf_Rel)
	JMP	.L33			# end loop over .rel.text
	

.L32a:
	MOVL	-68(%ebp), %eax
	ADDL	%eax, -12(%ebp)

	#  Write .rel.data section
	MOVL	-40(%ebp), %edi		# relocs.start
	SUBL	$28, %edi		# start at labels - 1;  sizeof(label)
.L33a:
	#  Loop: Generate the relocation table
	ADDL	$28, %edi		# sizeof(label)
	CMPL	-36(%ebp), %edi		# syms.end
	JGE	.L32			# finished

	#  Continue unless it's a .data (sectid 1) relocation
	CMPL	$1, 16(%edi)		# rel->sect
	JNE	.L33a

	#  Write r_offset
	PUSH	12(%edi)		# rel->val
	CALL	writedword
	POP	%eax

	#  Has the symbol already been resolved?
	MOVL	24(%edi), %eax		# rel->strno
	CMPL	$-1, %eax
	JNE	.L34a

	#  No.  So look up the symbol.
	LEA	-28(%ebp), %ecx		# syms
	PUSH	%ecx
	PUSH	%edi			# rel->name
	CALL	strlen
	PUSH	%eax
	CALL	findsym
	ADDL	$12, %esp
	SUBL	-28(%ebp), %eax		# - syms.start => byte offset into syms
	MOVL	%eax, 24(%edi)		# save in rel->strno

.L34a:
	#  Calculate and write the symbol offset.  rel->strno is in %eax
	XORL	%edx, %edx
	MOVL	$28, %ecx		# sizeof(label)
	DIVL	%ecx			# acts on %edx:%eax
	INCL	%eax			# for initial null symbol
	MOVB	$8, %cl
	SHLL	%eax			# <<= 8 (to construct r_info)
	ADDL	20(%edi), %eax		# += rel->type
	PUSH	%eax
	CALL	writedword
	POP	%eax

	ADDL	$8, -72(%ebp)		# .rel.text sz += sizeof(Elf_Rel)
	JMP	.L33a			# end loop over .rel.text
	

.L32:
	MOVL	-72(%ebp), %eax
	ADDL	%eax, -12(%ebp)

	POP	%eax			# fd_out **
	
	#  The output file is completely written. 
	#  mmap it to fix up relocations
	XORL	%eax, %eax
	PUSH	%eax			# 0 offset
	PUSH	-16(%ebp)		# fd_out
	MOVL	$1, %ecx
	PUSH	%ecx			# MAP_SHARED
	MOVL	$3, %ecx
	PUSH	%ecx			# PROT_READ|PROT_WRITE
	PUSH	-12(%ebp)		# size
	XORL	%eax, %eax		# NULL 
	PUSH	%eax
	MOVL	%esp, %ebx
	MOVL	$90, %eax		# 90 == __NR_mmap
	INT	$0x80
	CMPL	$-4096, %eax		# -4095 <= %eax < 0 for errno
	JA	error			# unsigned comparison handles above
	MOVL	%eax, %ebx	

	CMPL	$0xF0, -52(%ebp)	# Are we partial linking?
	JE	.L25a
	MOVB	$1, 0x10(%ebx)		# .o
	MOVB	$8, 0x30(%ebx)		# shnum

.L25a:
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
	MOVL	$0x74, %edx		# size of ELF & program headers
	ADDL	-44(%ebp), %edx		# size of .text and .data together 
	ADDL	-48(%ebp), %edx		# size of .text and .data together 
	MOVL	%edx, 0x20(%ebx)	# ELF shdroff

	#  .text section header needs .text section size
	MOVL	%edx, %esi
	ADDL	%ebx, %esi
	MOVL	-48(%ebp), %eax		# .text size
	MOVL	%eax, 0x3C(%esi)	# size field in 2nd section header

	#  .data offset & size
	ADDL	%eax, 0x5C(%esi)	# Write .data load addr in section hdr
	ADDL	%eax, 0x60(%esi)	# Write .data offset in section hdr
	MOVL	-44(%ebp), %eax		# .data size
	ADDL	%eax, 0x64(%esi)	# Write .data size in section hdr

	#  .shstrtab, .strtab, .symtab section headers
	MOVL	%edx, %eax
	ADDL	-52(%ebp), %eax		# section hdrs size
	MOVL	%eax, 0x88(%esi)	# .shstrtab offset
	ADDL	-56(%ebp), %edx		# += sizeof of section headers etc

	MOVL	%edx, 0xD8(%esi)	# .strtab offset
	MOVL	-60(%ebp), %eax
	MOVL	%eax, 0xDC(%esi)	# .strtab size
	ADDL	%eax, %edx		# += size of .strtab section

	MOVL	%edx, 0xB0(%esi)
	MOVL	-64(%ebp), %eax		# .symtab size
	MOVL	%eax, 0xB4(%esi)
	ADDL	%eax, %edx		# += size of .symtab section

	CMPL	$0xF0, -52(%ebp)	# Are we partial linking?
	JE	.L30a

	#  .rel.text, .rel.data only if partial linking
	MOVL	%edx, 0x100(%esi)
	MOVL	-68(%ebp), %eax		# .rel.text size
	MOVL	%eax, 0x104(%esi)
	ADDL	%eax, %edx		# += size of .rel.text section

	MOVL	%edx, 0x128(%esi)
	MOVL	-72(%ebp), %eax		# .rel.data size
	MOVL	%eax, 0x12C(%esi)
	ADDL	%eax, %edx		# += size of .rel.data section
	JMP	.L30

.L30a:
	#  The string literal "_start", only if not partial linking
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

	#  Has the symbol already been resolved?
	MOVL	24(%edi), %eax		# rel->strno
	CMPL	$-1, %eax
	JNE	.L35

	#  No.  So look up the symbol.
	LEA	-28(%ebp), %ecx		# syms
	PUSH	%ecx
	PUSH	%edi			# rel->name
	CALL	strlen
	PUSH	%eax
	CALL	findsym
	ADDL	$12, %esp
	SUBL	-28(%ebp), %eax		# - syms.start => byte offset into syms
	MOVL	%eax, 24(%edi)		# save in rel->strno

.L35:
	ADDL	-28(%ebp), %eax		# - syms.start => byte offset into syms
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
	MOVL	-48(%ebp), %eax		# Yes... so add .text size
	ADDL	%eax, (%ecx)
	ADDL	$0x1000, (%ecx)		# extra page for .data (0x08049074)

.L21c:
	#  If it's a PC-relative relocation, subtract the PC (%edx)
	CMPL	$2, 20(%edi)		# rel->type
	JNE	.L21a
	SUBL	%edx, (%ecx)		# -P per Fig 4.4 in ABI386 v4
	SUBL	$0x08048074, (%ecx)	# .text load address

.L21a:
	ADDL	$28, %edi		# ++rel; sizeof(label)
	JMP	.L21
.L22:
	POP	%ecx			# syms
.L30:
	#  Unmap and close the output - already in %ebx
	MOVL	-12(%ebp), %ecx		# file size
	MOVL	$91, %eax		# 91 == __NR_munmap
	INT	$0x80

	#  Set the permissions on the output.
	#  If we're overwriting an existing file, the O_CREAT permissions
	#  to open(2) are ignored, so an explicit chmod is necessary.
	MOVL	$0x1ED, %ecx		# 0755
	MOVL	-16(%ebp), %ebx		# fd_out
	CMPL	$0xF0, -52(%ebp)	# Are we partial linking?
	JE	.L31
	MOVL	$0x1A4, %ecx		# 0644
.L31:
	MOVL	$94, %eax		# 94 == __NR_fchmod
	INT	$0x80
	TESTL	%eax, %eax
	JNZ	error

	MOVL	-16(%ebp), %ebx		# fd_out
	MOVL	$6, %eax		# 6 == __NR_close
	INT	$0x80

	XORL	%ebx, %ebx
	JMP	success


####    #  And finally, the entry point.
	#  Last per requirement for elfify.
	JMP	_start 
