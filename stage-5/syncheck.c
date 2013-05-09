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

type_size(type) {
    auto t = type[0];

    if (t == '[]')
        return type_size(type[2]) * type[3][2];

    else if (t == '*')
        return 4;

    else if (t == 'dclt') {
        if (type[3][0] == 'char')
            return 1;
        else if (type[4] && type[4][0] == 'shor')
            return 2;           
        else
            return 4;
    }

    else if ( t == 'id' )
        return type_size( lookup_type( &type[2] ) );

    else int_error( "Unknown type: %Mc", type[0] );
}

expr_type(node) {
    auto t = node[0];
    extern compat_flag;

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
        else if ( compat_flag && op_type == s_int )
            return op_type;
        else
            int_error( "Unknown type in dereference: '%Mc'", op_type[0] );
    }


    int_error( "Calculating size of unknown type: %Mc", t );
    return 0;
}

static
can_deref(type) {
    auto t = type[0];
    extern compat_flag;

    /* For (hopefully temporary) compatibility with stage-4, we allow an 
     * implicit int to be a pointer.  Explicit int is not so treated. */
    return t == '[]' || t == '*' && type[1] == 1 || 
        compat_flag && type == s_int;
}

static
is_integral(type) {
    /* At present, all dclt types are integral.  Floats will change that. */
    return type[0] == 'dclt';
}

chk_subscr(node) {
    auto type1 = expr_type(node[2])/*, type2 = expr_type(node[3])*/;
    extern compat_flag;

    if ( can_deref(type1) ) {
        /*if ( !is_integral(type2) )
            error( "Attempted to use a non-integral type as a subscript" );*/
    }
    /* C allows the subscript arguments to be either way around. 
     * If they're the unorthodox way, swap them. */
    /*else if ( is_integral(type1) && can_deref(type2) ) {
        auto tmp = type1;
        type1 = type2;
        type2 = tmp;
    }*/
    else
        error( "Attempted to dereference a non-pointer type" );
    

    /* In compatibility mode, give an error if we're subscripting something 
     * that's not an array of dwords, as the elt size will change. */
    if ( compat_flag 
         && ( type1[0] == '[]' || type1[0] == '*' && type1[1] == 1 )
         && type_size( type1[2] ) != 4 )
        error( "Element size has changed since stage-4" );
}

chk_deref(node) {
    if ( !can_deref( expr_type(node[2]) ) )
        error( "Attempted to dereference a non-pointer type" );
}


