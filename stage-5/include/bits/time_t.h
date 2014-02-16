/* <bits/struct_tm.h>  --  the definition of struct tm 
 *
 * Copyright (C) 2014 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */

#ifndef __RBC_BITS_TIME_T_INCLUDED
#define __RBC_BITS_TIME_T_INCLUDED

/* time_t
 *
 * The C standard simply requires this to be arithmetic, and not necessarily
 * even signed.  The Linux kernel ABI makes it a 32-bit signed type. 
 */
typedef int time_t;

#endif
