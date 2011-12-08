BOOTSTRAP STAGE 3

One major limitation to the stage 2 assembler is that there are rather 
small fixed limits to many quantities: labels are limited to 11 
characters, and there must be no more than 256 of them; lines can be no
more than 80 characters long.  These limits exist because there is no
heap allocation of memory, and fixed-sized arrays are declared on the
stack.  This stage adds dynamic memory mangement to the assembler with 
a standard malloc() and realloc() interface, and uses this to remove
the 256 label limit.

The present malloc implementation is very inefficient because it punts
all the work to the kernel with a mmap(MAP_ANON) syscall.  This results
in a new memory page being allocated for every block of memory 
allocated.  For the present purpose this is acceptable, but it will
rapidly cease to be as code becomes more complex.  However, at present
there is a strong disincentive to implement a better malloc 
implementation: that there is no mechanism for code reuse.   Bug fixes
or improvements to the implementation will tend to get lost or 
incorrectly applied to the multiple copies scattered around the code.

This stage therefore introduces separate assembly and linking steps
allowing each executable to be formed from multiple object files.  This
means that some object files (for instance, those containing the 
malloc implementation) can linked into several different executables.
To support this, the assembler now includes ELF .rel.text, .symtab and 
.strtab sections in its output.
