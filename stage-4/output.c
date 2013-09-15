/* output.c  --  output stdio functions
 *
 * Copyright (C) 2013 Richard Smith <richard@ex-parrot.com> 
 * All rights reserved.
 */

/* Buffers for the output streams */
static __buf1[32];
static __buf2[32];

/*                    0     1       2       3       4       5    6     7
 * struct FILE      { fd    bufsz   bufp    buffer  bufend  mode bmode free } */
static __file1[8] = { 1,    128,    __buf1, __buf1, __buf1, 2,   0,    0    };
static __file2[8] = { 2,    128,    __buf2, __buf2, __buf2, 2,   2,    0    };
/* MODE is as per the second argument to open(2).  
 * BMODE is an _IO?BF flag */

/* The stdio objects themselves.  
 * We can't just use the arrays themselves because we need to force make 
 * lvalue versions of them, and arrays are only rvalues. */
stdout = __file1;
stderr = __file2;


/* The C library fflush() */
fflush( stream ) {
    auto count = stream[2] - stream[3];
    if ( count ) {
        if ( write( stream[0], stream[3], count ) != count )
            return -1;
        stream[2] = stream[3];
    }
    return 0;
}

/* The C library fclose() */
fclose( stream ) {
    auto rv;
    /* Only flush output streams */
    if ( stream[5] ) fflush( stream );
    rv = close( stream[0] );
    if ( stream[7] ) {
        free( stream[3] );
        free( stream );
    }
    return rv;
}

/* The C library freopen() */
freopen( filename, mode, stream ) {
    extern errno;
    /* 0 == O_RDONLY */
    auto fmode = 0, fd;

    /* stream->fd == -1 signifies a closed stream. */
    if ( stream[0] != -1 && fclose( stream ) == -1 ) 
        /* The standard does not state explicitly that errno is cleared, 
         * but it seems implied when it says that failure is ignored. */
        errno = 0;

    /* O_WRONLY=1 | O_CREAT=0x40 | O_TRUNC=0x200 */
    if ( rchar(mode, 0) == 'w' ) fmode |= 0x241;
    fd = open( filename, fmode, 0644 );
    if ( fd == -1 ) return 0;

    if ( fd != stream[0] ) {
        if ( dup2(fd, stream[0]) == -1 ) {
            /* This is nice to have, but not a requirement */
            stream[0] = fd;
            errno = 0;
        } else if ( close(fd) == -1 )
            return 0;
    }

    stream[2] = stream[4] = stream[3];
    stream[5] = fmode & 3;

    return stream;
}

fopen( filename, mode ) {
    auto stream = malloc(32); /* sizeof(struct FILE) */
    stream[0] = -1; /* an invalid file descriptor */
    stream[1] = 128;
    stream[2] = stream[3] = stream[4] = malloc( stream[1] );
    stream[5] = 0; /* O_RDONLY */
    stream[6] = 0; /* _IOFBF */
    stream[7] = 1;

    if ( freopen( filename, mode, stream ) )
        return stream;

    /* It failed, so clean up. */
    free( stream[3] );
    free( stream );
    return 0;
}

/* Implementation detail _fputsn() -- TODO declare this static */
static
_fputsn( ptr, len, stream ) {
    auto written = 0;

    while ( len ) {
        auto space = stream[1] - (stream[2] - stream[3]);

        /* If the buffer is partially full, and after filling and emptying it,
         * it still won't have space for the string, then flush the buffer
         * immediately. */
        if ( stream[2] != stream[3] && len >= space + stream[1] ) {
            if ( fflush( stream ) == -1 )
                return written;
        }

        /* If the buffer is empty and not big enough to hold the string,
         * then write the string out directly. */
        else if ( stream[2] == stream[3] && len >= stream[1] ) {
            auto n = write( stream[0], ptr, len );
            if ( n == -1 ) return written;
            written += n;
            if ( n != len ) return written;
            len = 0;
        }

        /* Otherwise append as much as possible to the buffer. */
        else {
            auto chunk = space < len ? space : len;
            memcpy( stream[2], ptr, chunk );
            stream[2] += chunk;
            ptr += chunk;
            len -= chunk;
            written += chunk;

            /* Flush the buffer now if it's full. */
            if ( chunk == space && fflush( stream ) == -1 )
                return written;
        }
    }

    return written;
}

/* The C library fwrite() */
fwrite( buf, size, nmem, stream ) {
  return _fputsn( buf, size * nmem, stream ) / size; 
}

/* The C library fputs() */
fputs( s, stream ) {
    return _fputsn( s, strlen(s), stream );
}

/* The B library putstr() -- unlike C's puts, it doesn't add a '\n' */
putstr( s ) {
    return fputs( s, stdout );
}

/* The C library puts() */
puts( s ) {
    return putstr(s) + putchar('\n');
}

/* The C library fputc() */
fputc( c, stream ) {
    return _fputsn( &c, 1, stream ) == 1 ? c : -1;
}

/* The C library putc() */
putc( s, stream ) {
    return fputc( s, stream );
}

/* The C library putchar() */
putchar( c ) {
    return fputc( c, stdout );
}

/* The C library vfprintf() */
vfprintf( stream, fmt, ap ) {
    auto written = 0, c, n;
    while ( c = rchar(fmt++, 0) ) {
        /* Pass normal characters straight through.
         * TODO:  We could strchr for '%' and push through the whole lot
         * with fputs */
        if (c != '%') {
            if ( fputc(c, stream) == -1 )
                return -1;
            ++written;
            continue;
        }

        c = rchar(fmt++, 0);

        /* This is our extension:  %Mc prints a multicharacter constant */
        if (c == 'M') {
            c = rchar(fmt++, 0);
            if (c == 'c') {
                ap += 4;
                n = strnlen(ap, 4);
                if ( _fputsn(ap, n, stream) != n )
                    return -1; 
                written += n;
            }
            /* %MX for any other X prints a literal 'M', and backtracks for 
             * the 'X' */
            else {
                c = rchar(--fmt, 0);
                if ( fputc(c, stream) == -1 )
                    return -1;
                ++written;
            }
        }
        /* %c prints a normal character */
        else if (c == 'c') {
            ap += 4;
            if ( fputc(*ap, stream) == -1 )
                return -1;
            ++written;
        }
        /* %s prints a null-terminated character string */
        else if (c == 's') {
            ap += 4;
            if ( ( n = fputs(*ap, stream) ) == -1 )
                return -1; 
            written += n;
        }
        /* %d prints a decimal number  */
        else if (c == 'd') {
            ap += 4;
            if (*ap == 0) { 
                if ( fputc('0', stream) == -1 )
                    return -1;
                ++written;
            }
            else {
                /* We don't currently support character arrays. */
                auto buffer[4] = {0,0,0,0},  b = 14,  i = *ap;

                if (i < 0) {
                    if ( fputc('-', stream) == -1 )
                        return -1;
                    ++written;
                    i = -i;
                }

                while (i) {
                    lchar(buffer, b--, i % 10 + '0');
                    i /= 10;
                }

                if ( ( n = fputs(buffer+b+1, stream) ) == -1 )
                    return -1; 
                written += n;
            }
        }
        /* %x prints a hexdecimal number  */
        else if (c == 'x') {
            ap += 4;
            if (*ap == 0) { 
                if ( fputc('0', stream) == -1 )
                    return -1;
                ++written;
            }
            else {
                /* We don't currently support character arrays. */
                auto buffer[4] = {0,0,0,0},  b = 14,  i = *ap;

                while (i) {
                    auto x = i % 16;
                    x = x >= 10 ? x + 'a' - 10 : x + '0';
                    lchar(buffer, b--, x);
                    i /= 16;
                }

                if ( ( n = fputs(buffer+b+1, stream) ) == -1 )
                    return -1; 
                written += n;
            }
        }
        /* %X for any other X prints a literal 'X' */
        else {
            if ( fputc(c, stream) == -1 )
                return -1;
            ++written;
        }
    }

    /* XXX  This isn't really right: we should handle line buffering 
     * distinctly from unbuffered, and also do similarly in the other ouput
     * functions. */
    if ( stream[6] ) fflush(stream);

    return written;
}

/* The C library printf() */
printf( fmt ) {
    return vfprintf( stdout, fmt, &fmt );
}

/* The C library fprintf() */
fprintf( stream, fmt ) {
    return vfprintf( stream, fmt, &fmt );
}
