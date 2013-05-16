/* stdlib.c  --  miscellaneous standard library functions
 *
 * Copyright (C) 2013 Richard Smith <richard@ex-parrot.com> 
 * All rights reserved.
 */

/* The C library abs() */
abs(n) {
    return n >= 0 ? n : -n;
}
