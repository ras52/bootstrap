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
so that ptr[1] is equivalent to *(ptr + 4).  When types are introduced
in a subsequent stage, this behaviour can be preserved because 
subscripting an int (other than with a pointer) is not legal in C.

Summary of differences from B:

  * Compound assignment operators are spelt OP= instead of =OP.
  * There are no relop assignment operators (e.g. =<, =>=, ===).
  * External declarations require '=' (i.e. 'i = 42' not 'i 42').
  * Arrays require a size (i.e. 'auto a[1] = {0}' not 'auto a[] = {0}').
  * Arrays with too many intialisers do not expand to accommodate them.
  * The '{' ... '}' around single-statement functions are required.
  * We support logical && and || complete with short circuiting.
  * We support the 'continue' keyword from C.
  * We don't allow backspace (character 0x7F) or dot (.) in identifiers.
  * The escape characters in strings is \ not *, and there is no \e.


TODO
  Switch statements
  Goto and labels
  Global-scope arrays
  Function-scope static variables
  Function pointers
