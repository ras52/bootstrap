/* main.c  --  code to parse command line and initialise the compiler
 *
 * Copyright (C) 2013 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */

/* Is the --compatibility=4 flag given? */
compat_flag = 0;

static
compile(output) {
    auto node;
    while ( peek_token() && (node = top_level()) ) {
        codegen( output, node );
        free_node( node );
    }
}

usage() {
    cli_error("Usage: ccx [--compatibility=N] [-o filename.s] filename.c\n");
}

main(argc, argv) 
    int argc;
    char **argv;
{
    extern char* strdup();
    extern struct FILE* fopen();
    extern char* opt_arg();

    auto char *filename = 0, *outname = 0;
    auto int l, i = 1, freeout = 0;
    auto struct FILE* file;

    while ( i < argc ) {
        auto char *arg = argv[i], *arg2;

        if ( arg2 = opt_arg( argv, argc, &i, "-o" ) ) {
            if ( outname ) cli_error(
                "Multiple output files specified: '%s' and '%s'\n",
                outname, arg2 );
            outname = arg2;
        }

        else if ( strcmp( arg, "--help" ) == 0 ) 
            usage();

        else if ( arg2 = opt_arg( argv, argc, &i, "--compatibility" ) ) {
            if ( strcmp( arg2, "4" ) == 0 )
                compat_flag = 1;
            else if ( strcmp( arg2, "5" ) != 0 )
                cli_error("Compatibility with stage %s not supported", arg2);
        }

        else if ( rchar(argv[i], 0) == '-' )
            cli_error("cc: unknown option: %s\n", argv[i]);

        else {
            if ( filename ) cli_error(
                "cc: multiple input files specified: '%s' and '%s'\n",
                filename, argv[i]);
            filename = argv[i];
            ++i;
        }
    }

    if ( !filename )
        cli_error("cc: no input file specified\n");

    init_stypes();
    init_symtab();
    init_scan(filename, 0); 

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
