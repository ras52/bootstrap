/* <bits/null.h>  --  the NULL macro is defined by multiple headers
 *
 * Copyright (C) 2005, 2013 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */

#ifndef __RBC_BITS_NULL_INCLUDED
#define __RBC_BITS_NULL_INCLUDED

/* C90 7.1.6 Common definitions <stddef.h> (part implementation)
 *
 * "An integral constant expression with the value 0, or such an 
 * expression cast to type void *, is called a null pointer constant."
 * [C90 6.2.2.3]
 *
 * NULL "expands to an implementation-defined null pointer
 * constant." [C90 7.1.6]
 */

/* FIXME  There is no void type in stage 5. */
#define NULL 0

#endif
