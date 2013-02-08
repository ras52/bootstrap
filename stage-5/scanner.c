/* scanner.c  --  code to tokenising C input stream
 *
 * Copyright (C) 2013 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */

token = 0;

static __value[20];  /* 80 bytes */
value = __value;

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

/* Check whether the null-terminated string, VALUE, is a keyword, and if so
 * return the keyword token (which is a multicharacter literal containing
 * at most the first four characters of the keyword, e.g. 'whil' for "while"); 
 * otherwise return 'id' for an identifier. */
static
chk_keyword() {
    auto keywords[15] = {
        "auto", "break", "case", "continue", "default", "do", "else", 
        "extern", "goto", "if", "return", "static", "switch", "while", 0
    };

    auto i = 0;
    while ( keywords[i] && strcmp(keywords[i], value) != 0 )
        ++i;

    if ( keywords[i] ) return *keywords[i];
    else return 'id';
}

/* Read a word (i.e. identifier or keyword) starting with C (which passes
 * isidchar1(c)) into VALUE, and return the token type ('id' or the 
 * appropriate keyword token, e.g. 'whil'). */
static
get_word(c) {
    auto i = 0;
    lchar( value, i++, c );
    while ( i < 80 && isidchar( c = getchar() ) )
        lchar( value, i++, c );
    if ( i == 80 ) _error();
    else lchar( value, i, 0 );
    ungetchar(c);
    return chk_keyword();
}

/* Read a number (in octal, hex or decimal) starting with digit C into VALUE, 
 * and return the token type, 'num'. */
static
get_number(c) {
    auto i = 0;
    if ( c == '0' ) {
        lchar( value, i++, c );
        c = getchar();
        /* Is it hex? */
        if ( c == 'x' ) {
            lchar( value, i++, c );
            do lchar( value, i++, c );
            while ( isxdigit( c = getchar() ) );
        }
        /* Is it octal? */
        else if ( isdigit(c) ) {
            do lchar( value, i++, c );
            while ( isdigit( c = getchar() ) );
        }
        /* else it's a literal zero */
    }
    /* It must be a decimal */
    else {
        do lchar( value, i++, c );
        while ( isdigit( c = getchar() ) );
    }
    lchar( value, i, 0 );
    ungetchar(c);
    return 'num';   
}

/* Read an operator, starting with C, and return it as a multicharacter 
 * literal */
static
get_multiop(c) {
    auto mops[19] = { 
        /* 0     1     2     3     4     5     6     7     8     9 */
        '++', '--', '<<', '>>', '<=', '>=', '==', '!=', '&&', '||',
        '*=', '%=', '/=', '+=', '-=', '&=', '|=', '^=',  0 
    };


    /* Search for two-character operator name in the mops[] list, above */
    auto c2 = getchar();
    if ( c2 == -1 ) return c;
    else lchar( &c, 1, c2 );

    auto i = 0;
    while ( mops[i] && mops[i] != c )
        ++i;

    /* Not found: it is a single character operator */
    if ( mops[i] == 0 ) {
        ungetchar( rchar( &c, 1 ) );
        return rchar( &c, 0 );
    }

    if ( c == '<<' || c == '>>' ) {
        auto c2 = getchar();
        if ( c2 == '=' ) lchar( &c, 2, c2 );
        else ungetchar(c2);
    }

    return c;
}

/* Read a string or character literal into VALUE, beginning with quote mark, 
 * Q, and return the appropriate token type ('str' or 'chr'). */
static
get_qlit(q) {
    auto i = 0, c;
    lchar( value, i++, q );
    while ( i < 79 && (c = getchar()) != -1 && c != q ) {
        lchar( value, i++, c );
        if ( i < 79 && c == '\\' ) {
            if ( (c = getchar()) == -1 ) _error();
            lchar( value, i++, c );
        }
    }

    if ( i == 79 || c == -1 ) _error();
    lchar( value, i++, q );
    lchar( value, i++, 0 );
    return q == '"' ? 'str' : 'chr';
}

next() {
    auto c = skip_white();
    if ( c == -1 )
        token = c;
    else if ( isidchar1(c) )
        token = get_word(c);
    else if ( isdigit(c) )
        token = get_number(c);
    else if ( c == '\'' || c == '"' )
        token = get_qlit(c);
    else 
        token = get_multiop(c);
    return token;
}
