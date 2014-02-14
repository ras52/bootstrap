/* cpptype.c  --  stub code to avoid the preprocessor knowing about types
 *
 * Copyright (C) 2014 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */

is_typedef( name ) {
    return 0;
}

lookup_type( node ) {
    return implct_int();
}

is_declared( name ) {
    /* For the purpose of preprocessor expression evaluation, all names 
     * can be used undeclared (c.f. C90 6.8.1, which says they're equal to 0).
     * This effectively means all identifiers are declared. */
    return 1;
}

chk_subscr( node ) {
    /* The normal logic for disallowing [] in a integral constant expression 
     * is based on the impossibility of their being arguments of any type
     * for which it would be valid.  This logic is in the C version of 
     * chk_subscr().  In the preprocessor, we give a blanket rejection. */
    error("Subscript operator not permitted in constant expression");
}

chk_member( node ) {
    /* As per chk_subscr(), the logic for C is rather round-about. */
    error("Member access not permitted in constant expression");
}

chk_addr( node ) {
    /* As per chk_subscr(), the logic for C is rather round-about. */
    error("Taking addresses not permitted in constant expression");
}

chk_deref( node ) {
    /* As per chk_subscr(), the logic for C is rather round-about. */
    error("Dereferencing not permitted in constant expression");
}

/* Operations that are already disallowed by virtue of being in a ICE */
chk_incdec( node ) {}
chk_call( node ) {}
chk_assign( node ) {}
chk_comma( node ) {}

/* Operations that are always valid because the only types are int */
chk_mult( node ) {}
chk_add( node ) {}
chk_shift( node ) {}
chk_cmp( node ) {}
chk_bitop( node ) {}
chk_int( node ) {}

type_name( node ) {
    /* This gets called in sizeof expessions and in casts.  The preprocessor
     * never sees a sizeof expression because it does not recognise keywords
     * and therefore sees sizeof(foo) as a function call.  Nor does the
     * preprocessor ever see a cast because, for similar reasons, it never
     * recognises anything as a decl spec.  Thus, in (int)foo, the (int)
     * is treated as a primary expression with int evaluation to 0, and
     * the 'foo' causes a parser error. */
    int_error("Unexpected type name in preprocessor");
}

prom_type( type ) {
    return type;
}

/* Static types */
static s_int = 0;

init_stypes() {
    s_int = new_node('dclt', 3);
    set_op( s_int, 0, new_node('int', 0) );
}

implct_int() {
    return s_int;
}

fini_stypes() {
    free_node(s_int);
}

size_t_type() {
    int_error("Use of size_t in preprocessor");
}

type_size() {
    int_error("Size of type required in preprocessor");
}

is_dclspec() {
    return 0;
}
