/* scanner.c  --  code for tokenising the C input stream
 *
 * Copyright (C) 2013 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */

static line = 1;
static filename;

static
print_msg(fmt, ap) {
    extern stderr;
    fprintf(stderr, "%s:%d: ", filename, line);
    vfprintf(stderr, fmt, ap);
    fputc('\n', stderr);
}

warning(fmt) {
    print_msg(fmt, &fmt);
}

error(fmt) {
    print_msg(fmt, &fmt);
    exit(1);
}

int_error(fmt) {
    extern stderr;
    fputs("Internal error: ", stderr);
    vfprintf(stderr, fmt, &fmt);
    fputc('\n', stderr);
    exit(2);
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
chk_keyword(node) {
    /* Argument is:  struct node { int type; int dummy; char str[]; } */
    
    auto keywords[28] = {
        /* Complete list of keywords per K&R, minus 'entry'.
         * C90 adds 'const', 'enum', 'signed', 'void', and 'volatile'
         *
         * 'do' and 'if' have an extra NUL character to pad them to 4 bytes
         * for casting to an int (i.e. a multicharacter literal). */
        "auto", "break", "case", "char", "continue", "default", "do\0", 
        "double", "else", "extern", "float", "for", "goto", "if\0", 
        "int", "long", "register", "return", "short", "sizeof", "static", 
        "struct", "switch", "typedef", "union", "unsigned", "while", 0
    };

    /* TODO: Types not yet implemented: char, double, int, long, short, 
     * sizeof, struct, typedef, union, unsigned */
    /* TODO: Misc not yet implemented: register */

    auto i = 0, str = &node[2];
    while ( keywords[i] && strcmp(keywords[i], str) != 0 )
        ++i;

    if ( keywords[i] ) {
        /* Change the id node to an op node. */
        node[0] = *keywords[i];
        node[2] = node[3] = node[4] = node[5] = 0;
    }
    
    return node;
}

/* Create a node, NODE, which will be returned; read a word (i.e. identifier 
 * or keyword) starting with C (which has already been checked by isidchar1) 
 * into NODE->str, set NODE->type, and return NODE. */
static
get_word(stream, c) {
    auto i = 0, len = 32;  /* Len must be >= 16, which is the op node size */
    /* struct node { int type; int dummy; char str[]; } */
    auto node = malloc(8 + len);
    auto str = &node[2];

    node[0] = 'id';  node[1] = 0;
    lchar( str, i++, c );
    while (1) { 
        while ( i < len && isidchar( c = fgetc(stream) ) )
            lchar( str, i++, c );
        if ( i < len )
            break;
        len *= 2;
        node = realloc( node, 8 + len );
        str = &node[2];
    } 

    lchar( str, i, 0 );
    ungetc(c, stream);
    return chk_keyword( node );
}


/* Read a number (in octal, hex or decimal) starting with digit C into BUF, 
 * which must be 16 bytes long (or longer). */
static
read_number(stream, buf, c) {
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
    auto buf[16], nptr;
    /* struct node { int type; int dummy; int val; }; */
    auto node = malloc(12);

    read_number( stream, buf, c );
    node[0] = 'num';  node[1] = 0;
    node[2] = strtol( buf, &nptr, 0 );
    if ( rchar(nptr, 0) )
        error("Unexpected character '%c' in string \"%s\"",
              rchar(nptr, 0), buf);
    
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

/* struct node { int type; int nops; node* op[4]; } */
node_new(type) {
    /* For binary operators, op[0] is the lhs and op[1] the rhs; for unary 
     * prefix operators, only op[0] is used; and for unary postfix only 
     * op[1] is used. 
     * 
     * The scanner never reads a ternary operator (because ?: has two separate
     * lexical elements), but we generate '?:' nodes in the expression parser
     * and want a uniform interface.  Similarly, the 'for' node is a quaternary
     * "operator" (init, test, incr, stmt). */
    auto n = malloc(24);

    n[0] = type; n[1] = 0;

    /* The operands will get filled in by the parser */
    n[2] = n[3] = n[4] = n[5] = 0;

    return n;
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
        return node_new( rchar( &c, 0 ) );
    }

    /* Is it a <<= or >>= operator? */
    if ( c == '<<' || c == '>>' ) {
        c2 = fgetc(stream);
        if ( c2 == '=' ) lchar( &c, 2, c2 );
        else ungetc(c2, stream);
    }

    return node_new(c);
}

/* Convert a quoted character literal in STR (complete with single quotes,
 * and escapes) into an integer, and return that. */
parse_chr(str) {
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
            else if ( c == 't' ) c == '\t';
            else if ( c == '0' ) c == '\0';
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
    auto i = 0, len = 32, c;
    /* struct node { int type; int dummy; char str[]; } */
    auto node = malloc(8 + len);
    auto str = &node[2];

    lchar( str, i++, q );
    while (1) {
        while ( i < len-1 && (c = fgetc(stream)) != -1 && c != q ) {
            lchar( str, i++, c );
            if ( i < len-1 && c == '\\' ) {
                if ( (c = fgetc(stream)) == -1 ) break;
                lchar( str, i++, c );
            }
        }

        if ( c == -1 )
            error("Unexpected EOF during %s literal",
                  q == '"' ? "string" : "character");

        if ( i < len-1 )
            break;
        len *= 2;
        node = realloc( node, 8 + len );
        str = &node[2];
    }

    /* We can only reach here if i < len-1, so no need to check for overflow */
    lchar( str, i++, q );
    lchar( str, i++, 0 );
    node[0] = q == '"' ? 'str' : 'chr';
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

/* Unallocate a node */
free_node(node) {
    auto i = 0;

    if (!node) return;

    /* As a safety measure, if it's the current node, unset TOKEN. */
    if ( node == token ) token = 0;

    /* The switch table contains weak links, so don't recurse into that. */
    if ( node[0] != 'swtb' )
        while ( i < node[1] ) 
            free_node( node[ 2 + i++ ] );

    free(node);
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

