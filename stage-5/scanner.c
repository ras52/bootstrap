/* scanner.c  --  code for converting preprocessor tokens to C ones
 *
 * Copyright (C) 2013 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */


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
         * TODO: Not yet implemented: double, typedef, union. 
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

/* Create a node, NODE, which will be returned; read a number (oct / hex / dec)
 * starting with character C (which has already been checked by isdigit), 
 * parse it into NODE->val, set NODE->type, and return NODE. */
static
get_number(stream, c, c2) {
    auto char *nptr;
    auto ppnode = get_ppnum(stream, c, c2);

    /* struct node { int type; int dummy; node* type; int val; }; */
    auto node = new_node('num', 0);


    /* At present we only support integer constants.  Floats need 
     * recognising and treating separately. */
    extern errno;
    errno = 0;
    node[3] = strtoul( &ppnode[3], &nptr, 0 );
    if ( errno ) error("Overflow in integer constant");

    /* We only use the implicit int type if we don't have a type suffix.
     * Conceptually a suffix makes it no longer implicit.  And practically
     * we don't want to alter the implicit int type. */
    if ( !rchar(nptr, 0) )
        node[2] = add_ref( implct_int() );
    else {
        node[2] = new_node('dclt', 3);
        node[2][3] = new_node('int', 0);
    } 

    /* Process type suffixes */
    while ( c = rchar(nptr, 0) ) {
        if ( (c == 'u' || c == 'U') && !node[2][5] )
            node[2][5] = new_node('unsi', 0);
        else if ( (c == 'l' || c == 'L') && !node[2][4] )
            node[2][4] = new_node('long', 0);
        else break;
        ++nptr;
    }

    /* TODO: Check for signed overflow.
     * Also, hex constants without 'u' may still be unsigned. */

    if (c)
        error("Unexpected character '%c' in number \"%s\"",
              c, &ppnode[3]);

    free_node(ppnode);
    return node;
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

static input_strm;

/* Contains the token most recently read by next() */
static token;

/* Contains a single push-back token */
static pb_token;

/* Read the next lexical element as a syntax tree node */
static
do_next() {
    auto stream = input_strm, c;
    do {
        c = skip_white(stream);
        if ( c == -1 )
            token = 0;
        else if ( c == '\n#' )
            start_ppdir(stream);
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
        else if ( c == '\'' || c == '"' ) {
            auto int l;
            token = get_qlit(stream, c, &l);
    
            /* Character literals have type int in C. */
            if (c == '\'') 
                token[2] = add_ref( implct_int() );
            /* String literals have type char[N] */
            else 
                token[2] = chr_array_t(l);
        }
        else 
            token = get_multiop(stream, c);
    } while ( c == '\n#' );
}

static
next() {
    if (pb_token) {
        token = pb_token;
        pb_token = 0;
    }
    else do_next();
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
    next();
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
    if (!token) error("Unexpected end of file");
    return token;
}

peek_token() {
    return token ? token[0] : 0;
}

/* Handle a #pragma directive */
prgm_direct(stream) {
    auto struct node* tok;
    auto char* str;
    auto int c = skip_hwhite(stream);

    /* The standard requires unrecognised #pragmas to be allowed, but
     * this is a bit silly. */
    if ( !isidchar1(c) ) {
        warning("Unfamiliar form of #pragma directive");
        pp_slurp(stream);
        return;
    }
   
    tok = get_word(stream, c);
    str = node_str(tok);

    /* Our #pragmas all live in the RBC namespace (for Richard's Bootstrap 
     * Compiler). */
    if ( strcmp( str, "RBC" ) != 0 ) {
        /* An unknown pragma: silently ignore it. */
        pp_slurp(stream);
        free_node(tok);
        return;
    }
    free_node(tok);

    c = skip_hwhite(stream);
    if ( !isidchar1(c) )
        error("#pragma RBC requires a command argument");
    tok = get_word(stream, c);
    str = node_str(tok);
    
    /* We only know about #pragma RBC compatibility */
    if ( strcmp( str, "compatibility" ) == 0 ) {
        extern compat_flag;
        auto int n = pp_dir_num(stream);
        if ( n < 4 || n > 5 )
            error("Compatibility with stage %d not supported", n);
        compat_flag = ( n == 4 );
        free_node(tok);
    }
    else {
        warning("Unhandled #pragma RBC %s", str);
        pp_slurp(stream);
        free_node(tok);
        return;
    }
}


