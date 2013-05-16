/* scanner.c  --  code for tokenising the C input stream
 *
 * Copyright (C) 2013 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */

static line = 1;
static filename;

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
    auto c = fgetc(stream);
    if ( c == '\n' ) ++line;
    return c;
}

/* Skips over a C-style comment (the opening / and * are already read). */
static
skip_ccomm(stream) {
    auto c;

    do {
        do c = fgetcl(stream);
        while ( c != -1 && c != '*' );
        c = fgetcl(stream);
    } while ( c != -1 && c != '/' );

    if ( c == -1 )
        error("Unexpected EOF during comment");
}

/* Skips over whitespace, including comments, and returns the next character
 * without ungetting it. */
static
skip_white(stream) {
    auto c;
    while (1) {
        do c = fgetcl(stream);
        while ( isspace(c) ); 

        if (c == '/') {
            auto c2 = fgetc(stream);
            if ( c2 != '*' ) {
                ungetc(c2, stream);
                return c;
            }
            skip_ccomm(stream);
        }
        else return c;
    }
}

/* Is C a valid character to start an identifier (or keyword)? */
static
isidchar1(c) {
    return c == '_' || isalpha(c);
}

/* Is C a valid character to continue an identifier (or keyword)? */
static
isidchar(c) {
    return c == '_' || isalnum(c);
}

/* Check whether the null-terminated string, NODE->str, is a keyword, and 
 * if so set NODE->type to the keyword token (which is a multicharacter 
 * literal containing at most the first four characters of the keyword, 
 * e.g. 'whil' for "while"); otherwise set NODE->type = 'id' for an 
 * identifier.   Returns NODE. */
static
chk_keyword(node)
{
    /* Argument is:  struct node { int type; int dummy; char str[]; } */
    
    auto char *keywords[29] = {
        /* Complete list of keywords per K&R, minus 'entry', plus 'signed'
         * from C90.  C90 also adds 'const', 'enum', 'void', and 'volatile'.
         *
         * 'do' and 'if' have an extra NUL character to pad them to 4 bytes
         * for casting to an int (i.e. a multicharacter literal). 
         *
         * TODO: Not yet implemented: double, sizeof, struct, typedef, union. 
         */
        "auto", "break", "case", "char", "continue", "default", "do\0", 
        "double", "else", "extern", "float", "for", "goto", "if\0", 
        "int", "long", "register", "return", "signed", "short", "sizeof", 
        "static", "struct", "switch", "typedef", "union", "unsigned", "while", 
        0
    };

    auto i = 0;
    while ( keywords[i] && strcmp(keywords[i], &node[3]) != 0 )
        ++i;

    if ( keywords[i] ) {
        /* Change the id node to an op node. */
        node[0] = *keywords[i];

        /* Zero the memory used by the string: it's now an node* array[4]. */
        memset( &node[3], 0, 16 );
    }
    
    return node;
}

/* Append character CHR to the payload of the node *NODE_PTR which is treated 
 * as a string with current length *LEN_PTR.  The value of *LEN_PTR is 
 * incremented.  The node may be reallocated. */
static
node_lchar( node_ptr, len_ptr, chr )
    int *len_ptr;
{
    auto node = *node_ptr;
    node = grow_node( node, *len_ptr );
    lchar( &node[3], (*len_ptr)++, chr );
    *node_ptr = node;
}

/* Create a node, NODE, which will be returned; read a word (i.e. identifier 
 * or keyword) starting with C (which has already been checked by isidchar1) 
 * into NODE->str, set NODE->type, and return NODE. */
static
get_word(stream, c) {
    /* struct node { int type; int dummy; char str[]; } */
    auto node = new_node('id', 0);
    auto i = 0;

    node_lchar( &node, &i, c );
    while ( isidchar( c = fgetc(stream) ) )
        node_lchar( &node, &i, c );

    ungetc(c, stream);
    node_lchar( &node, &i, 0 );
    return chk_keyword( node );
}


/* Read a number (in octal, hex or decimal) starting with digit C into BUF, 
 * which must be 16 bytes long (or longer). */
static
read_number(stream, buf, c) 
    char *buf;
{
    auto i = 0;
    if ( c == '0' ) {
        lchar( buf, i++, c );
        c = fgetc(stream);
        /* Is it hex? */
        if ( c == 'x' ) {
            lchar( buf, i++, c );
            do lchar( buf, i++, c );
            while ( i < 16 && isxdigit( c = fgetc(stream) ) );
        }
        /* Is it octal? */
        else if ( isdigit(c) ) {
            do lchar( buf, i++, c );
            while ( i < 16 && isdigit( c = fgetc(stream) ) );
        }
        /* else it's a literal zero */
    }
    /* It must be a decimal */
    else {
        do lchar( buf, i++, c );
        while ( i < 16 && isdigit( c = fgetc(stream) ) );
    }
    if ( i == 16 )
        error("Overflow reading number");
    
    lchar( buf, i, 0 );
    ungetc(c, stream);
}

/* Create a node, NODE, which will be returned; read a number (oct / hex / dec)
 * starting with character C (which has already been checked by isdigit), 
 * parse it into NODE->val, set NODE->type, and return NODE. */
static
get_number(stream,c) {
    auto char buf[16], *nptr;
    /* struct node { int type; int dummy; int val; }; */
    auto node = new_node('num', 0);

    read_number( stream, buf, c );
    node[3] = strtol( buf, &nptr, 0 );
    if ( rchar(nptr, 0) )
        error("Unexpected character '%c' in string \"%s\"",
              rchar(nptr, 0), buf);
    
    node[2] = add_ref( implct_int() );
    return node;
}

/* Test whether the two characters in the multicharacter literal, OP, is a 
 * valid lexical representation of an operator.  So '[]' does not  */
static
is_2charop(op) {
    /* TODO: digraphs, trigraphs, and '->' */
    auto mops[19] = { 
        /* 0     1     2     3     4     5     6     7     8     9 */
        '++', '--', '<<', '>>', '<=', '>=', '==', '!=', '&&', '||',
        '*=', '%=', '/=', '+=', '-=', '&=', '|=', '^=',  0 
    };

    /* Search for two-character operator name in the mops[] list, above */
    auto i = 0;
    while ( mops[i] && mops[i] != op )
        ++i;

    return mops[i] ? 1 : 0;
}

/* Read an operator, starting with character C, and return a node with
 * NODE->type set to the operator as a multicharacter literal. */
static
get_multiop(stream, c) {
    /* Fetch the second character */
    auto c2 = fgetc(stream);
    if ( c2 == -1 ) return c;
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

/* Convert a quoted character literal in STR (complete with single quotes,
 * and escapes) into an integer, and return that. */
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

/* Read a string or character literal into VALUE, beginning with quote mark, 
 * Q, and return the appropriate token type ('str' or 'chr'). */
static
get_qlit(stream, q) {
    auto i = 0, l = 0, c;
    /* struct node { int type; int dummy; char str[]; } */
    auto node = new_node( q == '"' ? 'str' : 'chr', 0 );

    node_lchar( &node, &i, q );
    
    while ( (c = fgetc(stream)) != -1 && c != q ) {
        node_lchar( &node, &i, c );
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

    /* Character literals have type int in C. */
    if (q == '\'') 
        node[2] = add_ref( implct_int() );
    /* String literals have type char[N] */
    else 
        node[2] = chr_array_t(l);

    return node;
}

static input_strm;

/* Contains the token most recently read by next() */
static token;

peek_char() {
    auto stream = input_strm;
    auto c = skip_white(input_strm);
    ungetc(c, stream);
    return c;
}

/* Read the next lexical element as a syntax tree node */
next() {
    auto stream = input_strm;
    auto c = skip_white(stream);
    if ( c == -1 )
        token = 0;
    else if ( isidchar1(c) )
        token = get_word(stream, c);
    else if ( isdigit(c) )
        token = get_number(stream, c);
    else if ( c == '\'' || c == '"' )
        token = get_qlit(stream, c);
    else 
        token = get_multiop(stream, c);
    return token;
}

init_scan(in_filename) {
    extern stdin;
    freopen( in_filename, "r", stdin );
    filename = in_filename;
    input_strm = stdin;
    next();
}

/* Test whether token type OP is an operator in the syntax tree. */
is_op(op) {
    /* TODO: '.', ',' */
    return strnlen(&op, 4) == 1 && strchr("&*+-~!/%<>^|=,", op)
        || is_2charop(op) || op == '?:' || op == '[]' 
        || op == '>>=' || op == '<<=';
}

/* Skip over a piece of syntax, deallocating its node */
skip_node(type) {
    if (!token)
        error("Unexpected EOF when expecting '%Mc'", type);

    else if (token[0] != type)
        error("Expected a '%Mc'", type);
    
    free_node(token);
    return next();
}

/* Take ownership of the current node, and call next() */
take_node(arity) {
    auto node = token;
    node[1] = arity;
    next();
    return node;
}

/* Require P to be non-null, and give an 'unexpected EOF' error if it is not.
 * Returns P. */
req_token() {
    if (!token) error("Unexpected EOF");
    return token;
}

peek_token() {
    return token ? token[0] : 0;
}

