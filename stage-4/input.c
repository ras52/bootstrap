/* input.c
 *
 * Copyright (C) 2013 Richard Smith <richard@ex-parrot.com> 
 * All rights reserved.
 */

/* Implementation detail _fgetsn() -- TODO declare this static */
_fgetsn( ptr, len, stream ) {
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
    return _fgetsn( buf, size * nmem, stream ) / size; 
}

/* The C library fgetc() */
fgetc( stream ) {
    auto c = 0;
    if ( _fgetsn( &c, 1, stream ) == 1 ) return c;
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

/* The C library atoi() */
atoi( str ) {
    auto c = rchar( str, 0 ), n = 0;
    if (c == '-') return -atoi( str+1 );

    while ( c >= '0' && c <= '9' ) {
        n = n * 10 + c - '0';
        c = rchar( ++str, 0 );
    }

    return n;
}
