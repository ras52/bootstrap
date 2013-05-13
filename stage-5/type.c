/* type.c  --  type checking beyond the grammar
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
        if ( !lookup_sym( &node[3], &dummy ) )
            error("Non-lvalue identifier '%s' where lvalue is required",
                  &node[3]);
    }

    else if ( type != '*' && type != '[]')
        error("Non-lvalue expression used as lvalue");
}

/* Static types */
static s_int = 0;
static s_char = 0;
static s_ulong = 0;

init_stypes() {
    s_int = new_node('dclt', 3);
    s_int[3] = new_node('int', 0);

    s_char = new_node('dclt', 3);
    s_char[3] = new_node('char', 0);

    s_ulong = new_node('dclt', 3);
    s_ulong[3] = add_ref( s_int[3] );
    s_ulong[4] = new_node('long', 0);
    s_ulong[5] = new_node('unsi', 0);
}

fini_stypes() {
    free_node(s_int);
    free_node(s_char);
    free_node(s_ulong);
}


implct_int() {
    return s_int;
}

chr_array_t(len) {
    auto node = new_node('[]', 2);
    node[3] = add_ref( s_char );
    node[4] = new_node('num', 0);
    node[4][3] = len;
    return node;
}

type_size(type) {
    auto t = type[0];

    if (t == '[]')
        return type_size(type[3]) * type[4][3];

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
        return type_size( lookup_type( &type[3] ) );

    else int_error( "Cannot determine size of unknown type: %Mc", type[0] );
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
    auto type1 = node[3][2], type2 = node[4][2];
    extern compat_flag;

    if ( can_deref(type1) ) {
        /*if ( !is_integral(type2) )
            error( "Attempted to use a non-integral type as a subscript" );*/
    }
    /* C allows the subscript arguments to be either way around. 
     * If they're the unorthodox way, swap them. */
    /* XXX swap the node[3] and node[4] too */
    /*else if ( is_integral(type1) && can_deref(type2) ) {
        auto tmp = type1;
        type1 = type2;
        type2 = tmp;
    }*/
    else
        error( "Attempted to dereference a non-pointer type" );
   
    /* For (hopefully temporary) compatibility with stage-4, we allow an 
     * implicit int to be dereferenced.  Explicit int is not so treated. */
    if ( compat_flag && type1 == s_int )
        node[2] = add_ref(s_int);
    else 
        node[2] = add_ref(type1[3]); 

    /* In compatibility mode, give an error if we're subscripting something 
     * that's not an array of dwords, as the elt size will change. */
    if ( compat_flag && type_size(node[2]) != 4 )
        error( "Element size has changed since stage-4" );

}

chk_deref(node) {
    extern compat_flag;
    auto type = node[3][2];
    if ( !can_deref(type) )
        error( "Attempted to dereference a non-pointer type" );

    /* For (hopefully temporary) compatibility with stage-4, we allow an 
     * implicit int to be dereferenced.  Explicit int is not so treated. */
    if ( compat_flag && type == s_int )
        node[2] = add_ref(s_int);
    else 
        node[2] = add_ref(type[3]); 

}

/* Do an integral promotion, per C90 6.2.1.1 */
prom_type(type) {
    if (!type)
        int_error("Integral promotion of null type");

    if (type[0] != 'dclt')
        int_error("Integral promotion of a unknown type: %Mc", type[0]);

    /* Only char and short types engage in promotion. */
    if (type[4] && type[4][0] == 'long' || !type[4] && type[3][0] == 'int')
        return type;

    /* Because short is smaller than int on this platform, everything
     * promotes to int rather than unsigned int. */
    else return s_int;
}

/* This tests whether both are null, or both have the same tok-type [0] */
static
node0_eq(n1, n2) {
    if (!n1) return !n2;
    else return n1[0] == n2[0];
}

static
type_eq(type1, type2) {
    if (type1[0] != 'dclt' || type2[0] != 'dclt')
        int_error("Comparison of unknown types: %Mc and %Mc",
                  type1[0], type2[0]);

    return type1[3][0] == type2[3][0]
        && node0_eq(type1[4], type2[4]) && node0_eq(type1[5], type2[5]);
}

/* Do the 'usual arithmetic conversions', per C99 6.3.1.8. */
/* TODO: Check this is correct for C90 */
usual_conv(type1, type2) {
    /* The integral promotions get rid of any chars or shorts. */
    type1 = prom_type(type1);
    type2 = prom_type(type2);

    if (type_eq(type1, type2))
        return type1;

    /* Are they both signed or both unsigned?  Because there are no chars, 
     * and signed {short,int,long}s are canonicalised to remove the signed, 
     * we just test for the presence or absence of any signed modifier. */
    if ( !type1[5] == !type2[5] )
        /* They're not equal, and shorts have been removed, so one must be 
         * a long and one an int.  Just test for a length modifier. */
        return type1[4] ? type1 : type2;

    /* We have a signed and unsigned type.  Make it so that type1 is signed. */
    if ( type1[5] ) {
        auto tmp = type1; type1 = type2; type2 = tmp;
    }

    /* "Otherwise, if the operand that has unsigned integer type has rank 
     * greater or equal to the rank of the type of the other operand" 
     * i.e. if the unsigned one is long and/or the signed one is not long. */
    if ( type2[4] || !type1[4] )
        return type2;

    /* "Otherwise, if the type of the operand with signed integer type can
     * represent all of the values of the type of the operand with unsigned 
     * integer type".   We can only now have an unsigned int and a signed long,
     * and sizeof(int) == sizeof(long), so this cannot happen.
     *
     * "Otherwise, both operands are converted to the unsigned integer type
     * corresponding to the type of the operand with signed integer type"
     * i.e. to unsigned long. */
    return s_ulong;
}
