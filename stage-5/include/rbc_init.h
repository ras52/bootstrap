/* rbc_init.h  --  definitions read at the start of every compilation 
 * 
 * Copyright (C) 2013, 2015 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */

/* The primary purpose of this file was so that neither the compiler
 * driver, nor the preprocessor, need to be updated when the compiler
 * is updated to include new functionality. */

/* The current compiler version.  Note version numbers are stages, 
 * so 4 was the first version, and this is 5. */
#define __RBC_VERSION 5

/* To allow headers in this stage to be used later, it's convenient to
 * allow some extra keywords to appear in them. */
#define void int
#define const
