BOOTSTRAP STAGE 5

Stage 5 reimplements the compiler from stage 4 in the B-like language 
of stage-4 compiler.  This allows a significantly more advanced 
implementation, with the result that it is both more efficient and 
supports a wider range of language constructs, most notably, a type 
system.

New features in stage 5 cc:
  - for loops
  - comma operator
  - goto and labelled statements
  - switch, case labels and default (implemented as sequential if-else)
  - a type system, including all of C's integer and character types, 
      pointers, arrays and function pointers
  - structs
  - member access with the -> and . operators
  - sizeof operator
  - type casts


  Usage: cc -S [--compat] [-o filename.s] filename.c

  Usage: cpp [-I include-dir] [-o filename.i] filename.c


TODO:
  - Errors on duplicate declarations at global scope
  - Tentative definitions
  - Typedefs
  - Unions, bit fields (probably not in this stage?), floats
  - #if, #elif, #else
  - Macros with arguments
