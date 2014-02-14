BOOTSTRAP STAGE 5

Stage 5 reimplements the compiler from stage 4 in the B-like language 
of stage-4 compiler.  This allows a significantly more advanced 
implementation, with the result that it is both more efficient and 
supports a wider range of language constructs, most notably, a type 
system.

New features in stage 5 compiler:
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
  - C++-style comments
  - typedefs

The compiler proper is named ccx, and this is supplemented with a 
preprocessor (cpp) and compiler driver (cc).

  Usage: cpp [-I include-dir] [-D name[=val]] [-o filename.i] filename.c
  Usage: ccx [--compat] [-o filename.s] filename.i

TODO:
  - Errors on duplicate declarations at global scope
  - Tentative definitions
  - Prototypes
  - Unions, bit fields (probably not in this stage?), floats
  - #elif
  - Macros with arguments
