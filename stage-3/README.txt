BOOTSTRAP STAGE 3

One major limitation to the stage 2 assembler is that there are rather 
small fixed limits to many quantities: labels are limited to 11
characters, and there must be no more than 256 of them; lines can be no
more than 80 characters long.  These limits exist because there is no
heap allocation of memory, and fixed-sized arrays are declared on the
stack.  This stage takes the stage 2 assembler source, translates it 
into assembly language (from hexadecimal), and adds dynamic memory 
mangement to the assembler with a standard malloc() and realloc() 
interface.  This is used to remove the limit of 256 labels.

The present malloc implementation is very inefficient because it punts
all the work to the kernel with a mmap(MAP_ANON) syscall.  This results
in a new memory page being allocated for every block of memory
allocated.  For the present purpose this is acceptable, but it will
rapidly cease to be as code becomes more complex.  However, at present
there is a strong disincentive to implement a better malloc
implementation: that there is no mechanism for code reuse.  Bug fixes or
improvements to the implementation will tend to get lost or incorrectly
applied to the multiple copies scattered around the code.

To allow code reuse this stage introduces separate assembly and linking
steps allowing each executable to be formed from multiple object files.
This means that some object files (for instance, those containing the
malloc implementation) can linked into several different executables.
To support this, the assembler (which now writes ELF directly, removing
the need for the stage 1 elfify progrma) includes .rel.text, .symtab and
.strtab sections in its output.  The R_386_PC32 relocation type is used
for relocations between symbols in the .text section.

Another difficulty with the stage 2 assembler was that the only place to
store data was the stack (or one of the registers).  Variables like the
input read buffer had to be placed on the stack and pointers to it
passed around to all functions, with the result that the code was rarely
refactored into separate functions.  This is addressed by the stage-3
assembler which supports a writable .data section.  References in the
.text section to objects in the .data section are handled by way of
R_386_32 relocations.

The .int and .byte directives allow 32-bit and 8-bit integers to be
included directly into the output.  Unlike the existing .hex directive
which only accepts hexadecimal octets without prefixes, these support
any form of literal.  The .string directive allows for strings in
double quotes with a maximum length of 78 characters.  They are 
automatically null terminated, and the following escapes understood:

  \n \t \" \\

The use of symbols as address immediates (e.g. ADDL foo, %eax) is
deprecated.  In the stage-2 assembler, this stored the address of foo in
%eax.  The versions of the MOV instruction that transfer a symbol value
to or from the accumulator are now added, so that MOVL foo, %eax copies
the symbol value (as is standard practice for that notation) and not the
symbol address (as in stage 2).  The following instructions are also
added:

  HLT, LEAVE, MOVSX, MOVZX, SETcc

And support for the following AT&T aliases for Intel mnemonics has also
been added:

  CBTW, CLTD, CWTL, MOVZBL, MOBSBL

The assembler requires its source file to be suffixed .s and
automatically assigns the output file name by replacing the .s with a .o
suffix.

  Usage: as test.s

The linker can take arbitrary input and output file names.  The output
file is specified with the -o option which must be first on the command
line.

  Usage: ld -o executable file1.o file2.o ...

