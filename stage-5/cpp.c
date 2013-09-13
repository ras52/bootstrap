/* main.c  --  the C preprocessor
 *
 * Copyright (C) 2013 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */

static input_strm;

/* Contains the token most recently read by next() */
static token;

/* Read the next lexical element as a syntax tree node */
next() {
    auto stream = input_strm;
    auto c = skip_white(stream);
    if ( c == -1 )
        token = 0;
    else if ( isidchar1(c) )
        token = get_word(stream, c);
    else if ( isdigit(c) )
        token = get_ppnum(stream, c, 0);
    else if ( c == '.' ) {
        /* A . could be the start of a ppnumber, or a '.' operator  */
        auto c2 = fgetc(stream);
        if ( isdigit(c2) )
            token = get_ppnum(stream, c, c2);
        else {
            ungetc(c2, stream);
            token = new_node( c, 0 );
        }
    }        
    else if ( c == '\'' || c == '"' )
        token = get_qlit(stream, c, 0);
    else 
        token = get_multiop(stream, c);
    return token;
}

init_scan(in_filename) {
    extern stdin;
    freopen( in_filename, "r", stdin );
    set_file( in_filename );
    input_strm = stdin;
    next();
}



static
cli_error(fmt) 
    char *fmt;
{
    extern stderr;
    vfprintf(stderr, fmt, &fmt);
    exit(1);
}

usage() {
    cli_error("Usage: cpp [-o filename.i] filename.c\n");
}

main(argc, argv) 
    int argc;
    char **argv;
{
    char *filename = 0, *outname = 0;
    int i = 0;

    while ( ++i < argc ) {
        if ( strcmp( argv[i], "-o" ) == 0 ) {
            if ( ++i == argc ) usage();
            outname = argv[i];
        }

        else if ( strcmp( argv[i], "--help" ) == 0 ) 
            usage();

        else if ( argv[i][0] == '-' )
            cli_error("cpp: unknown option: %s\n", argv[i]);

        else {
            if ( filename ) cli_error(
                "cpp: multiple input files specified: '%s' and '%s'\n",
                filename, argv[i] );
            filename = argv[i];
        }
    }

    if ( !filename )
        cli_error("cpp: no input file specified\n");
    init_scan( filename );

    return 0;
}

/* Null handling of pragma directive.  We only need to understand this in C. */
prgm_direct(stream) {
}
