/* input.c
 *
 * Copyright (C) 2013, 2020 Richard Smith <richard@ex-parrot.com> 
 * All rights reserved.
 */

/* Buffers for the input streams.  Note, because we're being compiled by the
 * stage 4 compiler, arrays are int arrays, meaning these are 4*32 = 128 bytes
 * long: hence the 128 bufsz parameter in the stream definitions. */
static __buf0[32];

/*                    0     1       2       3       4       5    6     7
 * struct FILE      { fd    bufsz   bufp    buffer  bufend  mode bmode free } */
static __file0[8] = { 0,    128,    __buf0, __buf0, __buf0, 0,   1,    0    };
/* BUFEND is the end of data read into the buffer from the file. 
 * MODE is as per the second argument to open(2).  
 * BMODE is an _IO?BF flag 
 * FREE is a bit field, with 1 indicating that the buffer should be freed, 
 * 2 indicating that the stream itself should be freed, and 3 both. */

/* The stdio objects themselves.  
 * We can't just use the arrays themselves because we need to force make 
 * lvalue versions of them, and arrays are only rvalues. */
stdin  = __file0;

/* Implementation detail __fgetsn(): reads LEN bytes from STREAM into PTR,
 * and returns the number of bytes read. */
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
    auto s = fgets( stdin, 0x7FFFFFFF ); /* INT_MAX */
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

/* Skip whitespace and a sign, and return the sign character, if any */
static
skip_ws_sgn( stream ) {
    auto c = fskipws( stream );
    if (c == '+' || c == '-') { 
        fgetc(stream); 
        return c; 
    }
    return 0;
}

/* Common part of strtoul and strtol: read an unsigned 32-bit integer
 * without a sign, but possibly with a hex or octal prefix.
 * Returns 0 on success, 1 on overflow, and 2 on malformed input. */
static
strtoi32( val_ptr, stream, base ) {
    auto overflow = 0;

    auto c = fgetc( stream );

    /* Look for a base prefix. */
    auto hex_prefix = 0;
    if ( (base == 0 || base == 16) && c == '0' ) {
        c = fgetc( stream );
        if (c == 'x') { base = 16; c = fgetc( stream ); hex_prefix = 1; }
        /* If we have 0 not followed by x, it is an octal prefix, or a 
         * bare 0.  Unget the next character so that we have something to
         * parse if it is a bare 0. */
        else if (base == 0) { base = 8; ungetc( c, stream ); c = '0';  }
    }
    if ( base == 0 ) base = 10;

    /* If the number does not start with a valid digit (after any prefix), 
     * then it is not of the 'expected form' and we should backtrack. */
    if (!( c >= '0' && c <= '0' + (base <= 10 ? base - 1 : 9) ||
           base > 10 && c >= 'a' && c <= 'a' + base - 11 || 
           base > 10 && c >= 'A' && c <= 'A' + base - 11 )) {
      if (hex_prefix) { ungetc( 'x', stream ); ungetc( '0', stream ); }
      return 2;  /* Malformed input */
    }

    /* We now know that we are going to have a valid integer (albeit perhaps
     * an overflowed one), so can start writing to *val_ptr. */
    *val_ptr = 0;

    /* Read the number.  Do not stop on overflow: we must keep going to
     * the end of the sequence of digits. */
    while (1) {
	/* Handling numbers > INTMAX and overflow is tricky because in stage-4
	 * all numbers are signed.  In particular, a * b involves an IMUL
	 * instruction.  We use __mul_add that uses MUL instead of IMUL, and
	 * detects overflow.  Its returns is the high dword from the result,
	 * but all we are about is whether it's zero.  |=ing two non-zero
         * results always gives a non-zero result. */
        if ( c >= '0' && c <= '0' + (base <= 10 ? base - 1 : 9) )
            overflow |= __mul_add( val_ptr, base, c - '0' );
        else if ( base > 10 && c >= 'a' && c <= 'a' + base - 11 )
            overflow |= __mul_add( val_ptr, base, c - 'a' + 10 );
        else if ( base > 10 && c >= 'A' && c <= 'A' + base - 11 )
            overflow |= __mul_add( val_ptr, base, c - 'A' + 10 );
        else break;

        c = fgetc( stream );
    }

    /* Ungetting EOF is harmless in our implementation. */
    ungetc( c, stream );
    return overflow ? 1 : 0;
}

/* A version of strtoul which works on a stream, and is used internally
 * by both strtoul and fscanf.  Returns 0 on success (including overflow), 
 * and 1 on malformed input, in which case only initial whitespace will 
 * have been consumed and *val_ptr not written to.. */
static
fstrtoul( val_ptr, stream, base ) {
    extern errno;

    /* C90 says that a negative sign are accepted on an unsigned number.  
     * "-1" parses as a very big positive number, namely INT_MAX. */
    auto sgn = skip_ws_sgn( stream );
    auto rv = strtoi32( val_ptr, stream, base );
    if ( rv == 2 ) {
        if (sgn) ungetc( sgn, stream );
        return 1;
    }

    if ( rv == 1 ) {
        *val_ptr = 0xFFFFFFFF; /* ULONG_MAX */
        errno = 34; /* ERANGE */
    }
    else if ( sgn == '-' )
        *val_ptr = -*val_ptr;
    return 0; 
}

/* A version of strtol which works on a stream, and is used internally
 * by both strtol and fscanf.  Returns 0 on success (including overflow), 
 * and 1 on malformed input, in which case only initial whitespace will 
 * have been consumed and *val_ptr not written to.. */
static
fstrtol( val_ptr, stream, base ) {
    extern errno;

    auto sgn = skip_ws_sgn( stream );
    auto rv = strtoi32( val_ptr, stream, base );
    if ( rv == 2 ) {
        if (sgn) ungetc( sgn, stream );
        return 1;
    }

    if ( sgn == '-' ) {
        /* This test is really if ( rw == 1 || *val_ptr < INT_MIN ) where 
	 * *val_ptr should be an unsigned int, but the stage-4 compiler
	 * doesn't support unsigned ints, so we use bit operations instead. */
        if ( rv == 1 || *val_ptr & 0x80000000 && *val_ptr != 0x80000000 ) {
            *val_ptr = 0x80000000; /* INT_MIN */
            errno = 34; /* ERANGE */
        }
        else *val_ptr = -*val_ptr;
    }
    else if ( rv == 1 || *val_ptr & 0x80000000 ) {
        *val_ptr = 0x7FFFFFFF; /* INT_MAX */
        errno = 34; /* ERANGE */
    }
    return 0;
}

/* The C library strtoul() per C90 7.10.1.6 */
strtoul( nptr, endptr, base ) {
    auto n = 0;
    auto stream = __fopenstr( nptr, 32 ); /* 32 == a plausible max length */
    auto rv = fstrtoul( &n, stream, base );
    if ( endptr ) *endptr = rv ? nptr : stream[2]; /* stream->bufp */
    fclose(stream);
    return n;           
}

/* The C library strtol() per C90 7.10.1.5 */
strtol( nptr, endptr, base ) {
    auto n = 0;
    auto stream = __fopenstr( nptr, 32 ); /* 32 == a plausible max length */
    auto rv = fstrtol( &n, stream, base );
    if ( endptr ) *endptr = rv ? nptr : stream[2]; /* stream->bufp */
    fclose(stream);
    return n;           
}

/* The C library atol() per C90 7.10.1.3 */
atol( str ) {
    /* C90 allows atol not to set errno as strtol does, but also allows
     * an implementation in terms of strtol which does set errno. */
    return strtol( str, 0, 10 );
}

/* The C library atoi() per C90 7.10.1.2 */
atoi( str ) {
    /* C90 specifically requires silent truncation of long to int.  This does
     * not change in later versions of the standard.  If the string was a 
     * long enough value that it overflows a long, the return value will be
     * LONG_MAX or LONG_MIN, and errno will be set accordingly (though atoi,
     * like atol, is allowed not to set errno).  But if the value is long
     * enough to overflow an int, but not to overflow a long, the value is 
     * silently truncated to an int; the standard does not allow INT_MAX or
     * INT_MIN to be returned (unless this happens to be the result of the 
     * truncation), and errno must not be set.  This seems like a deficiency
     * in the standard, though it is of no consequence to us here as,
     * effectively, sizeof(int) == sizeof(long) for us here. */
    return strtol( str, 0, 10 );
}

/* Utility used in vfscanf to skip whitespace */
static
fskipws( stream ) {
    while (1) {
        auto s = fgetc( stream );
        if ( s == -1 ) return s;
        else if ( !isspace(s) ) { ungetc( s, stream ); return s; }
    }
}

/* The C library vfscanf(), which was added in C99 7.19.6.9
 * See C90 7.9.6.2 for documentation on fscanf() */
vfscanf( stream, fmt, ap ) {
    auto items = 0, c, s, i;
    while ( c = rchar(fmt++, 0) ) {
        /* White-space characters mean to read up to the first non-white-space
         * character, which remains unread.  There is no requirement that it
         * must read any whitespace. */
        if ( isspace(c) ) {
            if ( fskipws(stream) == -1 )
                return items;
            continue;
        }

        /* Non-whitespace characters other than '%' mean to read that specific
         * chracter. */
        if ( c != '%' ) {
            s = fgetc( stream );
            if ( s == -1 ) return items;
            else if ( s != c ) { ungetc( s, stream ); return items; }
            continue;
        }

        c = rchar(fmt++, 0);

        /* In principle the '%' may be followed by an assignment-suppressing
	 * flag '*', a maximum width field, and a conversion modifier (e.g.
	 * 'l').  We support none of these, meaning the next character must be
	 * the * conversion specifier ... 
         *
         * C90 supports 'd', 'i', 'o', 'u', 'x', 'X', 'c', 's', all of which
         * are supported here; 'f', 'e', 'E', 'g', 'G', all of which parse
         * floating point types which we do not currently support; and the 
         * miscellaneous specifiers '[', 'p' and 'n' which are also not
         * supported. */

        /* Except for %[, %c and %n, we skip any white-space */
        if ( c != '[' && c != 'c' && c != 'n' )
            if ( fskipws(stream) == -1 )
                return items;

        /* %d reads a signed decimal integer */
        if ( c == 'd' ) {
            ap += 4;
            if ( fstrtol( *ap, stream, 10 ) ) return items;
            ++items;
        }
        /* %i reads a signed integer in a base inferred from its prefix */
        else if ( c == 'i' ) {
            ap += 4;
            if ( fstrtol( *ap, stream, 0 ) ) return items;
            ++items;
        }
        /* %o reads an unsigned octal integer */
        else if ( c == 'o' ) {
            ap += 4;
            if ( fstrtoul( *ap, stream, 8 ) ) return items;
            ++items;
        }
        /* %u reads an unsigned decimal integer */
        else if ( c == 'u' ) {
            ap += 4;
            if ( fstrtoul( *ap, stream, 10 ) ) return items;
            ++items;
        }
        /* %x and %X are synonyms which read an unsigned hexadecimal integer.
         * The duplication is to match the behaviour of printf(), where %x
         * prints in lower case while %X prints in upper case. */
        else if ( c == 'x' || c == 'X' ) {
            ap += 4;
            if ( fstrtoul( *ap, stream, 16 ) ) return items;
            ++items;
        }
        /* %c reads a single character */
        else if ( c == 'c' ) {
            s = fgetc( stream );
            /* This can be EOF because we don't call fskipws() for %c */
            if ( s == -1 ) return items;
            ap += 4;
            **ap = s;
            ++items;
        }
        /* %s reads a string of non-white-space characters */
        else if ( c == 's' ) {
            ap += 4;
            i = 0;  /* This will be an index into the *ap string */
            /* We can increment items now as we know we will fetch at least
             * one valid character, as leading white-space and EOF have been
             * handled above by fskipws(). */
            ++items;
            while (1) {
                s = fgetc( stream );
                if ( s == -1 || isspace(s) ) { ungetc( s, stream ); break; }
                lchar( *ap, i++, s );
            }
            /* Null-terminate the string */
            lchar( *ap, i, 0 );
            if ( s == -1 ) return items;
        }
        /* Either %% or an unsupported format specifier.  Anything other than
         * exactly %%, including unknown format specifiers or %% specified
         * with a flag, width or modifier, invokes undefined behaviour.  We
         * define this to require the specifier character, e.g. the
         * final '%', to be present literally in the input stream.  This
         * matches the required behaviour for '%%'. */
        else {
            s = fgetc( stream );
            if ( s == -1 ) return items;
            else if ( s != c ) { ungetc( s, stream ); return items; }
        }
    }
    return items;
}

/* The C library scanf() per C90 7.9.6.4 */
scanf( fmt ) {
    return vfscanf( stdin, fmt, &fmt );
}

/* The C library fscanf() per C90 7.9.6.2 */
fscanf( stream, fmt ) {
    return vfscanf( stream, fmt, &fmt );
}

/* The C library sscanf() */
sscanf( str, fmt ) {
    auto stream = __fopenstr( str, strlen(str) );
    auto items = vfscanf( stream, fmt, &fmt );
    fclose(stream);
    return items;
}
