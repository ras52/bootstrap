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

Objects in the .data can be initialised with the address of other 
objects, e.g. by passing a symbol name to the argument of an .int 
directive.   These are handled by R_386_32 relocations which are
stored in a .rel.data section.

The .text and .data directives are used to switch between sections, and
several other new assembler directives are added.  The complete list is
now as follows

  .text .data .global .globl .local .int .byte .long .hex
  .zero .align .string

The .global (or equivalently, .globl) and .local directives take a
symbol name as their single argument.  They specify the binding of 
that symbol.  Global binding is currently the default (for compatibility
with stage 2), though that will be changed in a later stage.

The .int and .byte directives allow 32-bit and 8-bit integers to be
included directly into the output; .long is a synonym for .int.  
Multiple integers, separated by commas, can be included as arguments.  
Unlike the existing .hex directive (unchanged from stage 2) which only 
accepts hexadecimal octets without prefixes, these support any form of 
literal.  The .zero directive takes one argument and writes that number 
of zeros to the output.  The .align directive also writes a number of
zeros to the output, but the argument to .align is the alignment 
required.  So .align 16 outputs enough zero bytes to align the section 
to the next 16-byte boundary.

The .string directive allows for strings in double quotes with 
a maximum length of 78 characters.  They are automatically null 
terminated, and the following escapes understood:

  \n \t \" \\

The use of symbols as address immediates with a $ prefix (e.g. ADDL foo,
%eax) is deprecated.  In the stage-2 assembler, this stores the address 
of foo in %eax.  To get this behaviour in the stage-3 assembler, add the
$ prefix.  We also now support the versions of the MOV instruction that 
transfer a symbol value (rather than the address) to or from the 
accumulator.  Thus MOVL foo, %eax copies the symbol value (as is 
standard practice for that notation) and not the symbol address (as in 
stage 2).  The following instructions are also added:

  MOVSX, MOVZX, SETcc

Support for the following AT&T aliases for Intel mnemonics has also
been added:

  CBTW, CLTD, CWTL, MOVZBL, MOBSBL

Character literals are now allowed as immediates, enclosed in single 
quotes.  (Note this is unlike the GNU assembler, where character 
literals begin with a single quote, but do not have a closing quote.)  
They can be preceded by a $ which is optional (on the basis that no-one
would choose to write an address in terms of its ASCII representation).
The same escape characters are accepted as for strings.  Multicharacter 
literals are allowed where a 32-bit immediate is expected, and may 
contain upto four characters.  Their layout is such as to make them 
useful for short text fragments: 'xyz' the same layout as "xyz".

The assembler requires its source file to be suffixed .s and
automatically assigns the output file name by replacing the .s with a 
.o suffix.

  Usage: as test.s

The linker can take arbitrary input and output file names.  The output
file is specified with the -o option which must be first on the command
line.

  Usage: ld [-r] -o output file1.o file2.o ...

If -r is specified, the linker partially links its input generating an 
object file as output.  Without it, the output is an executable.  At 
present, even when partial linking, there cannot be any undefined 
symbols in the output.


TODO

undefined symbols when partially linking (ld -r)
?? case insensitive mnemonics, registers & hex numbers
octal numbers
