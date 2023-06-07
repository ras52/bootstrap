# Bootstrap: Stage 1

In writing the stage 0 `unhex` tool, the two most tedious and error-prone
tasks were generating valid ELF headers, which entailed keeping track
of the size of the `.text` section and the location of the entry point,
and calculating the file offsets used as arguments to various `JMP` and 
`J*cc*` statements.  These two tasks were particularly prone to introduce
errors as the code was modified, perhaps to correct some error found
during testing.  Keeping all of the offsets and sizes updated proved
rather more onerous than manually converting the assembly language 
into hexadecimal values.

Therefore stage 1 adds two new tools, `unhexl` and `elfify`, to handle 
these tasks.  The first, `unhexl`, is a significantly improved version of
the stage 0 `unhex`.  It allows arbitrary white-space and comments.
More importantly, it allows labels to be defined and referenced – that
is what the `l` at the end of the program name refers to.

The grammar is:

```ebnf
  WS      ::= [ \t\n]
  XDIGIT  ::= [0-9A-F]
  LCHAR   ::= [0-9A-Za-z_] 
  LSTART  ::= [^ \t\n0-9A-F#]
  LREFEND ::= [^:0-9A-Za-z_]
  CHAR    ::= any character

  comment ::= '#' CHAR* '\n'
  octet   ::= XDIGIT XDIGIT
  label   ::= LSTART LCHAR+
  ldef    ::= label ':'
  lref    ::= label LREFEND

  file    ::= ( comment | octet | lref | ldef | WS* )*
```

In order to keep the grammar simple, only upper case letters are 
accepted in hexadecimal octets and labels must not start with a
valid hexadecimal digit.  It is suggested that labels start with
a '.' or a lower case letter.

Label references are converted into little-endian 32-bit offsets 
relative to the end of the address being written.  For example, the 
following x86 assembly will generate an infinite loop with the label 
reference expanding to `BF FF FF FF` (or -5 in decimal). 

```asm
  foo:
    E9 foo
```

The program takes its source on standard input and does a single pass 
over it writing data to standard output. 

> Usage: `unhexl < test.xl > test`

The fact that it is a single pass means references can only be made to 
labels already defined.  The lack of dynamic memory allocation in stage 
1 means the number of labels is limited to 256.  For the same reason, 
the line length is limited to 80 characters.  These constraints are not 
checked by the stage 1 program – failure to stick to 80 characters per 
lines or 256 labels *will* result in a buffer overflow.  Some other 
errors, including various syntax errors are flagged by a non-zero return 
status, but basically, only minimal error checking is done.

The second program, `elfify`, takes a `.text` section and converts it into 
a stand-alone ELF program.  Unlike the stage 0 `unhex`, this program has 
section headers and a minimimal `.shstrtab` section so that tools such as 
`objdump -d` will work on it.  It also adds these to any executable it 
creates.

Because `elfify` needs to find out place the size of the `.text` section
in the ELF program header, it cannot act as a straightforward filter 
on standard input.  (Placing the program header at the end of the file
does not help because the program header's offset is needed in the ELF
header.)  Instead it takes the name of the file containing the `.text`
section as its only command line argument.

> Usage: `elfify test.ts > test`

where `.ts` is used as the canonical extension for a `.text` section.

As `elfify` does not parse the `.text` section, it cannot work out where 
the entry point is: it just assumes that the entry point is 5 bytes
before the end of the `.text` section.  It is therefore suggested that all
programs end with a jump to the real entry point.  On x86, a 32-bit
relative jump requires precisely 5 bytes and can easily be generated
by `unhexl` by ending the file with:

```asm
  E9 main
```

We check the stage 1 tools are working correctly by using them to build
a new copy of `unhexl` from source in its own input language.  (This is
why there are two copies of the source for `unhexl`: a `.ts.x` file for
processing with the stage 0 `unhex`, and a `.ts.xl` file for use with the
stage 1 `unhexl`.)  This is repeated, and the second and third generation
unhexl binaries are required to be identical.
