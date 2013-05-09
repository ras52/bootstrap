/* syncheck.c  --  syntax checking beyond the grammar
 *
 * Copyright (C) 2013 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */

req_lvalue(node) {
    auto type = node[0], dummy;

    if ( type == 'num' || type == 'chr' || type == 'str' )
        error("Literal used as an lvalue");

    /* This will need reworking as the C standard considers arrays to 
     * be lvalues -- i.e. our concept of an lvalue is at odd's with std C's */
    else if ( type == 'id' ) {
        if ( !lookup_sym( &node[2], &dummy ) )
            error("Non-lvalue identifier '%s' where lvalue is required",
                  &node[2]);
    }

    else if ( type != '*' && type != '[]')
        error("Non-lvalue expression used as lvalue");
}

/* Static types */
static s_int = 0;

init_stypes() {
    s_int = new_node('dclt');
    s_int[1] = 4; /* arity */
    s_int[3] = new_node('int');
}

fini_stypes() {
    free_node(s_int);
}


implct_int() {
    return s_int;
}

expr_type(node) {
    auto t = node[0];

    if ( t == 'chr' || t == 'num' )
        return s_int;
    else if ( t == 'id' )
        return lookup_type( &node[2] );

    /* 'str' =>  char[N] */

    else if ( t == '++' || t == '--' ) /* Both postfix and prefix */
        return expr_type( node[2] );

    else if ( t == '[]' || t == '*' && node[1] == 1 ) {
        auto op_type = expr_type( node[2] );

        if ( op_type[0] == '[]' || op_type[0] == '*' && op_type[1] == 1 )
            return op_type[2];  /* strip the * or [] from the type */
        /* For (hopefully temporary) compatibility with stage-4, we allow an 
         * implicit int to be dereferenced.  Explicit int is not so treated. */
        else if ( op_type == s_int )
            return op_type;
        else
            int_error( "Unknown type in dereference: '%Mc'", op_type[0] );
    }

    int_error( "Unknown type" );
    return 0;
}

req_pointer(node) {
    auto type = expr_type(node), t = type ? type[0] : 0;

    /* For (hopefully temporary) compatibility with stage-4, we allow an 
     * implicit int to be a pointer.  Explicit int is not so treated. */
    if ( !( t == '[]' || t == '*' && type[1] == 1 || type == s_int ) )
        error( "Attempt to dereference a non-pointer type" );
}
