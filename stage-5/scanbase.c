/* scanner.c  --  code for tokenising the C input stream
 *
 * Copyright (C) 2013 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */

static int line = 0;
static char* filename;

set_file(name) 
    char *name;
{
    filename = name;
}

get_line() {
    return line;
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


/* A version of fgetc() that increments LINE if a '\n' is found */
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
    do c = fgetc(stream);
    while ( c != '\n' && isspace(c) ); 
    if ( c == -1 )
        error("End of file during preprocessor directive");
    return c;
}

/* Read a number in a preprocessor directive. */
pp_dir_num(stream) {
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

/* Handle a #line directive */
static
line_direct(stream) {
    /* We subtract 1 from the line number because it'll get incremented
     * at the end of the line. */
    line = pp_dir_num(stream) - 1;
}

/* Ignore anything to the next new line */
pp_slurp(stream) {
    auto int c;
    do c = fgetc(stream);
    while ( c != -1 && c != '\n' ); 
    if ( c == -1 )
        error("End of file during preprocessor directive");
    ungetc(c, stream);
    return c;
}

/* We've just read a #.  Read the rest of the directive. */
start_ppdir(stream) {
    auto struct node *tok, *ret = 0;
    auto char* str;
    auto int c = skip_hwhite(stream);

    /* The null directive -- just a '#' by itself on the line */
    if ( c == '\n' ) { 
        ungetc(c, stream); 
        return ret; 
    }
   
    /* In principle this matches the non-directive syntax of C99 / C++1x,
     * but that's for use as extensions only, and we unilaterally decide
     * that all extensions will begin with an identifier (like 'include'). */
    if ( !isidchar1(c) ) 
        error("Malformed preprocessor directive");

    tok = get_word(stream, c);
    str = node_str(tok);

    if ( strcmp( str, "line" ) == 0 )
        line_direct(stream);

    else if ( strcmp( str, "pragma" ) == 0 )
        ret = prgm_direct(stream);

    else
        error("Unknown preprocessor directive: %s", str);
    free_node(tok);

    /* Eat all trailing space */
    c = skip_hwhite(stream);
    if ( c == -1 )
        error( "End of file during preprocessor directive" );
    else if ( c != '\n' )
        error( "Unexpected content at end of #%s directive", str );
    ungetc(c, stream);

    return ret;
}

/* Skips over whitespace, including comments, and returns the next character
 * without ungetting it. */
skip_white(stream) {
    auto c, line_start = 0;
    /* Are we at the start of the file? */
    if (!line) line = line_start = 1;
        
    while (1) {
        do c = fgetcl(stream);
        while ( c != '\n' && isspace(c) ); 

        if (c == '\n') {
            line_start = 1;
            continue;
        } 
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
        else return c;

        line_start = 0;
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

static input_strm;

/* Contains the token most recently read by next() */
static token;

/* Contains a single push-back token */
static pb_token;

/* Read the next lexical element as a syntax tree node */
next() {
    auto stream = input_strm, c;
    if (pb_token) {
        token = pb_token;
        pb_token = 0;
    }
    else do {
        c = skip_white(stream);
        if ( c == -1 )
            token = 0;
        else if ( c == '\n#' ) {
            /* If start_ppdir() returns a token, then we return that.
             * This happens with #pragma in the preprocessor. */
            if ( token = start_ppdir(stream) ) c = 0;
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
    } while ( c == '\n#' );
    return token;
}

unget_token(t) {
    if (pb_token) int_error("Token push-back slot already in use");
    pb_token = token;
    token = t;
}

init_scan(in_filename) {
    extern stdin;
    freopen( in_filename, "r", stdin );
    set_file( in_filename );
    input_strm = stdin;
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

