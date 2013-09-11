BOOTSTRAP STAGE 5

Stage 5 reimplements the compiler from stage 4 in the B-like language 
of stage-4 compiler.  This allows a significantly more advanced 
implementation, with the result that it is both more efficient and 
supports a wider range of language constructs.

New features in stage 5 cc:
  - for loops
  - comma operator
  - goto and labelled statements
  - switch, case labels and default (implemented as sequential if-then-branch)
  - a type system, including integer and character types, pointers, arrays,
      functions, and structs

TODO:
  - Errors on duplicate declarations at global scope
  - Tentative definitions
  - More of a type system
