BOOTSTRAP EXPERIMENT

In this experiment I aim to develop from the ground up a working 
compiler, assembler, linker and library, for a C-like language.  I start
with a minimal program capable of generating itself from its source and
gradually develop higher level tools and abstractions, as follows.

This programs produced in this project are 32-bit ELF executables which
run on a Linux kernel running on an Intel x86 processor.  They run fine
on modern 64-bit systems.


Stage 0 -- unhex

  The starting point of the experiment is a tiny program for packing
  hexadecimal octets into binary.

Stage 1 -- unhexl & elfify

  This stage adds a tool to wrap a text section into a minimal ELF
  executable, as well as further developing the unhex program to support
  labels, references to earlier labels, and a freer input format that 
  allows comments.

Stage 2 -- as

  Here we introduce a light-weight assembler, written in machine code
  using no forward jumps.  It generates a text section that can be 
  wrapped with the stage 1 elfify program to produce an executable.

Stage 3 -- as & ld

  The assembler is rewritten in assembler language and is joined by a
  linker, which together allow for separate compilation units.

Stage 4 -- cc, crt0.o & libc.o

  The project's first compiler is added at this stage.  Its input
  language is a typeless subset of C similar to B, and it emits
  assembler language.  We also build a startup file (crt0.o) and the
  start of a simple C library.

Stage 5 -- ccx, cpp, cc & cmp

  The compiler is rewritten in its source language, and a type system
  added.  We use it to implement a fairly standards-compliant C 
  preprocessor, and a compiler driver that spawns the cpp, ccx (the
  compiler proper), as and ld.  Finally, cmp is a POSIX compliant
  utility to compare two files, which is used in a preproccesor test
  suite.


The code in this project is copyright (C) Richard Smith, 2009-2020, and
is licensed for use under version 3 or later of the GNU General Public
License, a copy of which can be found in the file LICENCE.txt.  The
documentation in these README.txt files is licensed under the Creative
Commons BY-NC-SA licence, version 4.
