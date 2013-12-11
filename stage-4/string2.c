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

/* The C library memmove() */
memmove(dest, src, n) {
    /* If we're copying to earlier memory, or if the blocks do not overlap,
     * then a forwards copy, as done by memcpy, will be fine. */
    if ( dest < src || dest > src + n ) return memcpy(dest, src, n);
 
    /* Otherwise we set the direction flag (DF), then call memcpy with the
     * end pointers (which will then copy backwards), and clear DF. 
     * We do not clear DF in memcpy because the ABI requires DF always to
     * be cleared before library calls. */
    __asm_std(); 
    memcpy(dest+n-1, src+n-1, n); 
    __asm_cld();
    return dest;
}

/* The C library strdup() */
strdup( str ) {
    auto l = strlen(str);
    auto str2 = malloc(l + 1);
    strcpy( str2, str );
    lchar( str2, l, 0 );
    return str2;
}
