BOOTSTRAP STAGE 5

Stage 5 reimplements the compiler from stage 4 in the B-like language 
of stage-4 compiler.  This allows a significantly more advanced 
implementation, with the result that it supports most of syntax of K&R
C (most notably, a type system) and is more efficient.

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

The compiler is named ccx.

  Usage: ccx [--compatibility=N] [-o filename.s] filename.c

The -o option specifies the output file's name; if it not specified and
the input file has a .c or .i extension, the output is the same file
name but with a .s extension.  

The --compatibility=4 flag enables compatibility with the stage 4
compiler.  This will permit arbitrary assignment to implicit int (but
not any variable with a declared type).  It will give an error when
subscripting something other than an array of 4-byte objects; most
commonly this triggers with character arrays which had to be manipulated
with lchar and rchar in stage 4.  An error is also given when doing
pointer arithmetic on objects that are not single bytes. 

The new C-like compiler is used to implement a simple preprocessor, cpp.

  Usage: cpp [OPTIONS] FILENAME

  Options:
    --help              Displays the help text
    -o FILENAME         Specifies the output file name (default: stdout)
    -I DIRECTORY        Appends a directory to the header search path
    -D NAME[=VAL]       Pre-defines a macro, optionally with a value
    --include FILENAME  Prefixes the specified file to the input

Finally, a compiler driver called cc has been written to simplify the
use of the four build tools (cpp, ccx, as and ld).

  Usage: cc [OPTIONS] FILES...

  Options:
    --help              Displays the help text
    -o FILENAME         Specifies the output file name
    -E                  Halt after preprocessing, generating .i files
    -S                  Halt after compiling, generating .s files
    -c                  Halt after assembling, generating .o files
    -I DIRECTORY        Appends a directory to the header search path
    -D NAME[=VAL]       Pre-defines a macro, optionally with a value
    --compatibility=N   Sets compatibility with the stage N tools
    --nostdlib          Do not link against crt0.o and libc.o

Input files are distinguished using their extensions.  A .c file as
assumed to be a C file that needs preprocessing; a .i file is assumed
not to require preprocessing; a .s file is assumed to be in assembly;
and a .o file is assumed to be an object file.

The compiler driver instructs the preprocessor to search the include/
directory and prepend include=include/rbc_init.h (which currently only
defines the version number in __RBC_INIT).  The __DATE__ and __TIME__
macros are also defined here.

TODO:
  - Errors on duplicate declarations at global scope
  - Tentative definitions
  - Prototypes
  - Unions, bit fields (probably not in this stage?), floats
  - Macros with arguments, # and ##
