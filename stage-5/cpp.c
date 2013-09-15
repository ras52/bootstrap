/* cpp.c  --  the C preprocessor
 *
 * Copyright (C) 2013 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */

/* The preprocessor does not know about keywords. */
chk_keyword(node) 
    struct node* node;
{
    return node;
}

/* The preprocessor doesn't need to understand pp-numbers, just parse them. */
get_number(stream, c1, c2) 
{
    return get_ppnum(stream, c1, c2);
}

do_get_qlit(stream, c) {
    return get_qlit(stream, c, 0);
}

/* Null handling of pragma directive.  We only need to understand this in C. */
prgm_direct(stream) {
    struct node* node = new_node('prgm', 0);
    int i = 0;
    while (1) {
        int c = fgetc(stream);
        if ( c == -1 )
            error("End of file during preprocessor directive");
        else if ( c == '\n' ) {
            ungetc(c, stream);
            break;
        }
        else node_lchar(&node, &i, c);
    }
    /* Null terminate */
    node_lchar( &node, &i, 0 );
    return node;
}

static
preprocess(output) {
    struct node* node;
    int out_line = 1;
    while ( node = next() ) {
        int in_line = get_line(), c;
        if ( out_line > in_line || out_line < in_line - 4 ) 
            fprintf(output, "\n#line %d\n", out_line = in_line);
        else if ( out_line == in_line ) 
            fputc(' ', output);
        else while ( out_line < in_line ) { 
            fputc('\n', output); ++out_line;
        }
        
        c = node_code(node);
        if ( c == 'id' || c == 'ppno' || c == 'str' || c == 'chr' )
            fputs( node_str(node), output );
        else if ( c == 'prgm' )
            fprintf( output, "#pragma %s", node_str(node) );
        else
            fprintf( output, "%Mc", c );

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
    cli_error("Usage: cpp [-o filename.i] filename.c\n");
}

main(argc, argv) 
    int argc;
    char **argv;
{
    extern stdout;
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

    preprocess( stdout );

    rc_done();
    return 0;
}


