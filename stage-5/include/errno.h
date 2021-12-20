/* <errno.h>  --  standard C library header for error diagnostics
 *  
 * Copyright (C) 2005, 2008, 2020 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */ 

#ifndef __RBC_ERRNO_INCLUDED
#define __RBC_ERRNO_INCLUDED

/* C90 7.1.4 defines three macros: EDOM, ERANGE and errno.  
 *
 * Other macro names beginning E[0-9A-Z] are reserved for use by the 
 * implementation for use as errno values (though obviously EOF conflicts).
 *
 * See discussion in C N1338 which suggests requiring errno to be a macro. */

#define EDOM 33
#define ERANGE 34

extern int errno; 

#define errno errno

#endif
