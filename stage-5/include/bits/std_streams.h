/* <bits/std_streams.h>  --  define stdin, stdout & stderr
 *  
 * Copyright (C) 2005, 2015 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */ 

#ifndef __RBC_BITS_STD_STREAMS_INCLUDED
#define __RBC_BITS_STD_STREAMS_INCLUDED

#include <bits/file.h>

/* C90 7.9.1  The definition of the standard stream macros.
 *
 * They "are expressions of type "pointed to FILE" that point to the FILE 
 * objects associated, respectively, with standard error, input, and output 
 * stream."  But we want ELF objects of the same name. */
extern FILE *stdin, *stdout, *stderr;

#define stdin  stdin
#define stdout stdout
#define stderr stderr

#endif 
