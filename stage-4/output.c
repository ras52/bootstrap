/* output.c  --  output stdio functions
 *
 * Copyright (C) 2013, 2014, 2015, 2016, 2020 
 * Richard Smith <richard@ex-parrot.com> 
 * All rights reserved.
 */

/* Buffers for the output streams.  Note, because we're being compiled by the
 * stage 4 compiler, arrays are int arrays, meaning these are 4*32 = 128 bytes
 * long: hence the 128 bufsz parameter in the stream definitions. */
static __buf1[32];
static __buf2[32];

/*                     0  1     2       3       4       5    6     7     8 9
 * struct FILE       { fd bufsz bufp    buffer  bufend  mode bmode flags pos }*/
static __file1[10] = { 1, 128,  __buf1, __buf1, __buf1, 2,   1,    0,    0,0 };
static __file2[10] = { 2, 128,  __buf2, __buf2, __buf2, 2,   3,    0,    0,0 };
/* BUFEND is the end of data read into the buffer from the file. 
 * MODE is as per the second argument to open(2).  
 * BMODE is an _IO?BF flag
 * FLAGS is a bit field: 
 *   0x01 indicates that the buffer in stream[3] should be freed on close;
 *   0x02 indicates that the stream object should be freed on close;
 *   0x04 is the EOF flag on the stream;
 *   0x08 is the error indicator on the stream. 
 * POS is a 64 bit counter */

/* The stdio objects themselves.  
 * We can't just use the arrays themselves because we need to force make 
 * lvalue versions of them, and arrays are only rvalues. */
stdout = __file1;
stderr = __file2;


/* An array of file* objects, indexed by file descriptor. */
static __files = 0;
/* Count of entries in the __files array */
static __filec = 0;


/* C90 section 7.9.5:  File access functions
 *
 * Status: all function implemented */

/* The C library fflush() per C90 7.9.5.2 */
fflush( stream ) {
    /* Calling flush on an input stream is undefined behaviour.  We define
     * it to be a no-op. */
    if ( stream[5] ) {
        auto count = stream[2] - stream[3];
        if ( count ) {
            if ( write( stream[0], stream[3], count ) != count ) {
                stream[7] |= 8; /* Set error indicator */
                return -1;
            }
            stream[2] = stream[3];
        }
    }
    return 0;
}

/* The C library fclose() per C90 7.9.5.1 */
fclose( stream ) {
    auto rv = -1;
    if ( stream[0] != -1 ) {
        fflush( stream );
        rv = close( stream[0] );
        if ( stream[0] < __filec )
            __files[ stream[0] ] = 0;
    }
    /* Free any buffers associated with the stream */
    if ( stream[7] & 1 ) free( stream[3] );
    if ( stream[7] & 2 ) free( stream );
    
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

/* The C library freopen() per 7.9.5.4 */
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

    clearerr( stream );

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
    auto stream = malloc(40); /* sizeof(struct FILE) */
    stream[0] = -1;  /* an invalid file descriptor */
    stream[1] = len;
    stream[2] = stream[3] = str;
    stream[4] = str + len;
    stream[5] = 2;   /* O_RDWR */
    stream[6] = 1;   /* _IOFBF */
    stream[7] = 2;   /* free the stream but not the buffer on close */
    stream[8] = stream[9] = 0;
    return stream;
}

/* The C library setvbuf() per C90 7.9.5.6 */
setvbuf( stream, buf, mode, size ) {
    if ( buf && (stream[7] & 1) ) {
        /* Free old buffer */
        free( stream[3] );
        stream[7] &= ~1;  /* We no longer manage the memory for the buffer. */
    }

    /* Without a buffer passed, this is a resize request. */
    else {
        if ( stream[7] & 1 ) buf = realloc( stream[3], size );
        else buf = malloc(size);
        if (!buf) return -1;
        stream[7] |= 1;   /* We do now manage the memory for the buffer. */
    }
        
    stream[1] = size;
    stream[2] = stream[3] = stream[4] = buf;
    stream[6] = mode;
    return 0;
}

/* The C library setbuf() per C90 7.9.5.5 */
setbuf( stream, buf ) {
    /* BUFSIZ == 4096 in our implementation. */
    setvbuf( stream, buf, buf ? 1 : 3, 4096 ); /* 1 == _IOFBF; 3 = _IONBF */
}

/* The C library fopen() per C90 7.9.5.3 */
fopen( filename, mode ) {
    auto stream = malloc(40); /* sizeof(struct FILE) */
    if (!stream) return 0;
    memset(stream, 0, 40);    /* sizeof(struct FILE) */
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


/* C90 section 7.9.10:  Error-handling functions
 *
 * Status: incomplete, as perror is missing as it relies on strerror which
 * is also not implemented */

/* The C library fopen() per C90 7.9.10.1 */
clearerr( stream ) {
    /* Clear the EOF flag and error indicator */
    stream[7] &= ~0xC;
}

/* The C library feof() per C90 7.9.10.2 */
feof( stream ) {
    return stream[7] & 4 ? 1 : 0;
}

/* The C library feof() per C90 7.9.10.3 */
ferror( stream ) {
    return stream[7] & 8;
}

/* The C library fgetpos() */
fgetpos( stream, pos ) {
  /* We define fpos_t as a struct { int hi, low; }; */
  pos[0] = stream[8];
  pos[1] = stream[9];
}


/* C90 section 7.9.8:  Direct input/output functions
 *
 * Status: all function implemented 
 * Implementation split between here and input.c */

/* __fputsn() is the only function to ever write data to the stream
 * (excepting ungetc which writes to the pushback slot).  Everything 
 * else is implemented on top of theis. */
static
__fputsn( ptr, len, stream ) {
    auto written = 0, flag = 0;

    while ( len ) {
        auto space = stream[1] - (stream[2] - stream[3]);

        /* If the buffer is partially full, and after filling and emptying it,
         * it still won't have space for the string, then flush the buffer
         * immediately. */
        if ( stream[2] != stream[3] && len >= space + stream[1] ) {
            if ( fflush( stream ) == -1 ) {
                flag = 8;  /* error flag */
                break; 
            }
        }

        /* If the buffer is empty and not big enough to hold the string,
         * then write the string out directly. */
        else if ( stream[2] == stream[3] && len >= stream[1] ) {
            auto n = write( stream[0], ptr, len );
            if ( n != len ) {
                if ( n != -1 ) written += n;
                flag = 8;  /* error flag */
                break;
            }
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
            if ( chunk == space && fflush( stream ) == -1 ) {
                flag = 8;  /* error flag */
                break;
            }
        }
    }

    /* Set error indicator, if required */
    stream[7] |= flag;

    /* Set the file position indicator */
    __add64( &stream[8], &stream[9], 0, written );

    return written;
}

/* The C library fwrite() per C90 7.9.8.2 */
fwrite( buf, size, nmem, stream ) {
  return __fputsn( buf, size * nmem, stream ) / size; 
}


/* C90 section 7.9.7:  Character input/output functions
 *
 * Status: all function implemented
 * Implementation split between here and input.c */

/* The C library fputs() per C90 7.9.7.4 */
fputs( s, stream ) {
    auto written = __fputsn( s, strlen(s), stream );
    /* XXX This is wrong -- see comment in vsfprintf */
    if ( stream[6] > 1 ) fflush(stream);
    return written;
}

/* The B library putstr() -- unlike C's puts, it doesn't add a '\n' */
putstr( s ) {
    return fputs( s, stdout );
}

/* The C library puts() per C90 7.9.7.10 */
puts( s ) {
    return putstr(s) + putchar('\n');
}

/* The C library fputc() per C90 7.9.7.3 */
fputc( c, stream ) {
    return __fputsn( &c, 1, stream ) == 1 ? c : -1;
}

/* The C library putc() per C90 7.9.7.8 */
putc( s, stream ) {
    return fputc( s, stream );
}

/* The C library putchar() per C90 7.9.7.9 */
putchar( c ) {
    return fputc( c, stdout );
}


/* C90 section 7.9.6:  Formatted input/output functions
 *
 * Status: all function implemented
 * Implementation split between here and input.c */

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

/* The C library vfprintf() per C90 7.9.6.7.  
 * See C90 7.9.6.1 for documentation on fprintf() */
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

        /* We must have a conversion specifier ... */
        c = rchar(fmt++, 0);

        /* First after the % are zero or more flag characters, in any order.
         * C90 supports '-', '+', ' ', '#' and '0'.  We only support '0'. */
        if ( c == '0' ) {
            padc = c;
            c = rchar(fmt++, 0);
        }  

        /* Second after the % is an optional field width, which may be either 
         * an integer or a '*'.  We only support integer field widths. */
        if ( isdigit(c) ) {
            width = strtoul(--fmt, &fmt, 10);
            c = rchar(fmt++, 0);
        }

	/* Third after the % comes an optional precision which is prefixed
         * with a '.' to delimit it from the field width.  Like the field
         * width, it may be either an integer or a '*'.  We support neither.
         *
	 * Fourth after the % is an optional length modifier character.  
	 * C90 supports 'h', 'l' and 'L'.  We support none of these, but as an
	 * extention, we support 'M' to print a multicharacter constant. 
	 * This does not conflict with any length modifier or format specifier
	 * in C99, C11, C17 or the current draft C20 spec (N2583).
         *
         * Fifth after the % is a mandatory conversion specifier; C90 supports
         * 'd', 'i', 'o', 'u', 'x', 'X', 'c', 's', 'p', 'n' and '%', all of 
         * which are supported here; and 'f', 'e', 'E', 'g', 'G', all of which
         * print floating point types which we do not currently support. */
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
            /* An other %MX forms are unsupported, so print it literally. */
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
        /* %d or %i are synonyms which print a signed decimal integer.
         * The duplication is to match the behaviour of scanf(), where
         * %i reads a signed integer, taking its base from its prefix, default
         * to decimal, while %d always reads a decimal signed integer.  */
        else if (c == 'd' || c == 'i') {
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
        /* %o prints an unsigned octal integer  */
        else if (c == 'o') {
            ap += 4;
            if ( ( n = fmt_uint( stream, *ap, 8, width, padc, 'a' ) ) == -1 )
                return -1;
            written += n;
        }
        /* %u prints an unsigned decimal integer  */
        else if (c == 'u') {
            ap += 4;
            if ( ( n = fmt_uint( stream, *ap, 10, width, padc, 'a' ) ) == -1 )
                return -1;
            written += n;
        }
        /* %x prints an unsigned hexdecimal integer in lower case */
        else if (c == 'x') {
            ap += 4;
            if ( ( n = fmt_uint( stream, *ap, 16, width, padc, 'a' ) ) == -1 )
                return -1;
            written += n;
        }
        /* %X prints an unsigned hexdecimal integer in upper case */
        else if (c == 'X') {
            ap += 4;
            if ( ( n = fmt_uint( stream, *ap, 16, width, padc, 'A' ) ) == -1 )
                return -1;
            written += n;
        }
        /* %p prints a unsigned void* pointer in an implementation defined
         * way, which we interpret to mean in hexadecimal with an '0x' prefix,
         * or as '(null)'. */
        else if (c == 'p') {
            ap += 4;
            if (!*ap) {
                if ( fputs("(null)", stream) == -1 )
                    return -1;
                written += 6;
            } else {
                if ( fputs("0x", stream) == -1 )
                    return -1;
                if ( ( n = fmt_uint( stream, *ap, 16, width, padc, 'a' ) )  
                             == -1 )
                    return -1;
                written += 2 + n;
            }
        }
        /* %n does not print anything.  The corresponding argument is an
         * int* to which the number of characters written so far is written. */
        else if (c == 'n') {
            ap += 4;
            **ap = written;
        }
        /* Either %% or an unsupported format specifier.  Anything other than
         * exactly %%, including unknown format specifiers or %% specified
	 * with a flag, width, precision or modifier, invokes undefined
	 * behaviour.  We define this to just print out the specifier
	 * character, e.g. the final '%'.  This matches the required behaviour
	 * for '%%'. */
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

/* The C library printf() per C90 7.9.6.3 */
printf( fmt ) {
    return vfprintf( stdout, fmt, &fmt );
}

/* The C library fprintf() per C90 7.9.6.1 */
fprintf( stream, fmt ) {
    return vfprintf( stream, fmt, &fmt );
}

/* The C library vprintf() per C90 7.9.6.8 */
vprintf( fmt, ap ) {
    return vfprintf( stdout, fmt, ap );
}

/* The C library vsnprintf(), which was added in C99 7.19.6.12 */
vsnprintf( buf, len, fmt, ap ) {
    auto stream = __fopenstr( buf, len );
    auto len = vfprintf( stream, fmt, ap ); 
    fputc( 0, stream );
    fclose(stream);
    return len;
}

/* The C library snprintf(), which was added in C99 7.19.6.5 */
snprintf( buf, len, fmt ) {
    return vsnprintf( buf, len, fmt, &fmt );
}

/* The C library sprintf() per C90 7.9.6.5 */
sprintf( buf, fmt ) {
    /* Because we're only doing output, bufend is never used, so it
     * doesn't matter if it compares incorrectly to bufp because 
     * comparisons in the stage-4 compiler use signed arithmetic. */
    return vsnprintf( buf, 0x7FFFFFFF, fmt, &fmt ); /* INT_MAX */
}

/* The C library vsprintf() per C90 7.9.6.9 */
vsprintf( buf, fmt, ap ) {
    return vsnprintf( buf, 0x7FFFFFFF, fmt, ap );
}


