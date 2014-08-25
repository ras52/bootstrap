/* scanner.c  --  code for tokenising the C input stream
 *
 * Copyright (C) 2013, 2014 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */

static struct FILE* input_strm;
static int line;
static char* filename;

get_line() {
    return line;
}

/* 11 character limit on external identifiers in stage-3 as & ld */
getfilename() {
    return filename;
}

static
print_msg(fmt, ap)
    char *fmt;
{
    extern stderr;
    fprintf(stderr, "%s:%d: ", filename, line);
    vfprintf(stderr, fmt, ap);
    fputc('\n', stderr);
}

warning(fmt)
    char *fmt;
{
    print_msg(fmt, &fmt);
}

error(fmt)
    char *fmt;
{
    print_msg(fmt, &fmt);
    exit(1);
}

int_error(fmt)
    char* fmt;
{
    extern stderr;
    fputs("Internal error: ", stderr);
    vfprintf(stderr, fmt, &fmt);
    fprintf(stderr, "\n  while processing %s:%d\n", filename, line);
    fflush(stderr);
    abort();
}


/* A version of fgetc() that increments LINE if a '\n' is found.  
 * We cannot call this universally unless we write a corresponding ungetc()
 * that decrements LINE.  */
static
fgetcl(stream) {
    auto int c = fgetc(stream);
    if ( c == '\n' ) ++line;
    return c;
}

/* Skips over a C-style comment (the opening / and * are already read). */
skip_ccomm(stream) {
    auto int c;

    do {
        do c = fgetcl(stream);
        while ( c != -1 && c != '*' );
        c = fgetcl(stream);
    } while ( c != -1 && c != '/' );

    if ( c == -1 )
        error("End of file during comment");
}

/* Skips over a C++-style comment (the opening // is already read). */
skip_cxcomm(stream) {
    auto int c;

    while (1) {
        do c = fgetc(stream);
        while ( c != '\n' && c != '\\' && c != -1 );
        if ( c == '\\' ) {
            c = fgetcl(stream);
            if ( c == -1 ) break;
        }
        else {
            ungetc(c, stream);
            break;
        }
    } 

    if ( c == -1 )
        error("End of file during comment");
}

/* Skip over whitespace other than '\n' (for the preprocessor), and return
 * the next character without ungetting it. */
skip_hwhite(stream) {
    auto int c;
    while (1) {
        /* This must be fgetc() not fgetcl(), because we unget a '\n' */
        do c = fgetc(stream);
        while ( c != '\n' && isspace(c) ); 
        if ( c == -1 )
            error("End of file during preprocessor directive");
        else if (c == '/') {
            auto c2 = fgetc(stream);
            if ( c2 == '*' ) 
                skip_ccomm(stream);
            else if ( c2 == '/' )
                skip_cxcomm(stream);
            else {
                ungetc(c2, stream);
                return c;
            }
        }
        else if (c == '\\') {
            auto c2 = fgetcl(stream);
            if ( c2 != '\n' ) {
                ungetc(c2, stream);
                return c;
            }
        }
        else return c;
    }
    return c;
}

/* Read a number in a preprocessor directive. */
pp_dir_num(stream) {
    extern struct node* get_ppnum();

    auto struct node* tok;
    auto char* nptr;
    auto int c = skip_hwhite(stream), n;

    if ( !isdigit(c) )
        error("Number expected in preprocessor directive");

    tok = get_ppnum(stream, c, 0);
    n = strtoul( node_str(tok), &nptr, 0 );
    if ( rchar(nptr, 0) )
        error("Trailing characters on number in preprocessor directive");
    free_node(tok);

    return n;
}

extract_str(node) {
    auto char* src = node_str(node);
    auto int len = strlen(src);
    auto char* dest = malloc(len + 1);
    auto int i = 0, j = 0, c;
    auto q = rchar( src, i++ );
    if ( q == '<' ) q = '>';

    while (1) {
        auto c = rchar( src, i++ );
        if ( c == q ) break;
        else if ( c == '\\' ) {
            c = rchar( src, i++ );
            if ( c == 'n' ) c = '\n';
            else if ( c == 't' ) c = '\t';
            else if ( c == '0' ) c = '\0';
        }
        lchar( dest, j++, c );
    }
    lchar( dest, j++, 0 );

    return dest;
}

/* Handle a #line directive */
static
line_direct(stream) {
    auto int c;

    /* We subtract 1 from the line number because it'll get incremented
     * at the end of the line. */
    line = pp_dir_num(stream) - 1;

    c = skip_hwhite(stream);
    if ( c == '"' ) {
        auto struct node* tok = get_qlit(stream, c, c, 0);
        /* Don't free(filename) until after this, or we have problems reporting
         * a problem with the string, e.g. if it contains invalid escapes. */
        auto char* new_name = extract_str(tok);
        free( filename ); filename = new_name;
        free_node(tok);
    }
    else if ( c == '\n' )
        ungetc(c, stream);
}

/* We've read a '#' at the start of a line, get the next token which will
 * the directive name (e.g. 'include'), which we return. */
struct node*
start_ppdir(stream) {
    extern struct node* get_word();
    auto int c = skip_hwhite(stream);

    /* The null directive -- just a '#' by itself on the line */
    if ( c == '\n' ) { 
        ungetc(c, stream); 
        return 0; 
    }
   
    /* In principle this matches the non-directive syntax of C99 / C++1x,
     * but that's for use as extensions only, and we unilaterally decide
     * that all our extensions will begin with an identifier (like 'include'). 
     * This is a valid implementation choice. */
    if ( !isidchar1(c) ) 
        error("Malformed preprocessor directive");

    return get_word(stream, c);
}

/* We've read all the expected content on the pp directive.  We now
 * expect just whitespace to the line end, and give an error if that's 
 * not the case. */
end_ppdir(stream, directive) {
    auto int c = skip_hwhite(stream);
    if ( c == '\n' )
        /* Need to unget it so that skip_white will get it again.  This is
         * necessary so that skip_white knows whether a # is part of a 
         * preprocessing directive, and also because skip_hwhite got it 
         * without bumping the line number. */
        ungetc( c, stream );
    else
        error( "Unexpected tokens after #%s directive", directive );
}

/* We've just read a #.  Read the rest of the directive. */
static
struct node*
pp_dir(stream) {
    extern struct node* prgm_direct();
    extern char* node_str();

    auto struct node *tok = start_ppdir(stream), *ret = 0;
    auto char* str;
    auto int c;

    /* Is it the null directive? */
    if ( !tok )
        return 0;

    str = node_str(tok);
    if ( strcmp( str, "line" ) == 0 )
        line_direct(stream);
    else if ( strcmp( str, "pragma" ) == 0 )
        ret = prgm_direct(stream);
    else
        pp_direct(stream, str);

    end_ppdir(stream, str);
    free_node(tok);
    return ret;
}

/* Skips over whitespace, including comments, and returns the next character
 * without ungetting it. */
static
skip_white(stream) {
    auto c, line_start = 0;
    /* Are we at the start of the file? */
    if (!line) line = line_start = 1;
        
    while (1) {
        do c = fgetcl(stream);
        while ( c != '\n' && isspace(c) ); 

        if (c == '\n')
            line_start = 1;
        else if (line_start && c == '#')
            /* We use this to transfer control back to the tokeniser */
            return '\n#';
        else if (c == '/') {
            auto c2 = fgetc(stream);
            if ( c2 == '*' ) 
                skip_ccomm(stream);
            else if ( c2 == '/' )
                skip_cxcomm(stream);
            else {
                ungetc(c2, stream);
                return c;
            }
        }
        else if (c == '\\') {
            auto c2 = fgetcl(stream);
            if ( c2 != '\n' ) {
                ungetc(c2, stream);
                return c;
            }
        }
        else return c;
    }
}

/* Is C a valid character to start an identifier (or keyword)? */
isidchar1(c) {
    return c == '_' || isalpha(c);
}

/* Is C a valid character to continue an identifier (or keyword)? */
isidchar(c) {
    return c == '_' || isalnum(c);
}

/* Create a node, NODE, which will be returned; read a word (i.e. identifier 
 * or keyword) starting with C (which has already been checked by isidchar1) 
 * into NODE->str, set NODE->type, and return NODE. */
struct node*
get_word(stream, c) {
    /* struct node { int type; int dummy; char str[]; } */
    auto struct node* node = new_node('id', 0);
    auto int i = 0;

    node_lchar( &node, &i, c );
    while ( isidchar( c = fgetc(stream) ) )
        node_lchar( &node, &i, c );

    ungetc(c, stream);
    node_lchar( &node, &i, 0 );
    return node;
}

/* Read a preprocessor number into a new node that will be returned.  
 * The pp-number will be converted into a proper number (or an error issued)
 * in the get_number() function. */
struct node*
get_ppnum(stream, c, c2) {
    auto int i = 0;

    /* struct node { int type; int dummy, dummy2; char str[]; }; */
    auto struct node* node = new_node('ppno', 0);

    node_lchar( &node, &i, c );
    if (c2) node_lchar( &node, &i, c2 );

    /* At this stage in processing, arbitrary suffixes (and infixes) are
     * permitted, not just the 'u' and 'l' type suffixes, the '0x' prefix,
     * the hex digits, and the 'e' expontent infix. */
    while ( (c = fgetc(stream)) != -1 && ( c == '.' || isalnum(c) ) ) {
        node_lchar( &node, &i, c );

        /* 'E' and 'e' are for exponents, and can have a sign suffix.
         * C99 adds 'P' and 'p'. */
        if ( c == 'e' || c == 'E' ) {
            c = fgetc(stream);
            if (c == '+' || c == '-') node_lchar( &node, &i, c );
            else if ( c == -1 ) break;
            else ungetc(c, stream);
        }
    }

    node_lchar( &node, &i, 0 );
    ungetc(c, stream);

    return node;
}

/* Read a string or character literal into VALUE, beginning with quote mark, 
 * Q, and return the appropriate token type ('str' or 'chr'). */
get_qlit(stream, q1, q2, len_ptr) 
    int* len_ptr;
{
    extern struct node* new_node();

    auto i = 0, l = 0, c;
    /* struct node { int type; int dummy; char str[]; } */
    auto code;
    auto struct node* node;
    auto char* errname;
    if      ( q1 == '"'  ) { code = 'str'; errname = "string literal";     }
    else if ( q1 == '\'' ) { code = 'chr'; errname = "character constant"; }
    else if ( q1 == '<'  ) { code = 'hdr'; errname = "header name";        }
    else int_error("Unknown type of quoted string %c...%c", q1, q2);
    node = new_node( code, 0 );

    node_lchar( &node, &i, q1 );
    
    while ( (c = fgetc(stream)) != -1 && c != q2 ) {
        if ( c == '\\' ) {
            auto int c2 = fgetc(stream);
            /* Line continuations are permitted inside string literals, etc.,
             * as they are handled in phase-2, before tokenisation. */
            if (c2 != '\n') {
                /* This is undefined behaviour in a header name, so we're free
                 * to parse escapes, as for string literals. 
                 * Pass off to the assembler.  TODO: The handling of escapes is 
                 * not conformant.  Also, share code with parse_chr(), above. */
                node_lchar( &node, &i, c );
                if ( (c = c2) == -1 ) break;
                node_lchar( &node, &i, c );
                ++l;
            }
        }
        else if ( c == '\n' )
            error("Line feeds are not permitted in a %s", errname);
        else {
            node_lchar( &node, &i, c );
            ++l;
        }
    }

    if ( c == -1 )
        error("Unexpected EOF during %s", errname);

    node_lchar( &node, &i, q2 );
    node_lchar( &node, &i, 0 );

    if (len_ptr)
        *len_ptr = l;

    return node;
}

/* Test whether the two characters in the multicharacter literal, OP, is a 
 * valid lexical representation of an operator.  So '[]' does not  */
static
is_2charop(op) {
    /* TODO: digraphs and trigraphs */
    auto mops[20] = { 
        /* 0     1     2     3     4     5     6     7     8     9 */
        '++', '--', '<<', '>>', '<=', '>=', '==', '!=', '&&', '||',
        '*=', '%=', '/=', '+=', '-=', '&=', '|=', '^=',  '->', 0 
    };

    /* Search for two-character operator name in the mops[] list, above */
    auto i = 0;
    while ( mops[i] && mops[i] != op )
        ++i;

    return mops[i] ? 1 : 0;
}

/* Read an operator, starting with character C, and return a node with
 * NODE->type set to the operator as a multicharacter literal. */
struct node*
get_multiop(stream, c) {
    /* Fetch the second character */
    auto c2 = fgetc(stream);
    if ( c2 == -1 ) return new_node( c, 0 );
    else lchar( &c, 1, c2 );

    /* If it's not a two-character operator, it must be a single character 
     * operator -- node that the only three-character operators and two-
     * character operators with an extra character appended.  
     * TODO: '...' breaks that assumption. */
    if ( !is_2charop(c) ) {
        ungetc( rchar( &c, 1 ), stream );
        return new_node( rchar( &c, 0 ), 0 );
    }

    /* Is it a <<= or >>= operator? */
    if ( c == '<<' || c == '>>' ) {
        c2 = fgetc(stream);
        if ( c2 == '=' ) lchar( &c, 2, c2 );
        else ungetc(c2, stream);
    }

    return new_node(c, 0);
}

/* Test whether token type OP is an operator in the syntax tree. */
is_op(op) {
    return strnlen(&op, 4) == 1 && strchr("&*+-~!/%<>^|=.,", op)
        || is_2charop(op) || op == '?:' || op == '[]' 
        || op == '>>=' || op == '<<=';
}

/* Contains the token most recently read by next() */
static struct node* token;

/* Read the next lexical element into TOKEN without preprocessing, and
 * return the code of TOKEN, -1 for EOF, or '\n#' for a # starting a 
 * preprocessor directive. */
static
raw_next(h_mode) {
    /* Oh! for headers */
    extern struct node* chk_keyword();
    extern struct node* get_number();
    extern struct node* new_node();
    extern struct node* do_get_qlit();
    extern struct node* pb_pop();

    auto c;
    auto struct FILE* stream = input_strm;

    if ( token = pb_pop() )
        return node_code( token );

    if ( h_mode )
        c = skip_hwhite(stream);
    else
        c = skip_white(stream);
    
    if ( c == -1 || c == '\n#' || h_mode && c == '\n' ) {
        token = 0;
        return c;
    }
    else if ( isidchar1(c) )
        token = chk_keyword( get_word(stream, c) );
    else if ( isdigit(c) )
        token = get_number(stream, c, 0);
    else if ( c == '.' ) {
        /* A . could be the start of a ppnumber, or a '.' operator  */
        auto c2 = fgetc(stream);
        if ( isdigit(c2) )
            token = get_number(stream, c, c2);
        else {
            ungetc(c2, stream);
            token = new_node( c, 0 );
        }
    }
    else if ( c == '\'' || c == '"' )
        token = do_get_qlit(stream, c, c);
    else 
        token = get_multiop(stream, c);

    return node_code( token );
}

/* Skip over a token.  This is used when skipping disabled #if..#else block. */
next_skip() {
    auto c = raw_next(0);
    free_node( token );
    return c;
}

/* This is used in horizontal mode by the preprocessor. */
pp_next() {
    while (1) {
        auto c = raw_next(0);
        if ( c == 'prgm' && cpp_pragma(token) )
            free_node( token );
        else return c;
    }
}

/* Read the next lexical element, handling preprocessing directives */
next() {
    auto c;
    do {
        c = raw_next(0);
        if ( c == -1 ) {
            /* In the preprocessor, there might be another file to handle. */
            if ( handle_eof() ) c = '\n#';
        }
        else if ( c == '\n#' ) {
            /* If pp_dir() returns a token, then we return that.
             * This happens with #pragma in the preprocessor. */
            if ( token = pp_dir(input_strm) ) c = 0;
        }
    } while ( c == '\n#' );
    return token;
}

/* Slurp all tokens until the next new line into *node_ptr (if not NULL), 
 * and return the next character (which will necessarily be '\n'). 
 * The current token will have been freed. */
pp_slurp(stream, node_ptr, try_expand)
    struct node** node_ptr;
    int (*try_expand)();
{
    extern struct node *vnode_app();
    auto int i = 0, c;
    while (1) { 
        /* If there's anything on the push-back stack, we've already
         * handled whitespace. */
        if ( pb_empty() ) {
            c = skip_hwhite(stream);
            ungetc(c, stream);
            if ( c == '\n' )
                break;
        }
       
        raw_next(0);

        /* If (*try_expand)() returns true, then it has handled the token,
         * and probably pushed something onto the push-back stack. */
        while ( try_expand && try_expand(stream, token) ) 
            ;

        c = peek_token();

        if ( node_ptr && token )
            *node_ptr = vnode_app( *node_ptr, token );
        else
            free_node( token );
    }
    return c;
}

unget_token(t) 
    struct node* t;
{
    pb_push(token);
    token = t;
}

static
do_open( in_filename, search_path ) 
    char* in_filename;
    char** search_path;
{
    extern char* strdup();
    extern struct FILE* fopen();

    auto int baselen = strlen( in_filename );
    while ( *search_path ) {
        /* Build the trial filename */
        auto int pathlen = strlen( *search_path );
        /* The 2 extra characters are for '/' and '\0'. */
        auto char* f = malloc( baselen + pathlen + 2 );
        auto struct FILE* file;
        strcpy( f, *search_path );
        lchar( f, pathlen, '/' );
        strcpy( f+pathlen+1, in_filename );
        if ( file = fopen( f, "r" ) ) {
            input_strm = file;
            if ( strcmp( *search_path, "." ) == 0 ) {
                /* It just looks more elegant to write "foo.h" instead of 
                 * "./foo.h" for files in the local directory.  */
                filename = strdup( in_filename );
                free(f);
            }
            else
                filename = f;
            line = 0;
            return;
        }
        free(f);
        /* This is ++search_path, but because sizeof(*search_path) > 1, we
         * cannot do that (and expect it to work) in the stage-4 cc. */
        search_path = &search_path[1];
    }
    error("Unable to load file \"%s\"", in_filename);
}

scan_str(str) {
    extern struct FILE* __fopenstr();
    input_strm = __fopenstr(str, strlen(str));
    filename = "<command-line>";
    line = 0;
}

init_scan(in_filename, search_path) {
    extern char* strdup();
    auto char* null_search[2] = { ".", 0 };
    filename = strdup("<command-line>");
    do_open( in_filename, search_path ? search_path : null_search );
    next();
}

open_scan(in_filename, search_path) {
    do_open( in_filename, search_path );
}

close_scan() {
    fclose( input_strm );
    free( filename );
}

store_scan(file_ptr, line_ptr, stream_ptr )
    char**        file_ptr;
    int*          line_ptr;
    struct FILE** stream_ptr;
{
    *file_ptr   = filename;
    *line_ptr   = line;
    *stream_ptr = input_strm;
}

/* 11 character limit on external identifiers in stage-3 as & ld */
restorescan(new_file, new_line, new_strm)
    char*        new_file;
    int          new_line;
    struct FILE* new_strm;
{
    filename   = new_file;
    line       = new_line;
    input_strm = new_strm;
}

/* Skip over a piece of syntax, deallocating its node */
skip_node(type) {
    if (!token)
        error("Unexpected EOF when expecting '%Mc'", type);

    else if ( node_code(token) != type)
        error("Expected a '%Mc'", type);
    
    free_node(token);
    return next();
}

/* Take ownership of the current node, and call next() */
take_node(arity) {
    auto node = token;
    set_arity( node, arity );
    next();
    return node;
}

get_node() {
    return token;
}

/* Require P to be non-null, and give an 'unexpected EOF' error if it is not.
 * Returns P. */
req_token() {
    if (!token) error("Unexpected end of file");
    return token;
}

peek_token() {
    return token ? node_code(token) : 0;
}

/* Convert a quoted character literal in STR (complete with single quotes,
 * and escapes) into an integer, and return that.   This is in this file
 * rather than scanner.c because the preprocessor needs to handle character
 * constants in #if expressions. */
parse_chr(str)
    char *str;
{
    auto i = 0, val = 0, bytes = 0;

    if ( rchar( str, i++ ) != '\'' )
        int_error("Character literal doesn't begin with '");

    while (1) {
        auto c = rchar( str, i++ );
        if ( c == '\'' ) break;
        else if ( bytes == 4 ) 
            error("Character literal overflows 32 bits");
        else if ( c == '\\' ) {
            c = rchar( str, i++ );
            if ( c == 'n' ) c = '\n';
            else if ( c == 't' ) c = '\t';
            else if ( c == '0' ) c = '\0';
        }
        val += c << (bytes++ << 3);
    }

    if ( bytes == 0 )
        error("Empty character literal");

    return val;
}

mk_number(ppnode) 
    struct node *ppnode;
{
    extern struct node *add_ref();
    extern struct node *new_node();

    /* struct node { int type; int dummy; node* type; int val; }; */
    auto struct node *node = new_node('num', 0), *type;
    auto char *nptr;
    auto int c;

    /* At present we only support integer constants.  Floats need 
     * recognising and treating separately. */
    extern errno;
    errno = 0;
    set_op( node, 0, strtoul( node_str(ppnode), &nptr, 0 ) );
    if ( errno ) error("Overflow in integer constant");

    /* We only use the implicit int type if we don't have a type suffix.
     * Conceptually a suffix makes it no longer implicit.  And practically
     * we don't want to alter the implicit int type. */
    if ( !rchar(nptr, 0) )
        type = add_ref( implct_int() );
    else {
        type = new_node('dclt', 3);
        set_op( type, 0, new_node('int', 0) );
    } 
    set_type( node, type );

    /* Process type suffixes */
    while ( c = rchar(nptr, 0) ) {
        if ( (c == 'u' || c == 'U') && !node_op( type, 2 ) )
            set_op( type, 2, new_node('unsi', 0) );
        else if ( (c == 'l' || c == 'L') && !node_op( type, 1 ) )
            set_op( type, 1, new_node('long', 0) );
        else break;
        ++nptr;
    }

    /* TODO: Check for signed overflow.
     * Also, hex constants without 'u' may still be unsigned. */

    if (c) error("Unexpected character '%c' in number \"%s\"",
                 c, node_str(ppnode) );

    return node;
}

node_streq(node, str)
    struct node* node;
    char* str;
{
    return node_code(node) == 'id' && strcmp( node_str(node), str ) == 0;
}

pp_end_expr() {
    extern struct node* new_node();
    /* We need to overwrite the existing token, as it is still the last
     * one on the stack (so that we didn't call next() and lose the '\n'.  */
    token = new_node('prgm', 2);
    set_op(token, 0, new_strnode('id', "RBC") );
    set_op(token, 1, new_strnode('id', "end_expr") );
}

set_token(node)
    struct node *node;
{
    free_node(token);
    token = node;
}
