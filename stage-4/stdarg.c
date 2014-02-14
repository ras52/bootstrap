/* stdarg.c
 *
 * Copyright (C) 2005, 2014 Richard Smith <richard@ex-parrot.com> 
 * All rights reserved.
 */

/* The intention is that the C library macro va_arg can be implemented as:
 * 
 *   #define va_arg( ap, type ) \
 *     ( * (type*) __va_arg( &(ap), sizeof(type) ) )
 */
__va_arg( ap, size ) {
    auto a = *ap;
    /* Round size to 4 byte alignment */
    size = (size + 3) & ~3;
    *ap += size;
    return a;
}

/* The intention is that the C library macro va_start can be implemented as:
 *
 *   #define va_start( ap, last ) \
 *     ( __va_start( &(ap), &(last), sizeof(last) ) )                     
 */
__va_start( ap, last, last_size ) {
    *ap = last;
    __va_arg( ap, last_size );
}
