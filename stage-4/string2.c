/* string2.c  --  additional, higher-level string handling functions
 *
 * Copyright (C) 2013 Richard Smith <richard@ex-parrot.com> 
 * All rights reserved.
 */

/* The BSD extension strlcat() */
strlcat(dest, src, n) {
    auto l1 = strnlen(dest, n), l2 = strnlen(src, n-l1);
    strncpy( dest + l1, src, l2 );
    lchar(dest, l1 + l2 < n ? l1 + l2 : n - 1, '\0');
    return dest;
}   

/* The C library strcat() */
strcat(dest, src) {
    auto l1 = strlen(dest), l2 = strlen(src);
    strcpy( dest + l1, src );
    lchar(dest, l1 + l2, '\0');
    return dest;
}
