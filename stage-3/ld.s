# ld.s

# Copyright (C) 2011 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

# ########################################################################

elf_hdr:

### The ELF header.      start: 0x00; length: 0x34
# (Offset 0x20 requires offset to section headers;
# and offset 0x18 requires the entry point address.)
# 
#                        32-bit, LSB ('Intel') byte order, ELF v.1
#           magic-----\  |      /--------------------------------/
.hex_bytes  7F 45 4C 46  01 01 01 00    00 00 00 00  00 00 00 00
#           exe-\ 386-\  ELF-v1----\    **entry**---\  phdr-off--\
.hex_bytes  02 00 03 00  01 00 00 00    00 00 00 00  34 00 00 00
#           **shdroff**  flags-----\    ehsz\
.hex_bytes  00 00 00 00  00 00 00 00    34 00
#           phsz\ phn-\  shsz\ shn-\    shst\
.hex_bytes  20 00 01 00  28 00 03 00    02 00 

# Program header.  start: 0x34; length: 0x20
# (Offsets 0x44 and 0x48 require .text size + 0x54)
#
#           PT_LOAD---\  offset----\    vaddr-----\  paddr-----\
.hex_bytes  01 00 00 00  00 00 00 00    00 80 04 08  00 80 04 08 
#           **filesz**\  **memsz**-\    PF_R|PF_X-\  align-----\
.hex_bytes  00 00 00 00  00 00 00 00    05 00 00 00  00 10 00 00

# end of ELF + Program headers -- position: 0x54

sect_hdr:

# Null section header.   start: +0x00; length: 0x28
# c.f. Fig 4.10 in gABI 4.1
.hex_bytes  00 00 00 00  00 00 00 00    00 00 00 00  00 00 00 00
.hex_bytes  00 00 00 00  00 00 00 00    00 00 00 00  00 00 00 00
.hex_bytes  00 00 00 00  00 00 00 00

# Text section header.   start: +0x28; length: 0x28
# (Offset +0x3C requires text section size.)
#
#           name-str--\  PROGBITS--\    EXEC|ALLOC\  load-addr-\
.hex_bytes  01 00 00 00  01 00 00 00    06 00 00 00  54 80 04 08
#           offset----\  **size**--\    /-- Fig 4.12 in gABI --\
.hex_bytes  54 00 00 00  00 00 00 00    00 00 00 00  00 00 00 00
#           align-----\  entry-sz--\
.hex_bytes  04 00 00 00  00 00 00 00

# Shstrtab section hdr.  start: +0x50; length: 0x28
# (Offset +0x60 requires offset to shstrtab.)
#
#           name-str--\  STRTAB----\    no-flags--\  load-addr-\ 
.hex_bytes  07 00 00 00  03 00 00 00    00 00 00 00  00 00 00 00
#           **offset**\  size------\    /-- Fig 4.12 in gABI --\
.hex_bytes  00 00 00 00  11 00 00 00    00 00 00 00  00 00 00 00
#           align-----\  entry-sz--\
.hex_bytes  01 00 00 00  00 00 00 00

# The shared string table itself.  start +0x78; length: 0x11
#
.hex_bytes  00                             # NULL.      Offset 0x00
.hex_bytes  2E 74 65 78 74 00              # .text      Offset 0x01
.hex_bytes  2E 73 68 73 74 72 74 61 62 00  # .shstrtab  Offset 0x07

# End of end headers.   offset 0x89


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


####	#  Function:	void writedwat( int dw, int offset, int fd )
	#  Seeks to offset and writes a dword
writedwat:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%ebx

	#  Seek to offset
	XORL	%edx, %edx		# SEEK_SET=0
	MOVL	12(%ebp), %ecx		# offset
	MOVL	16(%ebp), %ebx		# fd
	MOVL	$19, %eax		# _NR_lseek
	INT	$0x80
	CMPL	$0, %eax
	JL	error

	#  Write the dword
	MOVL	$4, %edx
	LEA	8(%ebp), %ecx
	MOVL	16(%ebp), %ebx		# fd
	MOVL	$4, %eax		# 4 == __NR_write
	INT	$0x80
	CMPL	$4, %eax
	JL	error

	POP	%ebx
	POP	%ebp
	RET

####	#  Function:  void* findsect( void* elf, void** ptr, int strlen, 
        #                             char name[...] )
	#  The NAME contain the section name directly on the stack
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
	LEA	20(%ebp), %eax
	PUSH	%eax			# Pointer to NAME

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


####	#  Function:  void* readsyms( void* elf, int sectno, vec<label>* syms )
	#  Populate the SYM vector with data from .symtab / .strtab for
	#  the SECTNO section of ELF.
readsyms:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%ebx
	PUSH	%edi
	PUSH	%esi

	#  Allocate the vector
	MOVL	$1024, %ecx
	PUSH	%ecx
	CALL	malloc
	POP	%ecx
	MOVL	16(%ebp), %edx
	MOVL	%eax, (%edx)
	MOVL	%eax, 4(%edx)
	ADDL	%ecx, %eax
	MOVL	%eax, 8(%edx)

	#  The string literal ".symtab"
	PUSH	%eax			# scratch
	MOVL	%esp, %ecx		# ptr to scratch
	MOVL	$0x626174, %eax		# 'tab'
	PUSH	%eax
	MOVL	$0x6D79732E, %eax	# '.sym'
	PUSH	%eax
	MOVL	$7, %eax		# strlen(".symtab")
	PUSH	%eax
	PUSH	%ecx
	PUSH	8(%ebp)			# The elf file image
	CALL	findsect
	ADDL	$20, %esp		# POP x5
	POP	%ebx			# Pointer to section header

	# Find pointer to .symtab section => %edi
	MOVL	0x10(%ebx), %edi	
	ADDL	8(%ebp), %edi

	#  The string literal ".strtab"
	PUSH	%eax			# scratch
	MOVL	%esp, %ecx		# ptr to scratch
	MOVL	$0x626174, %eax
	PUSH	%eax
	MOVL	$0x7274732E, %eax
	PUSH	%eax
	MOVL	$7, %eax
	PUSH	%eax			# strlen(".symtab")
	PUSH	%ecx
	PUSH	8(%ebp)			# The elf file image
	CALL	findsect
	ADDL	$20, %esp		# POP x5
	POP	%eax

	# Find pointer to .strtab section => %esi
	MOVL	0x10(%eax), %esi
	ADDL	8(%ebp), %esi

.L10a:
	#  Check whether we're overrunning the object file's symbol table
	MOVL	0x10(%ebx), %ecx	# offset of start of section
	ADDL	0x14(%ebx), %ecx	# + size of section
	ADDL	8(%ebp), %ecx		# => ptr to end of section
	CMPL	%ecx, %edi
	JGE	.L10b			# Done

	#  Check the symbol is in the correct section
	#  NB  If the symbol is undefined, the section will be SHN_UNDEF==0
	MOVL	0xC(%edi), %eax
	MOVB	$16, %cl
	SHRL	%eax
	CMPL	12(%ebp), %eax
	JNE	.L10c			# continue

	#  Check we're not about to overrun the label vector
	MOVL	16(%ebp), %edx
	MOVL	4(%edx), %eax		# vec.end
	CMPL	8(%edx), %eax		# vec.end_store
	JL	.L10d

	#  Grow storage
	MOVL	8(%edx), %eax
	SUBL	(%edx), %eax
	XORL	%edx, %edx
	MOVL	$2, %ecx
	MULL	%ecx			# acts on %edx:%eax
	PUSH	%eax			# new size (twice old)
	MOVL	16(%ebp), %edx
	PUSH	(%edx)			# ptr
	CALL	realloc

	#  Store pointers
	MOVL	16(%ebp), %edx
	MOVL	%eax, (%edx)		# store new pointer
	POP	%ecx
	SUBL	%ecx, 4(%edx)
	ADDL	%eax, 4(%edx)		# new vec->end
	POP	%ecx
	ADDL	%eax, %ecx
	MOVL	%ecx, 8(%edx)		# new vec->end_store

.L10d:
	#  Do a strcpy
	PUSH	%esi
	ADDL	(%edi), %esi		# source = string table + st_name 
	PUSH	%esi
	CALL	strlen
	POP	%esi
	CMPL	$12, %eax
	JGE	error			# Buffer overrun
	MOVL	%eax, %ecx
	PUSH	%edi
	MOVL	16(%ebp), %eax
	MOVL	4(%eax), %edi		# dest = vec.end
	REP MOVSB
	POP	%edi
	POP	%esi

	#  Copy the symbol address
	MOVL	0x4(%edi), %ecx		# st_value
	MOVL	16(%ebp), %edx		# vec
	MOVL	4(%edx), %eax		# dest = vec->end
	MOVL	%ecx, 12(%eax)

	ADDL	$16, 4(%edx)		# ++vec->end
.L10c:
	ADDL	0x24(%ebx), %edi	# Add sh_entsize
	JMP	.L10a

.L10b:
	POP	%esi
	POP	%edi
	POP	%ebx
	POP	%ebp
	RET


	#  The NAME contain the section name directly on the stack
####    #  Not a proper function.
	#  Exits program
error:
	MOVL    $1, %ebx
success:
	MOVL    $1, %eax   # 1 == __NR_exit
	INT     $0x80


####    #  The main function.
	#  Stack is arranged as follows:
	#
	#	-4(%ebp)	void* in    # mmapped file data
	#	-8(%ebp)        int32 fd_in
	#      -12(%ebp)	int32 text_written
	#      -16(%ebp)        int32 fd_out
	#      -20(%ebp)	label* sym_end_store
	#      -24(%ebp)	label* sym_end
	#      -28(%ebp)	label* sym_start

	#  --- The main loop
_start:
	MOVL	%esp, %ebp
	SUBL	$40, %esp	

	XORL	%eax, %eax
	MOVL	%eax, -12(%ebp)

	#  Check we have exactly three command line argument (-o out in.o)
	CMPL	$4, 0(%ebp)
	JNE	error

	#  Check argv[1] is '-o'
	MOVL	8(%ebp), %eax
	CMPB	$0x2D, 0(%eax)		# '-'
	JNE	error
	CMPB	$0x6F, 1(%eax)		# 'o'
	JNE	error
	CMPB	$0x0, 2(%eax)		# NUL
	JNE	error

	#  Open the output file
	MOVL	$0x1FD, %edx		# 0755
	MOVL	$0x242, %ecx		# O_RDWR=2|O_CREAT=0x40|O_TRUNC=0x200
	MOVL	12(%ebp), %ebx
	MOVL	$5, %eax		# 5 == __NR_open
	INT	$0x80
	CMPL	$0, %eax
	JL	error
	MOVL	%eax, -16(%ebp)		# fout.fd

	#  Open the input file
	XORL	%ecx, %ecx		# 0 == O_RDONLY
	MOVL	16(%ebp), %ebx
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

	# The string literal ".text"
	PUSH	%eax			# scratch
	MOVL	%esp, %ecx		# ptr to scratch
	MOVL	$0x74, %eax
	PUSH	%eax
	MOVL	$0x7865742E, %eax
	PUSH	%eax
	MOVL	$5, %eax		# strlen(".text")
	PUSH	%eax
	PUSH	%ecx
	PUSH	-4(%ebp)		# The elf file image
	CALL	findsect
	ADDL	$20, %esp		# POP x5
	POP	%edi			# Ptr to sect header

	#  Read the symbol table
	LEA	-28(%ebp), %ecx
	PUSH	%ecx			# vec<label>*
	PUSH	%eax			# .text section number, from findsect
	PUSH	-4(%ebp)
	CALL	readsyms
	POP	%eax
	POP	%eax
	POP	%eax

	#  Locate the ELF header
	#  CALL next_line; next_line: POP %eax  simply effects  MOV %eip, %eax
	CALL	.L4
.L4:
	POP	%ecx		# Currently assembled sub-optimally as 2 bytes
	ADDL	elf_hdr, %ecx	# Assembled as six bytes (op, modrm, imm32)
	ADDL	$8, %ecx	# len of prev two instr

	#  Write the ELF & program header
	MOVL	$0x54, %edx		# Length of headers
	MOVL	-16(%ebp), %ebx		# fd
	MOVL	$4, %eax		# 4 == __NR_write
	INT	$0x80
	CMPL	$0x54, %eax
	JL	error

	# Write out the text segment
	MOVL	0x14(%edi), %edx	# sh_size member
	MOVL	0x10(%edi), %ecx	# sh_offset member
	ADDL	-4(%ebp), %ecx		# data
	MOVL	-16(%ebp), %ebx		# fd
	MOVL	$4, %eax   		# 4 == __NR_write
	INT	$0x80
	CMPL	0x14(%edi), %eax
	JNE	error
	ADDL	%eax, -12(%ebp)		# Bytes written in .text

	#  Locate the section headers & .shstrtab
	#  CALL next_line; next_line: POP %eax  simply effects  MOV %eip, %eax
	CALL	.L5
.L5:
	POP	%ecx		# Currently assembled sub-optimally as 2 bytes
	ADDL	sect_hdr, %ecx	# Assembled as six bytes (op, modrm, imm32)
	ADDL	$8, %ecx	# len of prev two instr

	#  Write the section headers & .shstrtab
	MOVL	$0x89, %edx		# Length of headers
	MOVL	-16(%ebp), %ebx		# fd
	MOVL	$4, %eax		# 4 == __NR_write
	INT	$0x80
	CMPL	$0x89, %eax
	JL	error

	PUSH	-16(%ebp)		# fd

	#  Link ELF header to the section headers
	MOVL	$0x20, %eax		# ELF shdroff
	PUSH	%eax
	MOVL	-12(%ebp), %ecx		# .text sh_size
	ADDL	$0x54, %ecx		# for ELF
	PUSH	%ecx
	CALL	writedwat
	POP	%ecx
	POP	%eax

	#  Tell the program header the loadable program size
	MOVL	$0x44, %eax		# ELF shdroff
	PUSH	%eax
	PUSH	%ecx
	CALL	writedwat
	POP	%ecx
	POP	%eax
	MOVL	$0x48, %eax		# ELF shdroff
	PUSH	%eax
	PUSH	%ecx
	CALL	writedwat
	POP	%ecx
	POP	%eax

	#  .text section header needs .text section size
	MOVL	-12(%ebp), %ecx	# .text sh_size
	ADDL	$0x54, %ecx		# for ELF
	ADDL	$0x3C, %ecx		# size field in second section header
	PUSH	%ecx
	PUSH	-12(%ebp)
	CALL	writedwat
	POP	%ecx
	POP	%eax

	#  Shstrtab header needs link to table
	MOVL	-12(%ebp), %ecx	# .text sh_size
	ADDL	$0x54, %ecx		# for ELF
	ADDL	$0x60, %ecx		# offset field in third section header
	PUSH	%ecx
	ADDL	$0x18, %ecx
	PUSH	%ecx			# .shstrtab data
	CALL	writedwat
	POP	%ecx	
	POP	%ecx	
	POP	%ecx	

	#  The string literal "_start"
	MOVL	$0x7472, %eax		# 'rt'
	PUSH	%eax
	MOVL	$0x6174735F, %eax	# '_sta'
	PUSH	%eax
	MOVL	$7, %eax		# strlen("_start") + 1 /* for NUL */
	PUSH	%eax
	MOVL	%esp, %eax		# }
	ADDL	$4, %eax		# }     LEA	4(%esp), %eax
	PUSH	%eax

	#  Scan the labels looking for _start
	MOVL	-28(%ebp), %edi		# label_start
.L11a:
	CMPL	-24(%ebp), %edi		# end data
	JGE	error

	PUSH	%edi
	CALL	strneq
	POP	%ecx
	CMPL	$0, %eax
	JNE	.L11b		# Found it

	ADDL	$16, %edi
	JMP	.L11a
.L11b:
	PUSH	-16(%ebp)		# fd
	MOVL	$0x18, %eax		# Offset of e_entry
	PUSH	%eax

	#  Get the function address
	MOVL	0xC(%edi), %ecx		# label.value
	ADDL	$0x54, %ecx		# for ELF
	ADDL	$0x08048000, %ecx	# Load address 0x08048000
	PUSH	%ecx
	
	CALL	writedwat
	ADDL	$0x12, %esp

	XORL	%ebx, %ebx
	JMP	success


####    #  And finally, the entry point.
	#  Last per requirement for elfify.
	JMP    _start 
