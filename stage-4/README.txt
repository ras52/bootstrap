BOOTSTRAP STAGE 4

The main product of stage 4 is a complier.  The original intention had 
been to implement a B compiler, perhaps with the caveat that the few
constructs that changed syntactically between B and C would be 
implemented in the C way -- for example, += not =+, and 'extern' not
'extrn'.  But it rapidly became obvious that the B memory model was
not backwards compatible with the C memory model on machines with byte
addressing.  Implementing B on a x86 would require pointers to be
represented as integer offsets into memory, and a pointer dereference,
*ptr, would translate into a ModR/M + SIB instruction: (%ebx, %eax, 4) 
where %ebx is NULL and %eax is the pointer value.  Taking the address 
of an automatic variable would be worse and involve an explicit bit-
shift.

Nevertheless, B's lack of a type system significantly simplifies the 
implementation, and this feature of B has been retained in the stage 4 
compiler.  Our single type is a 32-bit integer which also serves as an
address.  Incrementing the value increments the underlying address by 
one, as with a char* in C.  This means, that unlike in B, incrementing 
an address  does not move to the next integer in an array: use ptr += 4 
for that.  However, subscripting with [] works with 32-bit word offsets,
so that ptr[1] is equivalent to *(ptr + 4).  To treat a pointer as a
string and get character-level access, B uses two functions lchar(s,n)
and rchar(s,n,c) to get and set a character, respectively, at the given
offset.  These are provided in char.s.  When types are introduced in a
subsequent stage, this behaviour can be preserved because subscripting
an int (other than with a pointer) is not legal in C.

For forwards compatibility, certain type constructs are allowed and
completely ignored.  The (otherwise unsupported) int keyword may be
placed immediately after auto, and the identifiers in an auto 
declaration can be preceded with one or more *.  A list of parameter 
declarations by precede the opening brace of a function.

Summary of differences from B:

  * Compound assignment operators are spelt OP= instead of =OP.
  * There are no relop assignment operators (e.g. =<, =>=, ===).
  * Definitions require '=' (i.e. 'i = 42' not 'i 42').
  * Arrays require a size (i.e. 'auto a[1] = {0}' not 'auto a[] = {0}').
  * Arrays with too many intialisers do not expand to accommodate them.
  * The '{' ... '}' around single-statement functions are required.
  * We support logical && and || complete with short circuiting.
  * We support the 'continue' keyword from C.
  * We support C's 'do' ... 'while' loop construct.
  * We allow 'static' on global variables and functions.
  * The return statement does not require brackets.
  * We don't allow backspace (character 0x7F) or dot (.) in identifiers.
  * The escape characters in strings is \ not *, and there is no \e.
  * We don't support the switch statement, and therefore case labels.
  * We don't support goto and labeled statements.

The stage 4 compiler is a simple afair, making a single pass over the 
input file and code generation is done straight out of the parser,
without building an abstract syntax tree (AST) representation.  This 
means that the code generated is very inefficient, and even very obvious
optimisations are not made.  As an example, all lvalue-to-rvalue
conversions are done as separate statements, so to read a local auto
variable, we generated LEA -offset(%ebp), %eax; MOVL (%eax), %eax
instead of the more obvious MOVL -offset(%ebp), %eax.

  Usage: cc -S file.c

The compiler is initially linked against a trivial I/O library that
implements the basic C I/O functions in an unbuffered manner, doing one
syscall per call to getchar() or putchar().  Similarly, malloc() is
implemented as in stage 3, by sending each allocation request to the
kernel as a mmap(MAP_ANON) call.  The resultant compiler, cc0, is used
to compile an improved set of I/O and memory-management functions that 
do buffering.  These are linked together with ld -r into a proto-C-
library, libc.o.  There is also a trivial startup file, crt0.o, that 
implements _start() by calling exit(main()).  We use these to relink 
the compiler against this to produce a significantly faster compiler.

Linking a program is typically achieved with a command such as:

  ld -o prog libc.o crt0.o file1.o file2.o ...

