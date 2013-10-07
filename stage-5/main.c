/* main.c  --  code to parse command line and initialise the compiler
 *
 * Copyright (C) 2013 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */

/* Is the --compat flag given? */
compat_flag = 0;

static
compile(output) {
    auto node;
    while ( peek_token() && (node = top_level()) ) {
        codegen( output, node );
        free_node( node );
    }
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
    cli_error("Usage: cc -S [--compat] [-o filename.s] filename.c\n");
}

main(argc, argv) 
    int argc;
    char **argv;
{
    extern char* strdup();
    extern struct FILE* fopen();

    auto char *filename = 0, *outname = 0;
    auto int l, i = 0, has_s = 0, freeout = 0;
    auto struct FILE* file;

    while ( ++i < argc ) {
        if ( strcmp( argv[i], "-S" ) == 0 ) 
            ++has_s;

        else if ( strcmp( argv[i], "-o" ) == 0 ) {
            if ( ++i == argc ) usage();
            if ( outname ) cli_error(
                "cc: multiple output files specified: '%s' and '%s'\n",
                outname, argv[i]);
            outname = argv[i];
        }

        else if ( strcmp( argv[i], "--help" ) == 0 ) 
            usage();

        else if ( strcmp( argv[i], "--compat" ) == 0 )
            compat_flag = 1;

        else if ( rchar(argv[i], 0) == '-' )
            cli_error("cc: unknown option: %s\n", argv[i]);

        else {
            if ( filename ) cli_error(
                "cc: multiple input files specified: '%s' and '%s'\n",
                filename, argv[i]);
            filename = argv[i];
        }
    }

    if ( !has_s )
        cli_error("cc: the -S option is mandatory\n");

    if ( !filename )
        cli_error("cc: no input file specified\n");

    init_stypes();
    init_symtab();
    init_scan(filename); 

    if (!outname) {
        /* We allow .c or .i filenames: .i is used for preprocessed source. */
        l = strlen(filename);
        if ( rchar( filename, l-1 ) != 'c' && rchar( filename, l-1 ) != 'i'
             || rchar( filename, l-2 ) != '.' )
            cli_error("cc: input filename must have .c extension\n");

        outname = strdup( filename );
        freeout = 1;
        lchar( outname, l-1, 's' );
    }

    file = fopen( outname, "w" );
    if (!file) cli_error( "cc: unable to open file '%s'\n", outname );
    if (freeout) free( outname );

    compile( file );

    fclose( file );
    close_scan();

    fini_symtab();
    fini_stypes();
    rc_done();
    return 0;
}
