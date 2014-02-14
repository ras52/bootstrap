/* scanner.c  --  code for converting preprocessor tokens to C ones
 *
 * Copyright (C) 2013, 2014 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */


/* Check whether the null-terminated string, NODE->str, is a keyword, and 
 * if so set NODE->type to the keyword token (which is a multicharacter 
 * literal containing at most the first four characters of the keyword, 
 * e.g. 'whil' for "while"); otherwise set NODE->type = 'id' for an 
 * identifier.   Returns NODE. */
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
        /* Change the id node to an op node, using the first four bytes
         * of the keyword as the multicharacter node code. */
        auto int* keyword = keywords[i];
        node[0] = *keyword;

        /* Zero the memory used by the string: it's now an node* array[4]. */
        memset( &node[3], 0, 16 );
    }
    
    return node;
}

/* Create a node, NODE, which will be returned; read a number (oct / hex / dec)
 * starting with character C (which has already been checked by isdigit), 
 * parse it into NODE->val, set NODE->type, and return NODE. */
get_number(stream, c, c2) {
    auto char *nptr;
    auto ppnode = get_ppnum(stream, c, c2);
    auto node = mk_number(ppnode);
    free_node(ppnode);
    return node;
}

/* Handle a #pragma directive */
prgm_direct(stream) {
    extern char* node_str();
    extern struct node* get_word();

    auto struct node* tok;
    auto char* str;
    auto int c = skip_hwhite(stream);

    /* The standard requires unrecognised #pragmas to be allowed, but
     * this is a bit silly. */
    if ( !isidchar1(c) ) {
        warning("Unfamiliar form of #pragma directive");
        /* A bare #pragma is a bit silly too, but the grammar allows it. */
        if ( c == '\n' ) ungetc(c, stream);  
        else pp_slurp(stream, 0, 0);
        return 0;
    }
  
    /* Get the pragma namespace */ 
    tok = get_word(stream, c);
    str = node_str(tok);

    /* Our #pragmas all live in the RBC namespace (for Richard's Bootstrap 
     * Compiler). */
    if ( strcmp( str, "RBC" ) != 0 ) {
        /* An unknown pragma: silently ignore it. */
        pp_slurp(stream, 0, 0);
        free_node(tok);
        return 0;
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
    }
    else {
        warning("Unhandled #pragma RBC %s", str);
        pp_slurp(stream, 0, 0);
    }

    end_ppdir(stream, "pragma RBC");
    free_node(tok);

    /* The return is a null node*, and indicates that we have handled
     * (or ignored) the #pragma, and not to include it in the output
     * token stream produced by the scanner. */
    return 0;
}

/* Hook for handling preprocessor directives other than #line and #pragma */
pp_direct(stream, str) {
    error("Unknown preprocessor directive: %s", str);
}

do_get_qlit(stream, c1, c2) {
    auto int l;
    auto tok = get_qlit(stream, c1, c2, &l);

    /* Character literals have type int in C. */
    if (c1 == '\'') 
        tok[2] = add_ref( implct_int() );
    /* String literals have type char[N] */
    else if (c1 == '\"')
        tok[2] = chr_array_t(l);
    else
        int_error("Unknown type of quoted string: %c...%c", c1, c2);
    return tok;
}

handle_eof() {
    return 0;
}
