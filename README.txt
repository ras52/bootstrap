BOOTSTRAP EXPERIMENT

In this experiment I aim to develop from the ground up a working 
compiler, assembler, linker and library, for a C-like language.  We
start with a minimal program capable of generating itself from its
source and gradually develop higher level tools and abstractions, as 
follows.


Stage 0 -- unhex

  The starting point of the experiment is a tiny program for packing
  hexadecimal octets into binary.

Stage 1 -- unhexl & elfify

  This stage adds add tool to wrap a text section into a minimal ELF
  executable, as well as further developing the unhex program to support
  labels, references to earlier labels, and a freer input format that 
  allows comments.

Stage 2 -- as

  Here we introduce a light-weight assembler, written in machine code
  using no forward jumps.  It generates a text section that can be 
  wrapped with the earlier elfify program to produce an executable.

Stage 3 -- as & ld

  The assembler is rewritten in assembly language and is joined by a
  linker, which together allow for separate compilation.

Stage 4 -- cc

  The project's first compiler is added at this stage.  Its input 
  language is a typeless language similar to B, except that it uses
  byte addressing, and certain syntactic incompatibilities with C 
  have been removed.


The code in this project is copyright (C) Richard Smith, 2009-13.
