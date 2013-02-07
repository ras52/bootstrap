/* output.c
 *
 * Copyright (C) 2013 Richard Smith <richard@ex-parrot.com> 
 * All rights reserved.
 */

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
    /* Only flush output streams */
    if ( stream[5] ) fflush( stream );
    return close( stream[0] );
}

/* The C library freopen() */
freopen( filename, mode, stream ) {
    auto fmode = 0, fd;

    if ( fclose( stream ) == -1 ) 
        /* The standard does not state explicitly that errno is cleared, 
         * but it seems implied when it says that failure is ignored. */
        errno = 0;

    if ( rchar(mode, 0) == 'w' ) fmode |= 0x41;  /* O_RWONLY | O_CREAT */
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

/* Implementation detail _fputsn() -- TODO declare this static */
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
    return putc( s, stream );
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
        
        if (c == 'c') {
            ap += 4;
            if ( fputc(*ap, stream) == -1 )
                return -1;
            ++written;
        }
        else if (c == 's') {
            ap += 4;
            if ( ( n = fputs(*ap, stream) ) == -1 )
                return -1; 
            written += n;
        }
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
        else {
            if ( fputc(c, stream) == -1 )
                return -1;
            ++written;
        }
    }

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
