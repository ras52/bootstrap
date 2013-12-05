/* cpp.c  --  the C preprocessor
 *
 * Copyright (C) 2013 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */

/* If only we had a preprocessor to avoid these duplications ... :-) */
struct node {
    int code;           /* character code for the node, e.g. '+' or 'if'. */
    int arity;          /* the number of nodes in the ops[] array. */
    struct node* type;  /* Always NULL in the preprocessor. */
    struct node* ops[4];
};

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

do_get_qlit(stream, c1, c2) {
    return get_qlit(stream, c1, c2, 0);
}

/* Pass through #pragma directive to the compiler proper */
prgm_direct(stream) {
    struct node* node = new_node('prgm', 0);
    pp_slurp(stream, &node);
    return node;
}

/* How deep are we with #if{,def,ndef} directive. */
static
struct if_group {
    int is_else;
    struct if_group* next;
} *if_stack = 0;

static
if_push(is_else) {
    struct if_group* g = (struct if_group*) malloc( sizeof(struct if_group) );
    g->is_else = is_else;
    g->next = if_stack;
    if_stack = g;
}

static
if_pop() {
    struct if_group* g = if_stack;
    if (!g) int_error("Popping from empty if-group stack");
    if_stack = g->next;
    free(g);
}

struct node* start_ppdir();
char* node_str();

/* Go into relaxed parsing mode, and skip to the corresponding #endif */
static
skip_if(stream) {
    struct if_group* start = if_stack->next;

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
            if_push(0);
        else if ( strcmp( str, "endif" ) == 0 )
            if_pop();
        else if ( strcmp( str, "else" ) == 0 && if_stack->next == start ) {
            if ( if_stack->is_else )
                error("An #if group cannot have more than one #else");
            if_stack->is_else = 1;
            free_node(tok);
            break;   
        }
        free_node(tok);

    } while ( if_stack != start );
}

struct node* get_word();

ifdf_direct(stream, directive, positive) {
    struct node* name;

    int c = skip_hwhite(stream);
    if ( !isidchar1(c) )
        error( "Expected identifier in #%s directive", directive );

    name = get_word( stream, c );
    end_ppdir( stream, directive );

    if ( pp_defined( node_str(name) ) != positive ) {
        if_push(0);
        skip_if(stream);
    }
    else
        if_push(0);

    free_node(name);
}

endi_direct(stream) {
    end_ppdir( stream, "endif" );

    if ( !if_stack )
        error( "Unexpected #endif directive" );
    if_pop();
}

else_direct(stream) {
    end_ppdir( stream, "endif" );

    if ( !if_stack )
        error( "Unexpected #else directive" );
    if ( if_stack->is_else )
        error("An #if group cannot have more than one #else");

    if_stack->is_else = 1;
    skip_if(stream);
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
    else if ( strcmp( str, "else" ) == 0 )
        else_direct( stream );
    else if ( strcmp( str, "endif" ) == 0 )
        endi_direct( stream );
    else if ( strcmp( str, "undef" ) == 0 )
        unde_direct( stream );
    else
        error("Unknown preprocessor directive: %s", str);
}

struct node* get_node();

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

    if ( !*file_ptr || strcmp( *file_ptr, in_file ) != 0 ) {
        /* Only at the very start of the input stream is *file_ptr null.
         * This test suppresses a blank line at the very start of the 
         * preprocessor output. */
        if (*file_ptr) fputc('\n', output);
        /* TODO:  Properly escape the filename. */
        fprintf( output, "#line %d \"%s\"\n", *line_ptr = in_line,
                 *file_ptr = in_file );
    }
    else if ( *line_ptr > in_line || *line_ptr < in_line - 4 ) 
        fprintf( output, "\n#line %d\n", *line_ptr = in_line );

    else if ( *line_ptr == in_line ) 
        fputc(' ', output);

    else while ( *line_ptr < in_line ) { 
        fputc('\n', output); 
        ++*line_ptr;
    }
}

static
print_node(output, node) 
    struct node* node;
{
    int c = node_code(node);

    if ( c == 'id' )
        fputs( node_str(node), output );
    else if ( c == 'ppno' || c == 'str' || c == 'chr' )
        fputs( node_str(node), output );
    else if ( c == 'prgm' ) {
        int i;
        fputs( "#pragma", output );
        for ( i = 0; i < node->arity; ++i ) {
            fputc(' ', output);
            print_node( output, node->ops[i] );
        }
    }
    else
        fprintf( output, "%Mc", c );
}

static
preprocess(output) {
    int c;
    int out_line = 1;
    char* out_file = 0;

    while ( c = peek_token() ) {
        /* We cannot call take_node() because (i) we don't want to set the
         * arity which, for a 'prgm' node, is unknown; and (ii) because it
         * will call next() immediately, which will cause any immediately
         * following preprocessor directive to be processesed, and if that
         * is an #undef removing and expansion of the current token, that 
         * will break things.  Oh, and calling next() after set_line() is
         * also necessary to get the right line numbers in the file. */
        struct node* node = get_node();

        if ( c == 'id' && can_expand( node_str(node) ) ) {
            /* It's a macro that has been expanded and pushed back on to
             * the parser stack: we do that to ensure proper re-scanning. */
            do_expand( node_str(node) );
        }
        else if ( c == 'prgm' && cpp_pragma(node) ) {
            /* It's a pragma that's been handled entirely in a preprocessor
             * and should not be included in the output. */
            free_node( node );
            next();
        }
        else {
            set_line( &out_line, &out_file, output );
            print_node(output, node);
            free_node( node );
            next();
        }
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
    cli_error("Usage: cpp [-I include-dir] [-o filename.i] filename.c\n");
}

main(argc, argv) 
    int argc;
    char **argv;
{
    char *filename = 0, *outname = 0;
    int i = 0;

    init_path();

    while ( ++i < argc ) {
        char *arg = argv[i];

        if ( strcmp( arg, "-o" ) == 0 ) {
            if ( ++i == argc ) usage(); arg = argv[i];
            if ( outname ) cli_error(
                "cpp: multiple output files specified: '%s' and '%s'\n",
                outname, arg);
            outname = arg;
        }

        else if ( strcmp( arg, "--help" ) == 0 ) 
            usage();

        else if ( strcmp( arg, "-I" ) == 0 ) {
            if ( ++i == argc ) usage();
            push_inc( argv[i] );
        }

        else if ( strcmp( arg, "-D" ) == 0 ) {
            if ( ++i == argc ) usage();
            parse_d_opt( argv[i] );
        }

        else if ( arg[0] == '-' )
            cli_error("cpp: unknown option: %s\n", arg);

        else {
            if ( filename ) cli_error(
                "cpp: multiple input files specified: '%s' and '%s'\n",
                filename, arg );
            filename = arg;
        }
    }

    if ( !filename )
        cli_error("cpp: no input file specified\n");
    init_scan( filename );

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
    struct if_group* if_at_entry;
};

static int incl_stksz = 0;
static struct scanner* incl_stack[256];

/* Search path for include files */
static char** inc_path;
static inc_len, inc_cap;

static
init_path() {
    inc_cap = 2;
    inc_path = (char**) malloc( inc_cap * sizeof(char*) );
    inc_path[0] = ".";
    inc_path[1] = 0;
    inc_len = 2;
}

static
push_inc(inc)
    char* inc;
{
    if ( inc_len == inc_cap ) {
        inc_cap *= 2;
        inc_path = (char**) realloc( inc_path, inc_cap * sizeof(char*) );
    }
    inc_path[ inc_len-1 ] = inc;
    inc_path[ inc_len++ ] = 0;
}

/* Handle a #include directive. */
static
incl_direct(stream) {
    int c = skip_hwhite(stream);
    /* TODO:  We should handle the #include pp-toks form of C90 (and later) 
     * that processes its args */
    if ( c == '"' || c == '<' ) {
        struct node* tok = get_qlit(stream, c, c == '<' ? '>' : c, 0);
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
        scanner->if_at_entry = if_stack;

        /* We ignore the first element of inc_path if it's a <header>.
         * That's because it is ".". */
        open_scan(file, inc_path + (c == '<') );

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
    if ( incl_stksz ) {
        close_scan();

        struct scanner* scanner = incl_stack[ --incl_stksz ];
        restorescan( scanner->filename, scanner->line, scanner->stream );
        if ( scanner->if_at_entry != if_stack )
            error("End of file while looking for #endif");
        free( scanner );
        return 1;
    } 
    else if ( if_stack )
        error("End of file while looking for #endif");
    else return 0;
}

/* Certain pragmas are handled entirely in the preprocessor. */
cpp_pragma(node) 
    struct node* node;
{
    if ( node->arity != 3 ) return 0;
    if ( !node_streq( node->ops[0], "RBC" ) ) return 0;

    if ( node_streq( node->ops[1], "cpp_unmask" ) ) {
        cpp_unmask( node_str( node->ops[2] ) );
        return 1;
    }

    return 0;
}
