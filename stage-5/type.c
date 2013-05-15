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
        if ( !is_integral(type2) )
            error( "Attempted to use a non-integral type as a subscript" );
    }
    /* C allows the subscript arguments to be either way around. 
     * If they're the unorthodox way, swap them. */
    else if ( is_integral(type1) && can_deref(type2) ) {
        auto tmp = type1; type1 = type2; type2 = tmp;
        tmp = node[3]; node[3] = node[4]; node[4] = tmp;
    }
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
    if ( compat_flag && type_size(node[2]) != 4 ) {
        auto buf[8] = {0};
        print_type( buf, 32, node[2] );
        error( "Element size has changed since stage-4: %s", buf);
    }

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

/* Do the 'usual arithmetic conversions', per C90 6.2.1.5 */
usual_conv(type1, type2) {
    /* The integral promotions get rid of any chars or shorts. 
     * And because we canonicalise types to remove 'signed' (except on char),
     * testing whether typeN[5] is non-null checks for unsigned; similarly 
     * testing whether typeN[4] is non-null checks for long.  */
    type1 = prom_type(type1);
    type2 = prom_type(type2);

    /* If either is a ulong, use that type. */
    if ( type1[5] && type1[4] ) return type1;
    if ( type2[5] && type2[4] ) return type2;

    /* If one is a uint and one is a long. 
     * Note that on this arch, sizeof(long) == sizeof(int), so a long cannot
     * represent all the values of a uint. */
    if ( type1[5] && type2[4] ) return s_ulong;
    if ( type2[5] && type1[4] ) return s_ulong;
    
    /* If one is a long. */
    if ( type1[4] ) return type1;
    if ( type2[4] ) return type2;

    /* If one is a uint. */
    if ( type1[5] ) return type1;
    if ( type2[5] ) return type2;

    /* They are both int. */
    return type1;
}

/* Set the type of a function call expression */
chk_call(p) {
    /* If the thing being called is an identifier and its type is null, 
     * the function is undeclared and it is assumed to return int. */
    if ( p[3][0] == 'id' && !p[3][2] )
        p[2] = add_ref( implct_int() );

    else {
        auto t = p[3][2]; /* lookup_type( &p[3][3] ); */
    
        /* Is it a function? */
        if ( t[0] == '()' )
            p[2] = add_ref( t[3] );

        /* or a pointer to a function? */
        else if ( t[0] == '*' && t[3][0] == '()' )
            p[2] = add_ref( t[3][3] );

        else error("Attempt to call a non-function");
    }
}

static
print_dclt(buf, sz, t) {
    if (t[5]) {
        if ( t[5][0] == 'unsi' ) strncat(buf, "unsigned", sz);
        else if ( t[5][0] == 'sign' ) strncat(buf, "signed", sz);
        strncat(buf, " ", sz);
    }
    if (t[4]) {
        if ( t[4][0] == 'long' ) strncat(buf, "long", sz);
        else if ( t[4][0] == 'shor' ) strncat(buf, "short", sz);
        strncat(buf, " ", sz);
    }
    if ( t[3][0] == 'int' ) strncat(buf, "int", sz);
    else if ( t[3][0] == 'char' ) strncat(buf, "char", sz);
}

static
print_type1(buf, sz, t) {
    if (!t) strncat(buf, "int", sz);
    else if (t[0] == 'dclt') print_dclt(buf, sz, t);
    else if (t[0] == '*') {
        print_type1(buf, sz, t[3]); 
        if (t[3] == '[]' || t[3] == '()') strncat(buf, "(", sz);
        strncat(buf, "*", sz); 
    }
    else if (t[0] == '[]') { print_type1(buf, sz, t[3]); }
    else if (t[0] == '()') { print_type1(buf, sz, t[3]); }
}

static
print_type2(buf, sz, t) {
    if (t[0] == '*') {
        if (t[3] == '[]' || t[3] == '()') strncat(buf, ')', sz);
        print_type2(buf, sz, t[3]);
    }
    else if (t[0] == '[]') {
        strncat(buf, "[]", sz);
        print_type2(buf, sz, t[3]); 
    }
    else if (t[0] == '()') {
        strncat(buf, "()", sz);
        print_type2(buf, sz, t[3]); 
    }
}

print_type(buf, sz, t) {
    print_type1(buf, sz, t);
    print_type2(buf, sz, t);
    return buf;
}
