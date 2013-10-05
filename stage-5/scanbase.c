/* scanner.c  --  code for tokenising the C input stream
 *
 * Copyright (C) 2013 Richard Smith <richard@ex-parrot.com>
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
    fputc('\n', stderr);
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

/* Skip over whitespace other than '\n' (for the preprocessor) */
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
            if ( c2 != '*' ) {
                ungetc(c2, stream);
                return c;
            }
            skip_ccomm(stream);
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
    auto int i = 1, j = 0, c;

    while (1) {
        auto c = rchar( src, i++ );
        if ( c == '\"' ) break;
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
        auto struct node* tok = get_qlit(stream, c, 0);
        /* Don't free(filename) until after this, or we have problems reporting
         * a problem with the string, e.g. if it contains invalid escapes. */
        auto char* new_name = extract_str(tok);
        free( filename ); filename = new_name;
        free_node(tok);
    }
    else if ( c == '\n' )
        ungetc(c, stream);
}

/* Slurp anything to the next new line into *node_ptr (if not NULL),  */
pp_slurp(stream, node_ptr) {
    auto int i = 0, c;
    while (1) {
        c = fgetc(stream);
        if ( c == -1 )
            error("End of file during preprocessor directive");
        else if ( c == '\n' ) {
            ungetc(c, stream);
            break;
        }
        else if ( node_ptr )
            node_lchar( node_ptr, &i, c );
    }
    if ( node_ptr )
        /* Null terminate */
        node_lchar( node_ptr, &i, 0 );
    return c;
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

    if ( !tok )
        return 0;

    str = node_str(tok);
    if ( strcmp( str, "line" ) == 0 )
        line_direct(stream);
    else if ( strcmp( str, "pragma" ) == 0 )
        ret = prgm_direct(stream);
    else
        pp_direct(stream, str);

    free_node(tok);

    /* Eat all trailing space */
    c = skip_hwhite(stream);
    if ( c != '\n' )
        error( "Unexpected content at end of #%s directive", str );
    ungetc(c, stream);

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
            if ( c2 != '*' ) {
                ungetc(c2, stream);
                return c;
            }
            skip_ccomm(stream);
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
get_qlit(stream, q, len_ptr) 
    int* len_ptr;
{
    auto i = 0, l = 0, c;
    /* struct node { int type; int dummy; char str[]; } */
    auto node = new_node( q == '"' ? 'str' : 'chr', 0 );

    node_lchar( &node, &i, q );
    
    while ( (c = fgetc(stream)) != -1 && c != q ) {
        node_lchar( &node, &i, c );
        /* TODO: The handling of escapes is not conformant.  Also, share
         * code with parse_chr(), above. */
        if ( c == '\\' ) {
            if ( (c = fgetc(stream)) == -1 ) break;
            node_lchar( &node, &i, c );
        }
        ++l;
    }

    if ( c == -1 )
        error("Unexpected EOF during %s literal",
              q == '"' ? "string" : "character");

    node_lchar( &node, &i, q );
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

/* Read the next lexical element without preprocessing */
static
raw_next() {
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

    c = skip_white(stream);
    
    if ( c == -1 || c == '\n#' ) {
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
        token = do_get_qlit(stream, c);
    else 
        token = get_multiop(stream, c);

    return node_code( token );
}

next_skip() {
    auto c = raw_next();
    free_node( token );
    return c;
}


/* Read the next lexical element, handling preprocessing directives */
next() {
    auto c;
    do {
        c = raw_next();
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

unget_token(t) 
    struct node* t;
{
    pb_push(token);
    token = t;
}

init_scan(in_filename) {
    extern char* strdup();
    extern struct FILE* fopen();

    filename = strdup( in_filename );
    input_strm = fopen( filename, "r" );
    line = 0;
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

/* Require P to be non-null, and give an 'unexpected EOF' error if it is not.
 * Returns P. */
req_token() {
    if (!token) error("Unexpected end of file");
    return token;
}

peek_token() {
    return token ? node_code(token) : 0;
}

