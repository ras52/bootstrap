/* type.c  --  type checking beyond the grammar
 *
 * Copyright (C) 2013 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */

static
req_lvalue(node) {
    auto type = node[0];

    if ( type == 'num' || type == 'chr' || type == 'str' )
        error("Literal used as an lvalue");

    /* This will need reworking as the C standard considers arrays to 
     * be lvalues -- i.e. our concept of an lvalue is at odd's with std C's */
    else if ( type == 'id' ) {
        if ( !lookup_sym( &node[3], 0 ) )
            error("Non-lvalue identifier '%s' where lvalue is required",
                  &node[3]);
    }

    /* This isn't quite right either.  The C standard says the result of 
     * the . operator is an lvalue iff its first argument is one. */
    else if ( type != '*' && type != '[]' && type != '.' && type != '->' )
        error("Non-lvalue expression used as lvalue");
}

/* Static types */
static s_int = 0;
static s_uint = 0;
static s_char = 0;
static s_ulong = 0;

init_stypes() {
    s_int = new_node('dclt', 3);
    s_int[3] = new_node('int', 0);

    s_uint = new_node('dclt', 3);
    s_uint[3] = add_ref( s_int[3] );
    s_uint[5] = new_node('unsi', 0);

    s_char = new_node('dclt', 3);
    s_char[3] = new_node('char', 0);

    s_ulong = new_node('dclt', 3);
    s_ulong[3] = add_ref( s_int[3] );
    s_ulong[4] = new_node('long', 0);
    s_ulong[5] = add_ref( s_uint[5] );
}

fini_stypes() {
    free_node(s_int);
    free_node(s_uint);
    free_node(s_char);
    free_node(s_ulong);
}


implct_int() {
    return s_int;
}

size_t_type() {
    /* The size_t type is also defined in <bits/size_t.h>. */
    return s_uint;
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

    /*else if ( t == 'id' )
        return type_size( lookup_type( &type[3] ) );*/

    else if ( t == 'stru' )
        return struct_size( &type[3][3] );

    else int_error( "Cannot determine size of unknown type: %Mc", type[0] );
}

is_pointer(type) {
    auto t = type[0];
    return t == '*' || t == '[]';
}

/* For (hopefully temporary) compatibility with stage-4, we allow an 
 * implicit int to be dereferenced like a pointer.  Explicit int is not 
 * so treated. */
static
can_deref(type) {
    extern compat_flag;
    return is_pointer(type) || compat_flag && type == s_int;
}

is_integral(type) {
    /* At present, all dclt types are integral.  
     * Floats and enums may change that. */
    return type[0] == 'dclt';
}

static 
is_arith(type) {
    /* At present, all dclt types are arithmetic.  
     * Enums may change that. */
    return type[0] == 'dclt';
}

static
elt_cmp(elt1, elt2) {
    return !elt1 && !elt2 || elt1 && elt2 && elt1[0] == elt2[0];
}

/* Test whether types are compatible */
static
are_compat(type1, type2) {
    /* "Two types have compatible type if their types are the same. 
     * Additional rules for determining whether two types are compatible
     * are described in 6.5.2 for type specifiers, in 6.5.3 for type 
     * qualifiers, and in 6.5.4 for declarators."  [C90 6.1.2.6; 
     * same wording in C99 6.2.7, except clause numbers.] 
     *
     * At the moment we ignore the additional rules. */

    if ( type1[0] == 'dclt' && type2[0] == 'dclt' )
        return elt_cmp( type1[3], type2[3] )
            && elt_cmp( type1[4], type2[4] )
            && elt_cmp( type1[5], type2[5] );

    else if ( type1[0] == 'stru' && type2[0] == 'stru' )
        /* TODO: Check rules on compatible structs.  We require them
         * to have the same tag name here. */
        return strcmp( node_str( type1[3] ), node_str( type2[3] ) ) == 0;

    else if ( type1[0] == '*' && type2[0] == '*' )
        /* "For two pointer types to be compatible, both shall be identically
         * qualified and both shall be pointers to compatible types." 
         * [C90 6.5.4.1]  Note we don't support type qualifiers yet. */
        return are_compat( type1[3], type2[3] );

    else if ( type1[0] == '[]' && type2[0] == '[]' )
        /* "For two array types to be compatible, both shall have compatible
         * element types, and if both size specifiers are present, they shall
         * have the same value."  [C90 6.5.4.2] */
        return are_compat( type1[3], type2[3] )
            && ( !type1[4] || !type2[4] 
                 || type1[4] == 'num' && type2[4] == 'num' 
                 && type1[4][3] == type1[4][3] );

    else if ( type1[0] == '()' && type2[0] == '()' )
        /* "For for function types to be compatible, both shall specify
         * compatible return types." [C90 6.5.4.3]   Further constraints
         * apply to parameters which are not implemented yet. */
        return are_compat( type1[3], type2[3] );

    else return 0;
}

chk_assign(node) {
    extern compat_flag;
    req_lvalue( node[3] );

    if ( node[0] == '=' ) {
        auto type1 = node[3][2], type2 = node[4][2];
        if ( is_arith(type1) && is_arith(type2) )
            ;
        else if ( is_pointer(type1) && is_pointer(type2) ) {
            if ( compat_flag && type1[3][0] != 'stru' && type2[3][0] != 'stru' )
                ;
            else if ( !are_compat(type1[3], type2[3]) )
                error("Cannot assign from an incompatible pointer");
        }
        else if ( compat_flag && type1 == s_int )
            /* Allow assignment to implicit int with --compatibility=4 */ ;
        else if ( type2 == s_int && node[4][0] == 'num' && node[4][3] == 0 )
            /* Allow '0' as a null-pointer literal */ ;
        else
            error("Cannot assign from an incompatible type");
    }

    node[2] = add_ref( node[3][2] );
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
        auto buf[8];
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

chk_member(node) {
    auto type1 = node[3][2];

    if ( node[0] == '->' ) {
        if ( !is_pointer(type1) ) 
            error( "Attempted to dereference a non-pointer type" );

        type1 = type1[3];
    }

    if ( type1[0] != 'stru' )
        error( "Attempted member access of a non-struct type: %Mc", type1[0] );
    if ( type1[3][0] != 'id' )
        int_error("Struct name is not an identifier");
    if ( node[4][0] != 'id' )
        int_error("Member name is not an identifier");

    /* Stored the member offset as an integer in node[5]; it won't be deleted
     * because the arity is 2, and node[5] is the 3rd arg slot. */
    node[2] = add_ref( member_type( &type1[3][3], &node[4][3], &node[5] ) );
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

static
op_type_err(type, op) {
    auto char buf[32];
    print_type(buf, 32, type);
    error("Invalid operand type '%s' to %Mc operator", buf, op);
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

static
req_type( type_pred, type, op ) 
    int (*type_pred)();
{
    if ( !type_pred(type) )
        op_type_err(type, op);
}

static
req_type2( type_pred, type1, type2, op )
    int (*type_pred)();
{
        req_type( type_pred, type1, op );
        req_type( type_pred, type2, op );
}

/* Type checking for multiplicative operators (*, / and %) */
chk_mult(p) {
    auto type1 = p[3][2], type2 = p[4][2], op = p[0];
   
    if ( op == '%' )
        req_type2( is_integral, type1, type2, op );
    else 
        req_type2( is_arith, type1, type2, op );

    p[2] = add_ref( usual_conv(type1, type2) );
}

/* Type checking for binary bit operators (&, | and ^). */
chk_bitop(p) {
    auto type1 = p[3][2], type2 = p[4][2];
    req_type2( is_integral, type1, type2, p[0] );
    p[2] = add_ref( usual_conv(type1, type2) );
}

/* If TYPE is an array type, promote it to a pointer; otherwise return 
 * a copy calling add_ref() on it. */
static
prom_array(type) {
    if ( type[0] == '[]' ) {
        auto ptr = new_node( '*', 1 );
        ptr[3] = add_ref( type[3] );
        return ptr;
    } 
    else return add_ref(type);
}

/* Determine the type of an + or - expression. */
chk_add(p) {
    extern compat_flag;
    auto type1 = p[3][2], type2 = p[4][2], op = p[0];

    /* Handle case of: ptr +/- int. */
    if (is_pointer(type1) && is_integral(type2)) 
        p[2] = prom_array(type1);

    /* Canonicalise case of: int + ptr => ptr + int. */
    else if (op == '+' && is_pointer(type2) && is_integral(type1)) {
        auto tmp = p[3]; p[3] = p[4]; p[4] = tmp;
        p[2] = prom_array(type2);
    }

    /* Handle case of: ptr - ptr. */
    else if (op == '-' && is_pointer(type1) && is_pointer(type2)) {
        /* TODO: Properly check pointers are compatible.  
         * But for now, just check they have the same size. */
        if ( type_size( type1[3] ) != type_size( type2[3] ) )
            error("Cannot take difference of incompatible pointers");

        /* We choose ptrdiff_t to be int. */
        p[2] = add_ref( implct_int() );
    }

    else {
        req_type2( is_arith, type1, type2, op );
        p[2] = add_ref( usual_conv(type1, type2) );
    }

    if ( compat_flag && is_pointer( p[2] ) && type_size( p[2][3] ) != 1 ) {
        auto buf[8];
        print_type( buf, 32, p[2][3] );
        error( "Element size has changed since stage-4: %s", buf );
    }
}

/* Type checking for call expressions */
chk_call(p) {
    /* If the thing being called is an identifier and its type is null, 
     * the function is undeclared and it is assumed to return int. */
    if ( p[3][0] == 'id' && !p[3][2] )
        p[2] = add_ref( implct_int() );

    else {
        auto t = p[3][2];
    
        /* Is it a function? */
        if ( t[0] == '()' )
            p[2] = add_ref( t[3] );

        /* or a pointer to a function? */
        else if ( t[0] == '*' && t[3][0] == '()' )
            p[2] = add_ref( t[3][3] );

        else error("Attempt to call a non-function");
    }
}

/* Type checking for prefix or postfix increment or decrement operators */
chk_incdec(p) {
    extern compat_flag;

    req_lvalue( p[3] );

    /* Its type is its argument's type. */
    p[2] = add_ref( p[3][2] );

    /* In stage-4, increments were always done as if on an int.  This 
     * means that int* now increments differently.  Warn about that. */
    if ( compat_flag && is_pointer( p[2] ) && type_size( p[2][3] ) != 1 ) {
        auto buf[8];
        print_type( buf, 32, p[2][3] );
        error( "Element size has changed since stage-4: %s", buf);
    }
        
}

chk_addr(p) {
    req_lvalue( p[3] );

    if ( p[3][0] == 'id' ) {
        auto strg = lookup_strg( &p[3][3] );
        if ( strg && strg[0] == 'regi' )
            error("Cannot take address of variable with register storage");
    }

    p[2] = new_node('*', 1);
    p[2][3] = add_ref( p[3][2] );
}

/* Type checking for shift operators */
chk_shift(p) {
    auto type1 = p[3][2], type2 = p[4][2];

    /* Both arguments shall be integers */
    req_type2( is_integral, type1, type2, p[0] );

    /* The standard says the rhs is promoted, but this is irrelevant,
     * as only values 0 <= rhs < 32 are valid, and when we do the shift
     * only the bits in %cl are actually used. */

    /* The type of a shift expression is the lhs type, promoted. */
    p[2] = add_ref( prom_type( p[3][2] ) );
}

chk_comma(p) {
    /* The type of a shift expression is the rhs type, unpromoted. */
    p[2] = add_ref( p[4] );
}

chk_int(p) {
    /* TODO: Replace calls to this with something more appropriate. */
    p[2] = add_ref( implct_int() );
}

chk_cmp(p) {
    auto type1 = p[3][2], type2 = p[4][2];
    if ( is_integral(type1) && is_integral(type2) )
        ;
    /* TODO: Check types are compatible */
    else if ( is_pointer(type1) && is_pointer(type2) )
        ;
    else
        error("Unable to compare incompatible types");
    p[2] = add_ref( implct_int() );
}

static
print_dclt(buf, sz, t) {
    if (t[5]) {
        if ( t[5][0] == 'unsi' ) strlcat(buf, "unsigned", sz);
        else if ( t[5][0] == 'sign' ) strlcat(buf, "signed", sz);
        strlcat(buf, " ", sz);
    }
    if (t[4]) {
        if ( t[4][0] == 'long' ) strlcat(buf, "long", sz);
        else if ( t[4][0] == 'shor' ) strlcat(buf, "short", sz);
        strlcat(buf, " ", sz);
    }
    if ( t[3][0] == 'int' ) strlcat(buf, "int", sz);
    else if ( t[3][0] == 'char' ) strlcat(buf, "char", sz);
}

static
print_type1(buf, sz, t) {
    if (!t) strlcat(buf, "int", sz);
    else if (t[0] == 'dclt') print_dclt(buf, sz, t);
    else if (t[0] == '*') {
        print_type1(buf, sz, t[3]); 
        if (t[3] == '[]' || t[3] == '()') strlcat(buf, "(", sz);
        strlcat(buf, "*", sz); 
    }
    else if (t[0] == '[]') { print_type1(buf, sz, t[3]); }
    else if (t[0] == '()') { print_type1(buf, sz, t[3]); }
}

static
print_type2(buf, sz, t) {
    if (t[0] == '*') {
        if (t[3] == '[]' || t[3] == '()') strlcat(buf, ')', sz);
        print_type2(buf, sz, t[3]);
    }
    else if (t[0] == '[]') {
        strlcat(buf, "[]", sz);
        print_type2(buf, sz, t[3]); 
    }
    else if (t[0] == '()') {
        strlcat(buf, "()", sz);
        print_type2(buf, sz, t[3]); 
    }
}

print_type(buf, sz, t) {
    lchar(buf, 0, '\0');
    print_type1(buf, sz, t);
    print_type2(buf, sz, t);
    return buf;
}
