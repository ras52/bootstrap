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
To support this, the assembler now includes ELF .rel.text, .symtab and
.strtab sections in its output, removing the need for the stage 1 elfify
program.  The R_386_PC32 relocation type is used for relocations between
symbols in the .text section.

The assembler requires its source file to be suffixed .s and
automatically assigns the output file name by replacing the .s with a .o
suffix.

  Usage: as test.s

The linker can take arbitrary input and output file names.  The output
file is specified with the -o option which must be first on the command
line.

  Usage: ld -o executable file1.o file2.o ...

