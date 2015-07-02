/* output.c  --  output stdio functions
 *
 * Copyright (C) 2013, 2014, 2015 Richard Smith <richard@ex-parrot.com> 
 * All rights reserved.
 */

/* Buffers for the output streams */
static __buf1[32];
static __buf2[32];

/*                    0     1       2       3       4       5    6     7
 * struct FILE      { fd    bufsz   bufp    buffer  bufend  mode bmode free } */
static __file1[8] = { 1,    128,    __buf1, __buf1, __buf1, 2,   1,    0    };
static __file2[8] = { 2,    128,    __buf2, __buf2, __buf2, 2,   3,    0    };
/* MODE is as per the second argument to open(2).  
 * BMODE is an _IO?BF flag */

/* The stdio objects themselves.  
 * We can't just use the arrays themselves because we need to force make 
 * lvalue versions of them, and arrays are only rvalues. */
stdout = __file1;
stderr = __file2;


/* An array of file* objects, indexed by file descriptor. */
static __files = 0;
/* Count of entries in the __files array */
static __filec = 0;

/* The C library fflush() */
fflush( stream ) {
    /* Calling flush on an input stream is undefined behaviour.  We define
     * it to be a no-op. */
    if ( stream[5] ) {
        auto count = stream[2] - stream[3];
        if ( count ) {
            if ( write( stream[0], stream[3], count ) != count )
                return -1;
            stream[2] = stream[3];
        }
    }
    return 0;
}

/* The C library fclose() */
fclose( stream ) {
    auto rv = -1;
    if ( stream[0] != -1 ) {
        fflush( stream );
        rv = close( stream[0] );
        if ( stream[0] < __filec )
            __files[ stream[0] ] = 0;
    }
    /* Free any buffers associated with the stream */
    if ( stream[7] ) {
        free( stream[3] );
        free( stream );
    }
    return rv;
}

/* Flush standard output streams; gets registered with atexit by crt0.o */
__io_flush() {
    extern stdin;

    auto i = 0;
    while ( i < __filec ) {
        auto f = __files[i];
        if (f) fclose(f);
        ++i;
    }

    fclose( stdin );
    fclose( stdout );
    fclose( stderr );
}

/* The C library freopen() */
freopen( filename, mode, stream ) {
    extern errno;
    /* 0 == O_RDONLY */
    auto fmode = 0, fd;

    /* stream->fd == -1 signifies a closed stream.  This is true if we're
     * called by fopen(). */
    if ( stream[0] != -1 ) {
        /* Don't call fclose because we want to retain the buffers. */
        fflush( stream );
        close( stream[0] );

        /* The standard does not state explicitly that errno is cleared, 
         * but it seems implied when it says that failure is ignored. */
        errno = 0;
    }

    /* O_WRONLY=1 | O_CREAT=0x40 | O_TRUNC=0x200 */
    if ( rchar(mode, 0) == 'w' ) fmode |= 0x241;
    fd = open( filename, fmode, 0644 );
    if ( fd == -1 ) return 0;

    if ( stream[0] == -1 )
        stream[0] = fd;
    else if ( fd != stream[0] ) {
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

/* A function to get a FILE* interface to a simple string. */
__fopenstr( str, len ) {
    auto stream = malloc(32); /* sizeof(struct FILE) */
    stream[0] = -1;  /* an invalid file descriptor */
    stream[1] = len;
    stream[2] = stream[3] = str;
    stream[4] = str + len;
    stream[5] = 2;   /* O_RDWR */
    stream[6] = 1;   /* _IOFBF */
    stream[7] = 0;   /* do not free() the buffer on close */
    return stream;
}

/* The C library setvbuf() */
setvbuf( stream, buf, mode, size ) {
    if ( buf ) {
        /* Free old buffer */
        if ( stream[7] ) free( stream[3] );
        stream[7] = 0;   /* We no longer manage the memory for the buffer. */
    }

    /* Without a buffer passed, this is a resize request. */
    else {
        if ( stream[7] ) buf = realloc( stream[3], size );
        else buf = malloc( stream[1] );
        if (!buf) return -1;
        stream[7] = 1;   /* We do now manage the memory for the buffer. */
    }
        
    stream[1] = size;
    stream[2] = stream[3] = stream[4] = buf;
    stream[6] = mode;
    return 0;
}

/* The C library setvbuf() */
setbuf( stream, buf ) {
    /* BUFSIZ == 4096 in our implementation. */
    setvbuf( stream, buf, buf ? 1 : 3, 4096 ); /* 1 == _IOFBF; 3 = _IONBF */
}

/* The C library fopen() */
fopen( filename, mode ) {
    auto stream = malloc(32); /* sizeof(struct FILE) */
    if (!stream) return 0;
    memset(stream, 0, 32);    /* sizeof(struct FILE) */
    stream[0] = -1;           /* an invalid file descriptor */

    setvbuf( stream, 0, 1, 128 );  /* 1 = _IOFBF */

    if ( !stream[3] || freopen( filename, mode, stream ) == 0 ) {
        /* It failed, so clean up. */
        free( stream[3] );
        free( stream );
        return 0;
    }

    /* Register the stream so it gets closed on exit */
    if ( stream[0] >= __filec ) {
        __filec = 1 + stream[0];
        __files = realloc( __files, __filec * 4 ); /* 4 == sizeof(FILE*) */
    }
    __files[ stream[0] ] = stream;

    return stream;
}

/* Implementation detail __fputsn() */
static
__fputsn( ptr, len, stream ) {
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
  return __fputsn( buf, size * nmem, stream ) / size; 
}

/* The C library fputs() */
fputs( s, stream ) {
    return __fputsn( s, strlen(s), stream );
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
    return __fputsn( &c, 1, stream ) == 1 ? c : -1;
}

/* The C library putc() */
putc( s, stream ) {
    return fputc( s, stream );
}

/* The C library putchar() */
putchar( c ) {
    return fputc( c, stdout );
}

static
pad( stream, width, n, padc ) {
    auto written = 0;
    if ( width >= 0 && n < width ) {
        auto pad[5];  memset( pad, padc, 16 );
        n = width - n;
        while ( n ) {
            auto m = n > 16 ? 16 : n;
            if ( __fputsn(pad, m, stream) == -1 )
                return -1;
            n -= m; written += m;
        }
    }
    return written;
}

/* Handle the %o, %u and %x options, and return the bytes written or -1. */
static
fmt_uint( stream, n, base, width, padc, xchr ) {
    /* Special case n=0. */
    if (n == 0) {
        auto written;
        if ( ( written = pad( stream, width, 1, padc ) ) == -1 ||
             fputc('0', stream) == -1 )
            return -1;
        return written + 1;
    }
    else {
        /* We don't currently support character arrays.  
         * 15 bytes is long enough even for octal. */
        auto buffer[4] = {0,0,0,0}, blen = 14, boff = blen, written1, written2;

        /* Fill in the buffer with the string in the correct base. */
        while (n) {
            auto x = n % base;
            x = x >= 10 ? x + xchr - 10 : x + '0';
            lchar(buffer, boff--, x);
            n /= base;
        }

        if ( ( written1 = pad( stream, width, blen-boff, padc ) ) == -1 ||
             ( written2 = fputs(buffer+boff+1, stream) ) == -1 )
            return -1; 
        return written1 + written2;
    }
}

/* The C library vfprintf() */
vfprintf( stream, fmt, ap ) {
    auto written = 0, c, n, m;
    while ( c = rchar(fmt++, 0) ) {
        auto width = -1, padc = ' ';

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

        /* Do we have a '0' flag? */
        if ( c == '0' ) {
            padc = c;
            c = rchar(fmt++, 0);
        }  

        /* Do we have a field width? */
        if ( isdigit(c) ) {
            width = strtoul(--fmt, &fmt, 10);
            c = rchar(fmt++, 0);
        }

        /* This is our extension:  %Mc prints a multicharacter constant */
        if (c == 'M') {
            c = rchar(fmt++, 0);
            if (c == 'c') {
                ap += 4;
                n = strnlen(ap, 4);
                if ( ( m = pad( stream, width, n, ' ' ) ) == -1 ||
                     __fputsn(ap, n, stream) != n )
                    return -1; 
                written += m + n;
            }
            /* An other %MX forms are supported, so print it literally. */
            else {
                if ( fputc('%', stream) == -1 )
                    return -1;
                ++written;
                fmt -= 2;
            }
        }
        /* %c prints a normal character */
        else if (c == 'c') {
            ap += 4;
            if ( ( m = pad( stream, width, 1, ' ' ) ) == -1 ||
                 fputc(*ap, stream) == -1 )
                return -1;
            written += m + 1;
        }
        /* %s prints a null-terminated character string */
        else if (c == 's') {
            ap += 4;
            if ( ( m = pad( stream, width, strlen(*ap), ' ' ) ) == -1 ||
                 ( n = fputs(*ap, stream) ) == -1 )
                return -1; 
            written += m + n;
        }
        /* %d prints a decimal number  */
        else if (c == 'd') {
            ap += 4;
            if (*ap == 0) { 
                if ( ( m = pad( stream, width, 1, padc ) ) == -1 ||
                     fputc('0', stream) == -1 )
                    return -1;
                written += m + 1;
            }
            else {
                /* We don't currently support character arrays. */
                auto buffer[4] = {0,0,0,0},  b = 14,  i = *ap, neg = 0;

                if (i < 0) { neg = 1; i = -i; }

                while (i) {
                    lchar(buffer, b--, i % 10 + '0');
                    i /= 10;
                }

                if ( padc != '0' && 
                         ( m = pad( stream, width, 14-b+neg, padc ) ) == -1 ||
                     neg && fputc('-', stream) == -1 ||
                     padc == '0' && 
                         ( m = pad( stream, width, 14-b+neg, padc ) ) == -1 ||
                     ( n = fputs(buffer+b+1, stream) ) == -1 )
                    return -1; 
                written += m + n + neg;
            }
        }
        /* %o prints an unsigned octal number  */
        else if (c == 'o') {
            ap += 4;
            if ( ( n = fmt_uint( stream, *ap, 8, width, padc, 'a' ) ) == -1 )
                return -1;
            written += n;
        }
        /* %u prints an unsigned decimal number  */
        else if (c == 'u') {
            ap += 4;
            if ( ( n = fmt_uint( stream, *ap, 10, width, padc, 'a' ) ) == -1 )
                return -1;
            written += n;
        }
        /* %x prints an unsigned hexdecimal number  */
        else if (c == 'x') {
            ap += 4;
            if ( ( n = fmt_uint( stream, *ap, 16, width, padc, 'a' ) ) == -1 )
                return -1;
            written += n;
        }
        /* %X prints an unsigned hexdecimal number in upper case */
        else if (c == 'X') {
            ap += 4;
            if ( ( n = fmt_uint( stream, *ap, 16, width, padc, 'A' ) ) == -1 )
                return -1;
            written += n;
        }
        /* Either %% or an unsupported format specifier: just print it out */
        else {
            if ( fputc('%', stream) == -1 )
                return -1;
            ++written;
            if ( c != '%' && width == -1 ) 
                --fmt;
        }
    }

    /* XXX  This isn't really right: we should handle line buffering 
     * distinctly from unbuffered, and also do similarly in the other ouput
     * functions. */
    if ( stream[6] > 1 ) fflush(stream);

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

/* The C library snprintf() */
snprintf( buf, len, fmt ) {
    auto stream = __fopenstr( buf, len );
    auto len = vfprintf( stream, fmt, &fmt ); 
    fputc( 0, stream );
    fclose(stream);
    return len;
}
