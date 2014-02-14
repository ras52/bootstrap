/* cpp.c  --  the C preprocessor
 *
 * Copyright (C) 2013, 2014 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */

struct pvector {
    char **start, **end, **end_store;
};
/* Search path for include files */
static struct pvector* incl_path;

/* Vector of scanner objects */
static struct pvector* incl_vec;

/* Vector of files to include in series */
static struct pvector* incl_files;

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
    pp_slurp(stream, &node, 0);
    return node;
}

/* How deep are we with #if{,def,ndef} directive. */
static
struct if_group {
    int is_else;  /* Have we had an #else yet? */
    int done;     /* Have we processed a group yet? */
    struct if_group* next;
} *if_stack = 0;

static
if_push(processing) {
    struct if_group* g = (struct if_group*) malloc( sizeof(struct if_group) );
    g->is_else = 0;
    g->done = processing;
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
 
        else if ( if_stack->next == start && !if_stack->done ) {
            if ( strcmp( str, "elif" ) == 0 ) {
                if_direct(stream, str, 1);
                free_node(tok);
                break;
            }
            else if ( strcmp( str, "else" ) == 0 ) {
                else_direct(stream);
                free_node(tok);
                break;   
            }
        }
        free_node(tok);

    } while ( if_stack != start );
}

struct node* get_word();

static
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
        if_push(1);

    free_node(name);
}

static
endi_direct(stream) {
    end_ppdir( stream, "endif" );

    if ( !if_stack )
        error( "Unexpected #endif directive" );
    if_pop();
}

static
else_direct(stream) {
    end_ppdir( stream, "endif" );

    if ( !if_stack )
        error( "Unexpected #else directive" );
    if ( if_stack->is_else )
        error("An #if group cannot have more than one #else");

    if_stack->is_else = 1;
    if (if_stack->done)
        skip_if(stream);
    else 
        if_stack->done = 1;
}

static
if_expand(node)
    struct node *node;
{
    /* This is a bit tricky as we cannot remove the current token from
     * the stack: instead, we must overwrite the current token to avoid 
     * an infinite loop. */
    if ( node->code == 'id' ) {
        char *str = node_str(node);
        if ( can_expand(str) ) {
            do_expand(str);
            return 1;
        }

        /* All other identifiers are replaced with 0. */
        else {
            set_code( node, 'num' );
            set_type( node, add_ref( implct_int() ) );
            set_op( node, 0, 0 );
        }
    }

    else if ( node->code == 'ppno' ) {
        /* We would overwrite the token in-place, but the mk_number() code 
         * allow that; so create a temporary and manually copy over. */
        struct node *num = mk_number(node);
        set_code( node, 'num' );
        set_type( node, add_ref( node_type(num) ) );
        set_op( node, 0, node_op(num, 0) );
        free_node(num);
        return 1;
    }
            
    return 0;
}

/* Tokenise the conditional on an #if or #elif line and push it back 
 * onto the stack with macros expanded, etc.  */
static
expand_cond(stream, directive) {
    struct node *e = new_node('mace', 0);
    int n;

    /* Locate the full preprocessor line, and use if_expand() to expand
     * macros, and convert pp-numbers to numbers. */
    pp_slurp(stream, &e, if_expand);
    if ( !e->arity )
        error( "Expected constant expression in #%s", directive );

    /* Set a _Pragma("RBC end_expr") node, and push back the expression.  
     * This will cause the expression parser die if it gets that far, rather 
     * than continuing into the next line of input. */
    pp_end_expr();

    for ( n = e->arity; n; --n )
        unget_token( add_ref( e->ops[n-1] ) );

    free_node(e);
}

/* Parse a conditional on an #if or #elif line and return its value */
static
eval_cond() {
    extern struct node *expr(), *eval();
    int n;
    struct node *e = expr(1);
    struct noed *val = eval(e);
    free_node(e);
    n = node_ival(val);
    free_node(val);
    return n;
}

/* Require the above _Pragma("RBC end_expr"); */
static
check_econd(directive) {
    extern struct node *get_node();
    struct node *e = get_node();
    if ( e->code != 'prgm' )
        error("Trailing content on #%s directive", directive);
    else if ( e->arity != 2 || !node_streq( e->ops[0], "RBC" )
              || !node_streq( e->ops[1], "end_expr" ) )
        int_error("Expected _Pragma(\"RBC end_expr\")");
    free_node(e);
}

static
if_direct(stream, directive, is_else) {
    int processing;

    if ( is_else && !if_stack )
        error( "Unexpected #%s directive", directive );
    if ( is_else && if_stack->is_else )
        error("An #%s group cannot follow an #else", directive );

    if ( is_else && if_stack->done ) {
        skip_if(stream);
        return;
    }

    expand_cond(stream, directive);
    processing = eval_cond() != 0;
    check_econd(directive);

    end_ppdir( stream, directive );

    if (is_else)
        if_stack->done = processing;
    else
        if_push(processing);

    if (!processing)
        skip_if(stream);
}

/* Hook for handling preprocessor directives other than #line and #pragma */
pp_direct(stream, str) {
    /* #pragma and #line are handled in scanbase.c in pp_dir() */
    if ( strcmp( str, "include" ) == 0 )
        incl_direct( stream );
    else if ( strcmp( str, "define" ) == 0 )
        defn_direct( stream );
    else if ( strcmp( str, "if" ) == 0 )
        if_direct( stream, str, 0 );
    else if ( strcmp( str, "ifdef" ) == 0 )
        ifdf_direct( stream, str, 1 );
    else if ( strcmp( str, "ifndef" ) == 0 )
        ifdf_direct( stream, str, 0 );
    else if ( strcmp( str, "elif" ) == 0 )
        if_direct( stream, str, 1 );
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
usage() {
    cli_error(
"Usage: cpp [-I include-dir] [-D name[=val]] [-o filename.i] filename.c\n"
    );
}

main(argc, argv) 
    int argc;
    char **argv;
{
    extern struct pvector* pvec_new();
    extern char* opt_arg();
    extern struct FILE *stdout, *fopen();
    struct FILE *f = stdout;
    char *filename = 0, *outname = 0, **t;
    int i = 1;

    incl_path = pvec_new();
    incl_vec = pvec_new();
    incl_files = pvec_new();

    /* The default include path for #include "foo.h" files only. */
    pvec_push( incl_path, "." );

    while ( i < argc ) {
        char *arg = argv[i], *arg2;

        if ( arg2 = opt_arg( argv, argc, &i, "-o" ) ) {
            if ( outname ) cli_error(
                "Multiple output files specified: '%s' and '%s'\n",
                outname, arg2 );
            outname = arg2;
        }

        else if ( strcmp( arg, "--help" ) == 0 ) 
            usage();

        else if ( arg2 = opt_arg( argv, argc, &i, "-I" ) )
            pvec_push( incl_path, arg2 );

        else if ( arg2 = opt_arg( argv, argc, &i, "-D" ) )
            parse_d_opt( arg2 );

        else if ( arg2 = opt_arg( argv, argc, &i, "-include" ) )
            pvec_push( incl_files, arg2 );

        else if ( arg[0] == '-' )
            cli_error("cpp: unknown option: %s\n", arg);

        else {
            if ( filename ) cli_error(
                "cpp: multiple input files specified: '%s' and '%s'\n",
                filename, arg );
            filename = arg;
            ++i;
        }
    }

    if ( !filename )
        cli_error("cpp: no input file specified\n");
    else /* Add the input filename last so that -include options are first */
        pvec_push( incl_files, filename );

    init_stypes();

    if (outname) {
        f = fopen( outname, "w" );
        if (!f) cli_error( "cpp: unable to open file '%s'\n", outname );
    }

    for ( t = incl_files->start; t != incl_files->end; ++t ) {
        init_scan(*t, 0);
        preprocess(f);
        close_scan();
    }

    if (outname)
        fclose(f);

    fini_macros();
    fini_stypes();
    rc_done();
    return 0;
}

struct scanner {
    char* filename;
    int line;
    struct FILE* stream;
    struct if_group* if_at_entry;
};



/* Handle a #include directive. */
static
incl_direct(stream) {
    int c = skip_hwhite(stream);
    /* TODO:  We should handle the #include pp-toks form of C90 (and later) 
     * that processes its args */
    if ( c == '"' || c == '<' ) {
        struct node* tok = get_qlit(stream, c, c == '<' ? '>' : c, 0);
        char* file = extract_str(tok);
    
        struct scanner* scanner = malloc( sizeof(struct scanner) );
        pvec_push( incl_vec, scanner );

        store_scan( &scanner->filename, &scanner->line, &scanner->stream );
        scanner->if_at_entry = if_stack;

        /* We ignore the first element of incl_path if it's a <header>.
         * That's because it is ".". */
        open_scan(file, incl_path->start + (c == '<') );

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
    if ( incl_vec->end != incl_vec->start ) {
        close_scan();

        struct scanner* scanner = (struct scanner*) pvec_pop( incl_vec );
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
static
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
