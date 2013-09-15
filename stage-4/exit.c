/* exit.c
 *
 * Copyright (C) 2013 Richard Smith <richard@ex-parrot.com> 
 * All rights reserved.
 */

static atexit_vec[3] = { 0, 0, 0 };  /* start, end, end_store */

atexit(fn) {
    if ( !atexit_vec[0] ) {
        auto sz = 1 * 4;  /* 4 == sizeof( void (*)() ) */
        auto p = malloc(sz);
        atexit_vec[0] = atexit_vec[1] = p;
        atexit_vec[2] = p + sz;
    }
    else {
        auto sz = atexit_vec[1] - atexit_vec[0];
        auto p = atexit_vec[0];
        if ( atexit_vec[1] == atexit_vec[2] ) {
            p = realloc( atexit_vec[0], 2*sz );
            atexit_vec[0] = p;
            atexit_vec[1] = p + sz;
            atexit_vec[2] = p + 2*sz;
        }
    }

    *atexit_vec[1] = fn;
    atexit_vec[1] += 4;  /* sizeof( void (*)() ) */
    return 1;
}

exit(code) {
    /* Call registered functions in reverse order */
    auto ptr = atexit_vec[1], start = atexit_vec[0];
    while ( ptr > start ) {
        ptr -= 4;  /* sizeof( void (*)() ) */
        (*ptr)();
    }

    /* We don't need to flush stdout and stderr, because there is an 
     * atexit handler registered that does that. */

    _exit( code );
}
