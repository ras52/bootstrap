/* cmp.c  --  an implementation of the POSIX cmp(1) utility
 *  
 * Copyright (C) 2015 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */ 

/* The Makefile sticks --compatibility=4 on the command line.  Remove it. */
#pragma RBC compatibility 5 

#include <stdio.h> 

usage() {
    cli_error("Usage: cmp [-s] file1 file2\n");
}

main(argc, argv) 
    int argc;
    char **argv;
{
    char *na, *nb;
    FILE *a, *b;
    int opt_s = 0, opt_l = 0;
    int i = 1, bytes = 0, lines = 1, status = 0;

    if (argc < 3) usage();

    /* The -s option suppressed output. */
    if ( strcmp( argv[i], "-s" ) == 0 )
        opt_s = 1, ++i;
    /* The -l option prints a byte-by-byte comparison in a whacky format. */
    else if ( strcmp( argv[i], "-l" ) == 0 )
        opt_l = 1, ++i;

    na = argv[i++];
    if ( strcmp( na, "-" ) ) {
        a = fopen( na, "r" );
        if (!a) cli_error( "cmp: unable to open file '%s'\n", na );
    }
    else a = stdin;

    nb = argv[i++];
    if ( strcmp( nb, "-" ) ) {
        b = fopen( nb, "r" );
        if (!b) cli_error( "cmp: unable to open file '%s'\n", nb );
    }
    else if ( a == stdin ) 
        cli_error( "cmp: cannot read standard input twice\n" );
    else b = stdin;

    if (i != argc) usage();

    while (1) {
        int ca = fgetc(a), cb = fgetc(b);
        ++bytes;

        /* The format of these messages, and whether they go to stdout or 
         * stderr, is prescribed by POSIX. */
        if ( ca == EOF && cb == EOF ) 
            break;
        else if ( ca == EOF || cb == EOF ) {
            if (!opt_s) {
                fflush(stdout);
                fprintf(stderr, "cmp: EOF on %s\n", ca == EOF ? na : nb);
            }
            status = 1; 
            break;
        }
        else if ( ca != cb ) {
            status = 1;
            if (opt_l)
                printf( "%d %o %o\n", bytes, ca, cb );
            else {
                if (!opt_s) 
                    printf( "%s %s differ: char %d, line %d\n", 
                            na, nb, bytes, lines );
                break;
            }
        }
        else if ( ca == '\n' ) 
            ++lines;
    }

    return status;
}
