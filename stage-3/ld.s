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
	#      -12(%ebp)	unused
	#      -16(%ebp)        int32 fd_out
	#      -20(%ebp)	int32 shoff
	#      -24(%ebp)	int32 shentsz
	#      -28(%ebp)	int32 endshoff
	#      -32(%ebp)	int32 shstroff
	#      -36(%ebp)	int32 .text shndx

	#  --- The main loop
_start:
	MOVL	%esp, %ebp
	SUBL	$36, %esp	

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
	POP	%edx			#                          } ADDL 0x58
	ADDL	$0x40, %esp		# $0x58 - 0x14 - 4         }

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

	#  Find shoff (at 0x20)
	MOVL	-4(%ebp), %ecx
	MOVL	0x20(%ecx), %eax
	ADDL	-4(%ebp), %eax
	MOVL	%eax, -20(%ebp)		# shoff

	#  Find shentsz (at 0x2E)
	MOVL	0x2E(%ecx), %eax
	ANDL	$0x0000FFFF, %eax
	MOVL	%eax, -24(%ebp)		# shentsz

	#  Find endshstr from shnum (at 0x30)
	MOVL	0x30(%ecx), %eax
	ANDL	$0x0000FFFF, %eax
	MULL	-24(%ebp)		# acts on %eax: result in %edx:%eax
	ADDL	-20(%ebp), %eax
	MOVL	%eax, -28(%ebp)		# endshstr

	#  Find .shstrtab section
	MOVL	0x32(%ecx), %eax
	ANDL	$0x0000FFFF, %eax
	MULL	-24(%ebp)		# acts on %eax: result in %edx:%eax
	ADDL	-20(%ebp), %eax		# beginning of shstrtab section
	MOVL	0x10(%eax), %eax	# sh_offset member
	ADDL	-4(%ebp), %eax
	MOVL	%eax, -32(%ebp)		# shstrstr offset

	# The string literal ".text\0\0\0"
	MOVL	$0x74, %eax
	PUSH	%eax
	MOVL	$0x7865742E, %eax
	PUSH	%eax

	MOVL	$6, %eax
	PUSH	%eax			# strlen(".text") + 1 /* for NUL */

	MOVL	%esp, %eax		# }
	ADDL	$4, %eax		# }     LEA	4(%esp), %eax
	PUSH	%eax

	#  Iterate though section looking for .text
	XORL	%eax, %eax
	MOVL	%eax, -36(%ebp)		# .text shndx counter
	MOVL	-20(%ebp), %edi
.L1:
	CMPL	-28(%ebp), %edi
	JGE	error

	#  The section name
	MOVL	(%edi), %eax
	ADDL	-32(%ebp), %eax
	PUSH	%eax
	CALL	strneq
	POP	%ecx
	CMPL	$0, %eax
	JNE	.L2

	ADDL	-24(%ebp), %edi		# += shentsz
	INCL	-36(%ebp)
	JMP	.L1
.L2:
	ADDL	$0x10, %esp		# POP x4

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
	JL	error

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
	MOVL	0x14(%edi), %ecx	# .text sh_size
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
	MOVL	0x14(%edi), %ecx	# .text sh_size
	ADDL	$0x54, %ecx		# for ELF
	ADDL	$0x3C, %ecx		# size field in second section header
	PUSH	%ecx
	PUSH	0x14(%edi)
	CALL	writedwat
	POP	%ecx
	POP	%eax

	#  Shstrtab header needs link to table
	MOVL	0x14(%edi), %ecx	# .text sh_size
	ADDL	$0x54, %ecx		# for ELF
	ADDL	$0x60, %ecx		# offset field in third section header
	PUSH	%ecx
	ADDL	$0x18, %ecx
	PUSH	%ecx			# .shstrtab data
	CALL	writedwat
	POP	%ecx	
	POP	%ecx	
	POP	%ecx	

	#  The string literal ".symtab\0"
	MOVL	$0x626174, %eax
	PUSH	%eax
	MOVL	$0x6D79732E, %eax
	PUSH	%eax

	MOVL	$8, %eax
	PUSH	%eax			# strlen(".symtab") + 1 /* for NUL */

	MOVL	%esp, %eax		# }
	ADDL	$4, %eax		# }     LEA	4(%esp), %eax
	PUSH	%eax

	#  Iterate though section looking for .symtab
	MOVL	-20(%ebp), %edi
.L6:
	CMPL	-28(%ebp), %edi
	JGE	error

	#  The section name
	MOVL	(%edi), %eax
	ADDL	-32(%ebp), %eax
	PUSH	%eax
	CALL	strneq
	POP	%ecx
	CMPL	$0, %eax
	JNE	.L7

	ADDL	-24(%ebp), %edi		# += shentsz
	JMP	.L6
.L7:
	ADDL	$0x10, %esp		# POP x4

#	#  The string literal ".strtab\0"
	MOVL	$0x626174, %eax
	PUSH	%eax
	MOVL	$0x7274732E, %eax
	PUSH	%eax

	MOVL	$8, %eax
	PUSH	%eax			# strlen(".strtab") + 1 /* for NUL */

	MOVL	%esp, %eax		# }
	ADDL	$4, %eax		# }     LEA	4(%esp), %eax
	PUSH	%eax

	#  Iterate though section looking for .strtab
	MOVL	-20(%ebp), %esi
.L8:
	CMPL	-28(%ebp), %esi
	JGE	error

	#  The section name
	MOVL	(%esi), %eax
	ADDL	-32(%ebp), %eax
	PUSH	%eax
	CALL	strneq
	POP	%ecx
	CMPL	$0, %eax
	JNE	.L9

	ADDL	-24(%ebp), %esi		# += shentsz
	JMP	.L8
.L9:
	ADDL	$0x10, %esp		# POP x4

	MOVL	0x10(%esi), %esi	
	ADDL	-4(%ebp), %esi		# %esi is .strtab
	MOVL	%edi, %ebx		# %ebx is symbol table section header
	MOVL	0x10(%edi), %edi
	ADDL	-4(%ebp), %edi		# %edi is .symtab

	#  The string literal "_start\0\0"
	MOVL	$0x7472, %eax
	PUSH	%eax
	MOVL	$0x6174735F, %eax
	PUSH	%eax

	MOVL	$7, %eax
	PUSH	%eax			# strlen("_start") + 1 /* for NUL */

	MOVL	%esp, %eax		# }
	ADDL	$4, %eax		# }     LEA	4(%esp), %eax
	PUSH	%eax

	#  Scan the symbol table for _start
.L10:
	#  Check whether we're overrunning the symbol table
	MOVL	0x10(%ebx), %ecx	# offset of start of section
	ADDL	0x14(%ebx), %ecx	# + size of section
	ADDL	-4(%ebp), %ecx		# => ptr to end of section
	CMPL	%ecx, %edi
	JGE	error

	MOVL	(%edi), %ecx		# st_name in symbol table entry
	ADDL	%esi, %ecx
	PUSH	%ecx
	CALL	strneq
	POP	%ecx
	CMPL	$0, %eax
	JNE	.L11

	ADDL	0x24(%ebx), %edi	# Add sh_entsize
	JMP	.L10
.L11:
	ADDL	$0x10, %esp		# POP x4

	#  Check the symbol is in the .text section
	MOVL	0xC(%edi), %eax
	MOVB	$16, %cl
	SHRL	%eax			# by %cl
	CMPL	-36(%ebp), %eax
	JNE	error

	PUSH	-16(%ebp)		# fd
	MOVL	$0x18, %eax		# Offset of e_entry
	PUSH	%eax

	#  Get the function address
	MOVL	0x4(%edi), %ecx		# st_value
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
