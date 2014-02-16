/* <bits/struct_tm.h>  --  the definition of struct tm 
 *
 * Copyright (C) 2014 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */

#ifndef __RBC_BITS_STRUCT_TM_INCLUDED
#define __RBC_BITS_STRUCT_TM_INCLUDED

/* struct tm
 *
 * The C standard requires all the following fields to be present, but 
 * imposes no order.  There seems no advantage to not following the order
 * in the standard.  Nor do we have a need for additional fields. 
 */
struct tm {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
};

#endif
