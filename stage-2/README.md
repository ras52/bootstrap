# Bootstrap: Stage 2

Although the two stage 1 tools, `unhexl` and `elfify`, certainly eased the
process of writing code, manually assembling opcodes is still painful,
especially the encoding of ModR/M bytes for instructions such as `MOV`.
After the need to manually track file offsets for jumps, badly-encoded 
opcodes were the most frequent source of error when writing the stage 1
tools.  As the stage 1 tools have already alleviated the need to 
handle offsets manually, the next logical step is a lightweight 
assembler.  This is the main tool introduced in this stage.

The stage 2 assembler makes two passes over the assembler source code, 
the first building up a symbol table and the second writing out machine 
code.  This means that forwards jumps are supported (unlike in the stage
1 `unhexl`).  The supported instruction set is loosely based on the Intel
8086 instruction set, but with 32-bit addressing (which the 8086, a
16-bit microprocessor, lacked).  Some more recent instructions are added
such as the far conditional jumps (opcode `0F 8x`, introduced in the 386).

For many instructions, we simplify the implementation by only supporting
the general version (e.g.  the far jump) without the corresponding
special cases (e.g. near or short jumps).  Mandatory instruction size
suffixes (`L` for 32-bit, `B` for 8-bit) are used to limit each mnemonic to
a single opcode family; no 16-bit instructions are supported.  There is
no support for adressing that involves a SIB byte -- so `(%ebp)` is not
supported, though `0(%ebp)` is.

Labels are restricted to 11 characters, and a maximum of 256 labels are
allowed.  When a label appears as an argument to a mnemonic, it is 
always treated as a 32-bit program counter relative (pcrel) offset.
This is correct for `JMP`, `Jcc` and `CALL`, but wrong when trying to locate
data complied into the program.  There is a `.hex` directive that takes a
stream of hexadecimal octets (without their '0x' prefix); it is useful
for including data into the object file, or manually assembling an
unsupported instruction.  This is the only assembler directive
implemented in the stage-2 assembler.

The assembler uses AT&T syntax, with the source before the destination
for operations with two operands, as that is the *de facto* standard in
the Unix environment.  This introduces some complications into the 
assembler: for example, in `MOVL addr, %eax`, `addr` is to be interpreted
as a pcrel address, which means relative to the end of the instruction.
However, when `addr` is parsed, the instruction length is unknown: 
had the destination been `4(%ebp)`, that would have been longer.  This 
requires some look-ahead in the parser.

Instructions are supposed to be delimited by either a new line or a 
semicolon; however, this has relaxed to allow prefixes (such as `REP`) to 
be treated as instructions with no arguments (like `NOP`).  Thus 
`REP SCASB` is valid, though the assembler believes it to be two 
instructions, not one with a prefix.  The full grammar is:

```ebnf
  HWS          ::= [ \t]
  DIGIT        ::= [0-9]
  NZDIGIT      ::= [1-9]
  XDIGIT       ::= [0-9A-F]
  LCHAR        ::= [0-9A-Za-z_]
  LSTART       ::= [.A-Za-z_]
  CHAR         ::= any character

  comment      ::= '#' CHAR* '\n'
  endline      ::= HWS* ( comment | '\n' | ';' )
  identifier   ::= LSTART LCHAR+
  labeldef     ::= identifier ':'
  mnemonic     ::= identifier   # from the list of known mnemonics
  integer      ::= ( '0' 'x' XDIGIT+ | NZDIGIT DIGIT* | '0' )
  immediate    ::= '$' integer
  regname8     ::= 'al' | 'cl' | 'dl' | 'bl' | 'ah' | 'ch' | 'dh' | 'bh'
  regname32    ::= 'eax' | 'ecx' | 'edx' | 'ebx' | 'esp' | 'ebp' | 'esi' | 'edi'
  register     ::= '%' ( regname8 | regname32 )
  regmem       ::= register | integer? '(' '%' regname32 ')'
  argument     ::= HWS* ( immediate | identifier | regmem )
  arguments    ::= argument HWS* ',' arguments | argument
  instruction  := mnemonic arguments endline?
  octet        ::= HWS* XDIGIT XDIGIT
  hexbytes     ::= '.hex' octet* endline
  directive    ::= hexbytes
  file         ::= labeldef | instruction | directive | endline
```

The list of supported mnemonics is:

```
  ADCx ADDx ANDx BSFL BSRL CALL CBW CDQ CLC CLD CMPx CMPSx 
  CWDE DECx DIVx HLT IDIVx IMULx INCx INT Jcc JMP LEA LEAVE
  LODSx MOVx MOVSx MULx NEGx NOP NOTx ORx POP POPF PUSH
  PUSHF REP REPE REPNE RET SALx SARx SBBx SCASx SHLx SHRx
  STC STD STOSx SUBx TESTL XCHGL XORx.
```

In that list, `x` represents a size suffix `L` or `B`, and `cc` is a
condition (`A`, `AE`, `B`, `BE`, `C`, `E`, `G`, `GE`, `L`, `LE`, `O`, `P`, 
`PE`, `PO`, `S`, `Z`, together with the negative `Ncc` versions, 
[except for `PE` and `PO`]).  Some instructions have implicit arguments,
and they *must not* be specified in the source.  The shift opcodes
(`SAL`, `SAR`, `SHL`, `SHR`) always shift by `%cl` bits, and the
multiplication-like opcodes (`MUL`, `IMUL`, `DIV`, `IDIV`) 
always act of `%edx:%eax` (in 32-bit mode) or `%ax` (in 8-bit mode).

Not all of the instructions normally represented by these mnemonics are 
supported.  `INT` only takes a 8-bit immediate; `CALL`, `JMP` and `Jcc`
take a program counter relative 32-bit immediate; the unary arithmetics,
`INCx`, `DECx`, `NEGx`, `NOTx`, `SALx`, `SHLx`, `SARx`, `SHRx`, `MULx`, 
`IMULx`, `DIVx` and `IDIVx`, take a single 8- or 32-bit r/m operand
(matching the regmem production); and `PUSH` and `POP` take a 32-bit
r/m.  `LEA` takes a 32-bit r/m followed by a 32-bit register; the binary
arithmetics, `MOVx`, `ADDx`, `SUBx`, `ADCx`, `SBBx`, `CMPx`, `ANDx`,
`ORx` and `XORx` take either a r/m and a register (in either order)
or an immediate followed by a r/m, all of the appropriate size.  The
remaining mnemonics have no operands.

Because of the need to make two passes over the source, it takes the
name of the source code file as its only command line argument; a
`.text` section is printed on standard output.

> Usage: `as test.s > test.ts`

The output is not a valid executable -- it just the `.text` section.  It
therefore needs using in conjunction with the stage 1 `elfify` tool to
produce an executable.
