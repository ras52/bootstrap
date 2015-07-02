/* input.c
 *
 * Copyright (C) 2013 Richard Smith <richard@ex-parrot.com> 
 * All rights reserved.
 */

/* Buffers for the output streams */
static __buf0[32];

/*                    0     1       2       3       4       5    6     7
 * struct FILE      { fd    bufsz   bufp    buffer  bufend  mode bmode free } */
static __file0[8] = { 0,    128,    __buf0, __buf0, __buf0, 0,   1,    0    };
/* MODE is as per the second argument to open(2).  
 * BMODE is an _IO?BF flag 
 * FREE indicates whether the structure should be passed to free(3) in fclose */

/* The stdio objects themselves.  
 * We can't just use the arrays themselves because we need to force make 
 * lvalue versions of them, and arrays are only rvalues. */
stdin  = __file0;

/* Implementation detail __fgetsn() */
static
__fgetsn( ptr, len, stream ) {
    auto nread = 0;

    while ( len ) {
        auto avail = stream[4] - stream[2];

        /* If we have any data buffered, then read that */
        if ( avail ) {
            auto chunk = avail < len ? avail : len;
            memcpy( ptr, stream[2], chunk );
            stream[2] += chunk;
            ptr += chunk;
            len -= chunk;
            nread += chunk;
        }

        /* If stream[0] is -1, it means the stream is not backed by a file:
         * what's in the buffer is all there is. */
        else if ( stream[0] == -1 )
            return nread;

        /* Otherwise, if we require more than our buffer holds, read it
	 * directly into the supplied memory region. */
        else if ( len >= stream[1] - 1 ) {
            auto n = read( stream[0], ptr, len );
            if ( n <= 0 ) return nread;
            nread += n;
            len -= n;
        }

        /* And for short reads, try to read a whole buffer's worth */
        else {
	    /* The +1 and -1 is to allow for a one-character putback area */
            auto n = read( stream[0], stream[3] + 1, stream[1] - 1 );
            if ( n <= 0 ) return nread;

            stream[2] = stream[3] + 1;
            stream[4] = stream[2] + n;
        }
    }

    return nread;
}

/* The C library fread() */
fread( buf, size, nmem, stream ) {
    return __fgetsn( buf, size * nmem, stream ) / size; 
}

/* The C library fgetc() */
fgetc( stream ) {
    auto c = 0;
    if ( __fgetsn( &c, 1, stream ) == 1 ) return c;
    else return -1; /* EOF */
}

/* The C library fgets() */
fgets( s, n, stream ) {
    auto i = 0;

    /* TODO:  strchr for the first '\n' */
    while ( i < n-1 ) {
        auto c = fgetc( stream );
        if ( c == -1 ) break;
        lchar( s, i++, c );
        if ( c == '\n' ) break;
    }

    if (i) {
        lchar( s, i, 0 );  /* Null-terminate */
        return s;
    }
    return 0;
}

/* The C library getc() */
getc( stream ) {
    return fgetc( stream );
}

/* The C library getchar() */
getchar() {
    return fgetc( stdin );
}

/* The C library gets() -- this is unsafe */
gets( s ) {
    auto s = fgets( stdin, 2147483647 ); /* INT_MAX */
    auto l = strlen(s);
    /* Strip the terminating new-line, if there is one */
    if ( rchar(s, l) == '\n' ) lchar(s, l, 0);
    return s;
}

/* The C library ungetc() */
ungetc( c, stream ) {
    if ( c == -1 ) return -1;

    /* If we have nothing at all... */
    if ( stream[2] == stream[4] ) {
        lchar( stream[3], 0, c );
        stream[2] = stream[3];
        stream[4] = stream[3] + 1;
        return c;
    }

    /* If we have a putback slot ... */
    if ( stream[2] > stream[3] )
        return lchar( --stream[2], 0, c );

    return -1;
}

/* Non-standard */
ungetchar( c ) {
    return ungetc( c, stdin );
}

/* Skip whitespace and a sign, and return non-zero if negative */
static
skip_ws_sgn( str_ptr ) {
    auto c, p = *str_ptr, sgn = 0;

    while ( isspace( c = rchar(p, 0) ) )
        ++p;

    if (c == '-') {
        sgn = -1; ++p;
    }
    else if (c == '+')
        ++p;

    *str_ptr = p;
    return sgn;
}

/* Common part of strtoul and strtol.  Returns non-zero if 32-bit overflow. */
static
strtoi32( val_ptr, str_ptr, base ) {
    auto c, p = *str_ptr, overflow = 0;

    c = rchar( p, 0 );

    /* Look for a base prefix. */
    if ( (base == 0 || base == 16) && c == '0' ) {
        c = rchar( ++p, 0 );
        if (c == 'x') { base = 16; c = rchar( ++p, 0 ); }
        else if (base == 0) base = 8;
    }
    if ( base == 0 ) base = 10;

    /* Read the number. */
    while (!overflow) {
        /* Handling numbers > INTMAX and overflow is tricky because 
         * in stage-4 all numbers are signed.  In particular, a * b
         * involves an IMUL instruction.  We use __mul_add that uses
         * MUL instead of IMUL, and detects overflow. */
        if ( c >= '0' && c <= '0' + (base <= 10 ? base - 1 : 9) )
            overflow = __mul_add( val_ptr, base, c - '0' );
        else if ( base > 10 && c >= 'a' && c <= 'a' + base - 11 )
            overflow = __mul_add( val_ptr, base, c - 'a' + 10 );
        else if ( base > 10 && c >= 'A' && c <= 'A' + base - 11 )
            overflow = __mul_add( val_ptr, base, c - 'A' + 10 );
        else break;

        c = rchar( ++p, 0 );
    }

    *str_ptr = p;
    return overflow;
}

/* The C library strtoul() */
strtoul( nptr, endptr, base ) {
    extern errno;
    auto n = 0;

    /* A negative sign is accepted on an unsigned number.  
     * "-1" parses as a very big positive number. */
    auto neg = skip_ws_sgn( &nptr );

    if ( strtoi32( &n, &nptr, base ) ) {
        n = 0xFFFFFFFF; /* ULONG_MAX */
        errno = 34; /* ERANGE */
    }
    else if ( neg )
        n = -n;

    if ( endptr ) *endptr = nptr;
    return n;           
}

/* The C library strtol() */
strtol( nptr, endptr, base ) {
    extern errno;
    auto n = 0;

    auto neg = skip_ws_sgn( &nptr );

    /* We ignore the overflow return as it then returns 0xFFFFFFFF */
    strtoi32( &n, &nptr, base );

    if ( neg ) {
        /* 0x70000000 is -INT_MIN, which is representable when negative */
        if ( n & 0x70000000 && n != 0x70000000 ) {
            n = 0x70000000; /* INT_MIN */
            errno = 34; /* ERANGE */
        }
        else n = -n;
    }
    else if ( n & 0x70000000 ) {
        n = 0x7FFFFFFF; /* INT_MAX */
        errno = 34; /* ERANGE */
    }

    if ( endptr ) *endptr = nptr;
    return n;           
}

/* The C library atoi() */
atoi( str ) { 
    return strtol( str, 0, 10 );
}
