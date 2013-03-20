/* main.c  --  code to tokenising C input stream
 *
 * Copyright (C) 2013 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */

static
compile(output) {
    auto node;
    while ( token && (node = top_level()) ) {
        codegen( output, node );
        free_node( node );
    }
}

static
cli_error(fmt) {
    vfprintf(stderr, fmt, &fmt);
    exit(1);
}

usage() {
    cli_error("Usage: cc -S [-o filename.s] filename.c\n");
}

main(argc, argv) {
    auto filename, l, outname = 0, freeout = 0;

    if ( argc < 3 ) usage();

    if ( strcmp( argv[1], "-S" ) )
        cli_error("cc: the -S option is mandatory\n");

    if ( strcmp( argv[2], "-o" ) == 0 ) {
        if ( argc != 5 ) usage();
        outname = argv[3];
        filename = argv[4];
    }
    else {
        if ( argc != 3 ) usage();
        filename = argv[2];
    }

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

    init_symtab();
    compile(stdout);
    return 0;
}
