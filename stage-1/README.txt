BOOTSTRAP STAGE 1

In writing the stage 0 unhex tool, the two most tedious and error-prone
tasks were generating valid ELF headers (which entailed keeping track
of the size of the .text section and the location of the entry point)
and calculating the file offsets used as arguments to various JMP and 
Jcc statements.  These two tasks were particularly prone to introduce
errors as the code was modified, perhaps to correct some error found
during testing.  Keeping all of the offsets and sizes updated proved
rather more onerous than manually convertible the assembly language 
into hexadecimal values.

Therefore stage 1 adds two new tools, unhexl and elfify, to handle 
these tasks.  The first, unhexl, is a significantly improved version of
the stage 0 unhex.  It allows arbitrary white-space and comments.  
More importantly, it allows labels to be defined and referenced.  The
grammar is:

  WS      := [ \t\n]
  XDIGIT  := [0-9A-F]
  LCHAR   := [0-9A-Za-z_] 
  LSTART  := [^ \t\n0-9A-F#]
  LREFEND := [^:]
  CHAR    := any character

  comment := '#' CHAR* '\n'
  octet   := XDIGIT XDIGIT
  label   := LSTART LCHAR+
  ldef    := label ':'
  lref    := label LREFEND

  file    := (comment | octet | lref | ldef | WS* )*

Label references are converted into little-endian 32-bit offsets 
relative to the end of the address being written.  For example, the 
following x86 assembly will generate an infinite loop with the label 
reference expanding to BF FF FF FF (or -5 in decimal). 

  foo:
    E9 foo

The program takes its source on standard input and does a single pass 
over it writing data to standard output.  

  Usage: unhexl < test.xl > test

The fact that it is a single pass means references can only be made to 
labels already defined.  The lack of dynamic memory allocation in stage 
1 means the number of labels is limited to 64.  For the same reason, 
the line length is limited to 80 characters: this constraint is not 
checked by the stage 1 program -- failure to stick to 80 character 
lines will result in a buffer overflow.  Other errors, including 
overflowing the symbol table and various syntax errors are flagged by a 
non-zero return status.



The second program, elfify, takes a .text section and converts it into 
a stand-alone ELF program.  Unlike the stage 0 unhex, this program has 
section headers and a minimimal .shstrtab section so that tools such as 
objdump -d will work on it.  It also adds these to any executable it 
creates.

Because elfify needs to find out place the size of the .text section
in the ELF program header, it cannot act as a straightforward filter 
on standard input.  (Placing the program header at the end of the file
does not help because the program header's offset is needed in the ELF
header.)  Instead it takes the name of the file containing the .text
section its only command line argument.

  Usage: elfify test.ts > test

where .ts is used as the canonical extension for a .text section.

As elfify does not parse the .text section, it cannot work out where 
the entry point is: it assumes that the entry point is 5 bytes before 
the end of the .text section.  It is therefore suggested that all 
programs end with a jump to the real entry point.  On x86, a 32-bit
relative jump requires precisely 5 bytes.
