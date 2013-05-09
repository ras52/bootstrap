/* main.c  --  code to tokenising C input stream
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
cli_error(fmt) {
    extern stderr;
    vfprintf(stderr, fmt, &fmt);
    exit(1);
}

usage() {
    cli_error("Usage: cc -S [--compat] [-o filename.s] filename.c\n");
}

main(argc, argv) {
    extern stdout;
    auto filename = 0, l, i = 0, has_s = 0, outname = 0, freeout = 0;

    while ( ++i < argc ) {
        if ( strcmp( argv[i], "-S" ) == 0 ) 
            ++has_s;

        else if ( strcmp( argv[i], "-o" ) == 0 ) {
            if ( ++i == argc ) usage();
            outname = argv[i];
        }

        else if ( strcmp( argv[i], "--help" ) == 0 ) 
            usage();

        else if ( strcmp( argv[i], "--compat" ) == 0 )
            compat_flag = 1;

        else if ( rchar(argv[i], 0) == '-' )
            cli_error("cc: unknown option: %s\n", argv[i]);

        else {
            if ( filename )
                cli_error("cc: multiple input files specified\n");
            filename = argv[i];
        }
    }

    if ( !has_s )
        cli_error("cc: the -S option is mandatory\n");

    if ( !filename )
        cli_error("cc: no input file specified\n");

    l = strlen(filename);
    if ( rchar( filename, l-1 ) != 'c' || rchar( filename, l-2 ) != '.' )
        cli_error("cc: input filename must have .c extension\n");
    init_scan(filename);

    if (!outname) {
        outname = strdup( filename );
        freeout = 1;
        lchar( outname, l-1, 's' );
    }
    freopen( outname, "w", stdout );
    if (freeout) free(outname);

    init_stypes();
    init_symtab();

    compile(stdout);

    fini_symtab();
    fini_stypes();
    rc_done();
    return 0;
}
