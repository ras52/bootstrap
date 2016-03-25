/* <bits/file_access.h>  --  file access functions for <stdio.h>
 *
 * Copyright (C) 2005, 2013, 2015 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */

#ifndef __RBC_BITS_FILE_ACCESS_INCLUDED
#define __RBC_BITS_FILE_ACCESS_INCLUDED

#include <bits/file.h>
#include <bits/size_t.h>

/* These macros match values are hardcoded into stage-4/output.c. */

/* Values suitable for the mode argument to setvbuf */
#define _IOFBF  1  /* Fully buffered */
#define _IOLBF  2  /* Line buffered */
#define _IONBF  3  /* Unbuffered */

/* The size buffer required as the buf argument to setbuf. 
 * Note: This is not (necessarily) the default buffer size. */
#define BUFSIZ  4096

#if 0
/* C90 7.9.5.1:  The fclose function */
int fclose( FILE* stream );

/* C90 7.9.5.2:  The fflush function */
int fflush( FILE* stream );

/* C90 7.9.5.3:  The fopen function */
FILE *fopen( char const* filename, char const* mode );

/* C90 7.9.5.4:  The freopen function */
FILE *freopen( char const *filename, char const *mode, FILE *stream ); 

/* C90 7.9.5.5:  The setbuf function */
void setbuf( FILE *stream, char *buf );

/* C90 7.9.5.6:  The setvbuf function */
int setvbuf( FILE *stream, char *buf, int mode, size_t size );

#else

/* Temporary versions while we don't support prototypes. */
FILE *freopen();
FILE *fopen();

#endif

#endif
