BOOTSTRAP STAGE 2

Although the two stage 1 tools, unhexl and elfify, certainly eased the
process of writing code, manually assembling opcodes is still painful,
especially the encoding of ModR/M bytes for instructions such as MOV.
After the need to manually track file offsets for jumps, badly-encoded 
opcodes were the most frequent source of error when writing the stage 1
tools.  As the stage 1 tools have already alleviated the need to 
handle offsets manually, the next logical step is a light-weight 
assembler.  This is the main tool introduced in this stage.

  WS           := [ \t\n]
  DIGIT        := [0-9]
  XDIGIT       := [0-9A-F]
  LCHAR        := [0-9A-Za-z_]
  LSTART       := [.A-Za-z_]
  CHAR         := any character

  comment      := '#' CHAR* '\n'
  endline      := WS* ( comment | '\n' )
  identifier   := LSTART LCHAR+
  labeldef     := identifier ':'
  mnemonic     := identifier            # known to program as a mnemonic
  immediate    := '$' ( '0' 'x' XDIGIT+ | DIGIT+ )
  argument     := WS* immediate
  arguments    := argument WS* ',' arguments | argument
  instruction  := mnemonic arguments endline

