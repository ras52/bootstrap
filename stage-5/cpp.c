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

/* Pass through #pragma directive to the compiler proper */
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

/* How deep are we with #if{,def,ndef} directive. */
static
int if_depth = 0;

struct node* start_ppdir();
char* node_str();

/* Go into relaxed parsing mode, and skip to the corresponding #endif */
skip_if(stream) {
    int start_depth = if_depth;
    ++if_depth;
    do {
        struct node* tok;
        char* str;
        int c;
    
        do c = next_skip(); 
        while ( c != -1 && c != '\n#' );
        if ( c == -1 ) error("End of file while looking for #endif");
    
        tok = start_ppdir(stream);
        str = node_str(tok);
        
        if ( strcmp( str, "ifdef" ) == 0 || strcmp( str, "ifndef" ) == 0 
             || strcmp( str, "if" ) == 0 )
            ++if_depth;
        else if ( strcmp( str, "endif" ) == 0 )
            --if_depth;
        free_node(tok);

    } while ( if_depth != start_depth );
}

struct node* get_word();

ifdf_direct(stream, directive, positive) {
    struct node* name;

    int c = skip_hwhite(stream);
    if ( !isidchar1(c) )
        error( "Expected identifier in #%s directive", directive );

    name = get_word( stream, c );
    c = skip_hwhite(stream);
    if ( c == '\n' )
        ungetc( c, stream );
    else
        error( "Unexpected tokens after #%s directive", directive );

    if ( pp_defined( node_str(name) ) != positive )
        skip_if(stream);
    else
        ++if_depth;

    free_node(name);
}

endi_direct(stream) {
    int c = skip_hwhite(stream);
    if ( c == '\n' )
        ungetc( c, stream );
    else
        error( "Unexpected tokens after #endif directive" );

    if (if_depth == 0)
        error( "Unexpected #endif directive" );
    --if_depth;
}

/* Hook for handling preprocessor directives other than #line and #pragma */
pp_direct(stream, str) {
    if ( strcmp( str, "include" ) == 0 )
        incl_direct( stream );
    else if ( strcmp( str, "define" ) == 0 )
        defn_direct( stream );
    else if ( strcmp( str, "ifdef" ) == 0 )
        ifdf_direct( stream, str, 1 );
    else if ( strcmp( str, "ifndef" ) == 0 )
        ifdf_direct( stream, str, 0 );
    else if ( strcmp( str, "endif" ) == 0 )
        endi_direct( stream );
    else if ( strcmp( str, "undef" ) == 0 )
        unde_direct( stream );
    else
        error("Unknown preprocessor directive: %s", str);
}

struct node* next();
struct node* take_node();

static
set_line( line_ptr, file_ptr, output )
    int* line_ptr;
    char** file_ptr;
    struct FILE* output;
{
    /* Before handling the token, we need to get to the right line
     * in the output.  We'll do this by emitting a few '\n's if 
     * it's only a small difference, and for larger offsets, backwards
     * offsets, or changes in filename, we emit an explicit #line.  */
    int in_line = get_line();
    char* in_file = getfilename();
    if ( !*file_ptr || strcmp( *file_ptr, in_file ) != 0 )
        /* TODO:  Properly escape the filename. */
        fprintf( output, "\n#line %d \"%s\"\n", *line_ptr = in_line,
                 *file_ptr = in_file );
    if ( *line_ptr > in_line || *line_ptr < in_line - 4 ) 
        fprintf( output, "\n#line %d\n", *line_ptr = in_line );
    else if ( *line_ptr == in_line ) 
        fputc(' ', output);
    else while ( *line_ptr < in_line ) { 
        fputc('\n', output); 
        ++*line_ptr;
    }
}

static
preprocess(output) {
    int c;
    int out_line = 1;
    char* out_file = 0;

    while ( c = peek_token() ) {
        struct node* node;
        /* Call set_line() before take_node() or we bump the line too soon. */
        set_line( &out_line, &out_file, output );

        node = take_node(0);
        if ( c == 'id' ) {
            if ( ! expand( node_str(node), output ) )
                fputs( node_str(node), output );
        }
        else if ( c == 'ppno' || c == 'str' || c == 'chr' )
            fputs( node_str(node), output );
        else if ( c == 'prgm' )
            fprintf( output, "#pragma %s", node_str(node) );
        else
            fprintf( output, "%Mc", c );
        free_node( node );
    }
    fputc('\n', output); /* file needs to end with '\n' */
}

static
cli_error(fmt) 
    char *fmt;
{
    extern stderr;
    vfprintf(stderr, fmt, &fmt);
    exit(1);
}

static
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
    init_scan( filename );  next();

    if (outname) {
        struct FILE* f = fopen( outname, "w" );
        if (!f) cli_error( "cpp: unable to open file '%s'\n", outname );
        preprocess(f);
        fclose(f);
    }
    else {
        extern stdout;
        preprocess( stdout );
    }

    close_scan();

    fini_macros();
    rc_done();
    return 0;
}

struct scanner {
    char* filename;
    int line;
    struct FILE* stream;
};

static int incl_stksz = 0;
static struct scanner* incl_stack[256];

/* Handle a #include directive. */
static
incl_direct(stream) {
    int c = skip_hwhite(stream);
    /* TODO:  We should handle #include <foo> and later the #include pp-toks 
     * form of C90 (and later) that processes its args */
    if ( c == '"' ) {
        struct node* tok = get_qlit(stream, c, 0);
        char* file = extract_str(tok);
    
        /* The C standard allows this to be as low as 15 [C99 5.2.4.1].
         * It would be easy to allow an arbitrary include depth, but it's
         * probably wise to prevent infinite #include recursion, and this 
         * is a simple way of doing it.  */
        if ( incl_stksz == 255 )
            error( "Exceeded #include stack size" );

        struct scanner* scanner = malloc( sizeof(struct scanner) );
        incl_stack[ incl_stksz++ ] = scanner;

        store_scan( &scanner->filename, &scanner->line, &scanner->stream );
        init_scan(file);

        free(file);
        free_node(tok);
    }
    else
        error( "Missing file name on #include" );
}

/* This is called by next() when we get EOF.  Returns 1 if EOF has been
 * handled -- by which we mean we've been able to pop the include stack --
 * or 0 if it really is EOF. */
handle_eof() {
    if ( if_depth )
        error("End of file while looking for #endif");

    if ( incl_stksz ) {
        close_scan();

        struct scanner* scanner = incl_stack[ --incl_stksz ];
        restorescan( scanner->filename, scanner->line, scanner->stream );
        free( scanner );
        return 1;
    } 
    else return 0;
}

