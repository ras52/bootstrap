/* scanner.c  --  code for tokenising the C input stream
 *
 * Copyright (C) 2013 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */


/* Skips over a C-style comment (the opening / and * are already read). */
static
skip_ccomm() {
    auto c;
    do {
        do c = getchar();
        while ( c != -1 && c != '*' );
        c = getchar();
    } while ( c != -1 && c != '/' );
    if ( c == -1 ) _error();
}

/* Skips over whitespace, including comments, and returns the next character
 * without ungetting it. */
static
skip_white() {
    auto c;
    while (1) {
        do c = getchar();
        while ( isspace(c) ); 

        if (c == '/') {
            auto c2 = getchar();
            if ( c2 != '*' ) {
                ungetchar(c2);
                return c;
            }
            skip_ccomm();
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
    /* Argument is:  struct node { int type; char str[]; } */
    
    auto keywords[15] = {
        "auto", "break", "case", "continue", "default", "do", "else", 
        "extern", "goto", "if", "return", "static", "switch", "while", 0
    };

    auto i = 0, str = node + 4;
    while ( keywords[i] && strcmp(keywords[i], str) != 0 )
        ++i;

    /* Set node->type; */
    node[0] = keywords[i] ? *keywords[i] : 'id';
    return node;
}

/* Create a node, NODE, which will be returned; read a word (i.e. identifier 
 * or keyword) starting with C (which has already been checked by isidchar1) 
 * into NODE->str, set NODE->type, and return NODE. */
static
get_word(c) {
    auto i = 0, len = 32;
    auto node = malloc(4 + len);  /* struct node { int type; char str[]; } */
    auto str = node + 4;

    lchar( str, i++, c );
    while (1) { 
        while ( i < len && isidchar( c = getchar() ) )
            lchar( str, i++, c );
        if ( i < len )
            break;
        len *= 2;
        node = realloc( node, 4 + len );
        str = node + 4;
    } 

    lchar( str, i, 0 );
    ungetchar(c);
    return chk_keyword( node );
}


/* Read a number (in octal, hex or decimal) starting with digit C into BUF, 
 * which must be 16 bytes long (or longer). */
static
read_number(buf, c) {
    auto i = 0;
    if ( c == '0' ) {
        lchar( buf, i++, c );
        c = getchar();
        /* Is it hex? */
        if ( c == 'x' ) {
            lchar( buf, i++, c );
            do lchar( buf, i++, c );
            while ( i < 16 && isxdigit( c = getchar() ) );
        }
        /* Is it octal? */
        else if ( isdigit(c) ) {
            do lchar( buf, i++, c );
            while ( i < 16 && isdigit( c = getchar() ) );
        }
        /* else it's a literal zero */
    }
    /* It must be a decimal */
    else {
        do lchar( buf, i++, c );
        while ( i < 16 && isdigit( c = getchar() ) );
    }
    if ( i == 16 ) _error();
    lchar( buf, i, 0 );
    ungetchar(c);
}

/* Create a node, NODE, which will be returned; read a number (oct / hex / dec)
 * starting with character C (which has already been checked by isdigit), 
 * parse it into NODE->val, set NODE->type, and return NODE. */
static
get_number(c) {
    auto buf[16], nptr;
    auto node = malloc(8); /* struct node { int type; int val; }; */

    read_number( buf, c );
    node[0] = 'num';
    node[1] = strtol( buf, &nptr, 0 );
    if ( rchar(nptr, 0) ) _error();
    return node;
}

/* Read an operator, starting with character C, and return a node with
 * NODE->type set to the operator as a multicharacter literal. */
static
get_multiop(c) {
    auto mops[19] = { 
        /* 0     1     2     3     4     5     6     7     8     9 */
        '++', '--', '<<', '>>', '<=', '>=', '==', '!=', '&&', '||',
        '*=', '%=', '/=', '+=', '-=', '&=', '|=', '^=',  0 
    };

    auto node = malloc(4); /* struct node { int type; } */

    {   /* Fetch the second character */
        auto c2 = getchar();
        if ( c2 == -1 ) return c;
        else lchar( &c, 1, c2 );
    }

    /* Search for two-character operator name in the mops[] list, above */
    auto i = 0;
    while ( mops[i] && mops[i] != c )
        ++i;

    /* Not found: it is a single character operator */
    if ( mops[i] == 0 ) {
        ungetchar( rchar( &c, 1 ) );
        node[0] = rchar( &c, 0 );
        return node;
    }

    /* Is it a <<= or >>= operator? */
    if ( c == '<<' || c == '>>' ) {
        auto c2 = getchar();
        if ( c2 == '=' ) lchar( &c, 2, c2 );
        else ungetchar(c2);
    }

    node[0] = c;
    return node;
}

/* Read a string or character literal into VALUE, beginning with quote mark, 
 * Q, and return the appropriate token type ('str' or 'chr'). */
static
get_qlit(q) {
    auto i = 0, len = 32, c;
    auto node = malloc(4 + len); /* struct node { int type; char str[]; } */
    auto str = node + 4;

    lchar( str, i++, q );
    while (1) {
        while ( i < len-1 && (c = getchar()) != -1 && c != q ) {
            lchar( str, i++, c );
            if ( i < len-1 && c == '\\' ) {
                if ( (c = getchar()) == -1 ) _error();
                lchar( str, i++, c );
            }
        }
        if ( i < len-1 )
            break;
        len *= 2;
        node = realloc( node, 4 + len );
        str = node + 4;
    }


    if ( i == len-1 || c == -1 ) _error();
    lchar( str, i++, q );
    lchar( str, i++, 0 );
    node[0] = q == '"' ? 'str' : 'chr';
    return node;
}

next() {
    auto c = skip_white();
    if ( c == -1 )
        return 0;
    else if ( isidchar1(c) )
        return get_word(c);
    else if ( isdigit(c) )
        return get_number(c);
    else if ( c == '\'' || c == '"' )
        return get_qlit(c);
    else 
        return get_multiop(c);
}
