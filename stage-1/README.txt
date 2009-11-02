BOOTSTRAP STAGE 1

In this stage we add elfify, a program that takes a .text section and
converts it into a stand-alone ELF program.  Unlike the stage-0 unhex,
this program has section headers and a minimimal .shstrtab section
so that tools such as objdump -d will work on it.  It also adds these
to any executable it creates.

Because elfify needs to find out place the size of the .text section
in the ELF program header, it cannot act as a straightforward filter 
on standard input.  (Placing the program header at the end of the file
does not help because the program header's offset is needed in the ELF
header.)  Instead it takes the name of the file containing the .text
section its only command line argument.

  Usage: elfify test.ts > test

where .ts is used as the canonical extension for a .text section.
