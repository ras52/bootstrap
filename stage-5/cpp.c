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

/* Hook for handling preprocessor directives other than #line and #pragma */
pp_direct(stream, str) {
    if ( strcmp( str, "include" ) == 0 )
        incl_direct( stream );
    else
        error("Unknown preprocessor directive: %s", str);
}


static
preprocess(output) {
    struct node* node;
    int out_line = 1;
    char* out_file = 0;
    while ( node = next() ) {
        int in_line = get_line(), c;
        char* in_file = getfilename();
        if ( !out_file || strcmp( out_file, in_file ) != 0 )
            /* TODO:  Properly escape the filename. */
            fprintf(output, "\n#line %d \"%s\"\n", out_line = in_line,
                    out_file = in_file );
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
    char *filename = 0, *outname = 0;
    int i = 0;

    while ( ++i < argc ) {
        if ( strcmp( argv[i], "-o" ) == 0 ) {
            if ( ++i == argc ) usage();
            if ( outname ) cli_error(
                "cpp: multiple output files specified: '%s' and '%s'\n",
                outname, argv[i]);
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

    if (outname) {
        auto f = fopen( outname, "w" );
        if (!f) cli_error( "cpp: unable to open file '%s'\n", outname );
        preprocess(f);
        fclose(f);
    }
    else {
        extern stdout;
        preprocess( stdout );
    }

    close_scan();

    rc_done();
    return 0;
}


static int incl_stksz = 0;
static struct scanner* incl_stack[256];

static
incl_direct(stream) {
    auto int c = skip_hwhite(stream);
    if ( c == '"' ) {
        auto struct token* tok = get_qlit(stream, c, 0);
        auto char* file = extract_str(tok);
    
        /* The C standard allows this to be as low as 15 [C99 5.2.4.1].
         * It would be easy to allow an arbitrary include depth, but it's
         * probably wise to prevent infinite #include recursion, and this 
         * is a simple way of doing it.  */
        if ( incl_stksz == 255 )
            error( "Exceeded #include stack size" );

        struct scanner* scanner = store_scan();
        incl_stack[ incl_stksz++ ] = scanner;
        init_scan(file);

        free(file);
        free_node(tok);
    }
    else
        error( "Missing file name on #include" );
}

handle_eof() {
    if ( incl_stksz ) {
        struct scanner* scanner = incl_stack[ --incl_stksz ];
        restorescan( scanner );
        return 1;
    } 
    else return 0;
}
