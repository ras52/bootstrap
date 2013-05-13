/* expr.c  --  code to parse expressions
 *
 * Copyright (C) 2013 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */

/*  constant    ::= int-const | char-const      TODO:  float, enum  
 *  primry-expr ::= identifier | constant | string-lit | '(' expression ')' */
static
primry_expr() {
    auto n, t = peek_token();
    if ( t == 'num' || t == 'str' ) 
        n = take_node(0);

    else if ( t == 'id' ) {
        n = take_node(0);
        /* If the identifier is undeclared, the type will be null. 
         * This happens when calling undeclared functions. */
        n[2] = lookup_type( &n[3] ); 
        if ( n[2] ) add_ref( n[2] );
    }

    else if ( t == 'chr' ) {
        n = take_node(0);
        /* Convert it into a integer literal: in C, char lits have type int. */
        n[0] = 'num';
        n[3] = parse_chr( &n[3] );
    }
    else if ( t == '(' ) {
        skip_node('(');
        n = expr(); 
        skip_node(')');
    }
    else
        error("Unexpected token '%Mc' while parsing expression", t);
    
    return n;
}

/*  arg-list ::= ( expr ( ',' expr )* )? */
static
arg_list(fn) {
    auto p = new_node('()', 0);
    p = vnode_app( p, fn );

    if ( peek_token() == ')' ) 
        return p;

    while (1) {
        req_token();
        p = vnode_app( p, assign_expr() );

        /* It would be easier to code for an optional ',' at the end, but
         * the standard doesn't allow for that. */
        req_token();
        if ( peek_token() == ',' )
            skip_node(',');
        else if ( peek_token() == ')' )
            break;
    }

    return p;
}

/* Use OP as the node, set TOKEN->op0 = LHS, and check a rhs exists
 * by calling next(). Return OP.  */
static
do_binop(lhs) {
    auto node = take_node(2);
    node[3] = lhs;
    req_token();
    return node;
}

/*  postfx-seq  ::= '(' arg-list ')' | '[' expr ']' | '++' | '--'   TODO . ->
 *  postfx-expr ::= primry-expr postfix-seq* */
static
postfx_expr() {
    auto p = primry_expr(), t;

    while (1) {
        req_token();
        t = peek_token();

        /* The only valid use of an undeclared symbol is in a simple function 
         * call.   In that case, foo() is valid, but, say, (foo)() is not. */
        if ( t != '(' && p[0] == 'id' && !is_declared(&p[3]) ) 
            error("Undefined symbol '%s'", &p[3]);


        if ( t == '++' || t == '--' ) {
            req_lvalue(p);

            /* Postfix ++ and -- are marked as binary operators.  This is 
             * what distinguishes them from the prefix versions which are 
             * marked as unary.  This is inspired by C++'s idea of
             * T operator++(T&, int) vs. T& operator++(T&). */
            p = do_binop( p );

            p[2] = add_ref( p[3][2] );
        }
    
        else if ( t == '[' ) {
            p = do_binop( p );
            p[0] = '[]';
            p[4] = expr();
            skip_node(']');

            chk_subscr(p);
        }
    
        else if ( t == '(' ) {
            skip_node('(');
            p = arg_list(p);
            skip_node(')');

            chk_call(p);
        }

        else break;
    }

    return p;
}

/*  unary-op   ::= '+' | '-' | '~' | '!' | '*' | '&' | '++' | '--'
 *  unary-expr ::= unary-op unary-expr | postfx-expr 
 *  TODO: sizeof */
static
unary_expr() {
    auto t = peek_token();

    if ( strnlen(&t, 4) == 1 && strchr("+-~!*&", t) 
         || t == '++' || t == '--' ) {
        auto p = take_node(1);
        req_token();
        p[3] = unary_expr();
        if (t == '+' || t == '-' || t == '~')
            p[2] = add_ref( prom_type( p[3][2] ) );
        else if (t == '!')
            p[2] = add_ref( implct_int() );
        else if (t == '++' || t == '--' || t == '&') {
            req_lvalue( p[3] );
            if (t == '&') {
                p[2] = new_node('*', 1);
                p[2][3] = add_ref( p[3][2] );
            }
            else p[2] = add_ref( p[3][2] );
        }
        else if (t == '*')
            chk_deref(p);
        return p;
    }

    else return postfx_expr();
}

/* cast-expr ::= unary-expr
 * TODO: cast expression */
static
cast_expr() {
    return unary_expr();
}

static
test_level( t, ap ) {
    while ( (ap += 4) && *ap ) 
        if ( *ap == t )
            return 1;
    return 0;
}

/* node* bin_level( node* (*chain)(), node* (*type)(node*, node*, int), ... );
 *
 * Handle a standard left-to-right binary operator precedence level.  
 * Call CHAIN() to parse the operands, so CHAIN should be a pointer 
 * to the parser function for a higher precedence level.  Then call
 * TYPE( lhstype, rhstype, toktype ) to determine the type of expression,
 * where lhstype and rhstype are the types of the left- and right-hand
 * operands, and toktype is the tok-type of the expression (i.e. the 
 * operator). 
 *
 * It is a variadic function, and the varargs are operator tokens in 
 * the predence level, followed by a 0 end marker. */
static
bin_level( chain, type ) 
    int (*chain)(), (*type)();
{
    auto p = chain();

    /* Left-to-right associatibity: a, b, c == (a, b), c */
    while ( test_level( peek_token(), &type ) ) { 
        p = do_binop( p );
        p[4] = chain();
        p[2] = add_ref( type( p[3][2], p[4][2], p[0] ) );
    }

    return p;
}

/* mult-op   ::= '*' | '/' | '%'
   mult-expr ::= cast-expr ( mult-op cast-expr )* */
static
mult_expr() {
    extern usual_conv();
    return bin_level( cast_expr, usual_conv, '*', '/', '%', 0 );
}

/* Determine the type of an + or - expression. */
static
add_type( type1, type2, op ) {
    extern usual_conv();
    if (op == '+') {
        /* TODO: Check that the other type is integral */
        if (type1[0] == '*' || type1[0] == '[]') return type1;
        else if (type2[0] == '*' || type2[0] == '[]') return type2;
    }
    else if (op == '-' && (type1[0] == '*' || type1[0] == '[]')) {
        /* TODO: Check pointers are compatible. */
        /* We choose ptrdiff_t to be int. */
        if (type2[0] == '*' || type2[0] == '[]') return implct_int();
        /* TODO: Check that the rhs is integral */
        else return type1;
    }
    
    /* We have two integral arguments */
    return usual_conv(type1, type2);
}

/* add-op   ::= '+' | '-'
   add-expr ::= mult-expr ( add-op mult-expr )* */
static
add_expr() {
    return bin_level( mult_expr, add_type, '+', '-', 0 );
}

/* shift-op   ::= '<<' | '>>'
   shift-expr ::= add-expr ( shift-op add-expr )* */
static
shift_expr() {
    extern prom_type();
    return bin_level( add_expr, prom_type, '<<', '>>', 0 );
}

/* rel-op   ::= '<' | '>' | '<=' | '>='
   rel-expr ::= shift-expr ( rel-op shift-expr )* */
static
rel_expr() {
    extern implct_int();
    return bin_level( shift_expr, implct_int, '<', '<=', '>', '>=', 0 );
}

/* eq-op   ::= '==' | '!='
   eq-expr ::= rel-expr ( eq-op rel-expr )* */
static
eq_expr() {
    extern implct_int();
    return bin_level( rel_expr, implct_int, '==', '!=', 0 );
}

/* bitand-expr ::= eq-expr ( '&' eq-expr )* */
static
bitand_expr() {
    extern usual_conv();
    return bin_level( eq_expr, usual_conv, '&', 0 );
}

/* bitxor-expr ::= bitand-expr ( '^' binand-expr )* */
static
bitxor_expr() {
    extern usual_conv();
    return bin_level( bitand_expr, usual_conv, '^', 0 );
}

/* bitor-expr ::= bitxor-expr ( '|' binand-expr )* */
static
bitor_expr() {
    extern usual_conv();
    return bin_level( bitxor_expr, usual_conv, '|', 0 );
}

/* logand-expr ::= bitor-expr ( '&&' binand-expr )* */
static
logand_expr() {
    extern implct_int();
    return bin_level( bitor_expr, implct_int, '&&', 0 );
}

/* logor-expr ::= logand-expr ( '||' binand-expr )* */
static
logor_expr() {
    extern implct_int();
    return bin_level( logand_expr, implct_int, '||', 0 );
}

/* cond-expr ::= logor-expr ( '?' expr ':' assign-expr )? */
static
cond_expr() {
    auto p = logor_expr();

    if ( peek_token() == '?' ) {
        p = do_binop( p );
        p[0] = '?:';  p[1] = 3;
        p[4] = expr();
        skip_node(':');
        req_token();
        p[5] = assign_expr();
        p[2] = add_ref( implct_int() );
    }

    return p;
}

is_assop(t) {
    return t == '='  || t == '+=' || t == '-=' || t == '*=' || t == '/=' || 
           t == '%=' || t == '&=' || t == '|=' || t == '^=' || t == '<<=' || 
           t == '>>=';
} 

/* assign-op   ::= '=' | '+=' | '-=' | '*=' | '/=' | '%=' 
 *                     | '&=' | '|=' | '^=' | '<<=' | '>>='
 * assign-expr ::= cond-expr ( assign-op assign-expr )?  */
assign_expr() {
    /* The C grammar says the l.h.s. is actually a unary-expr, however
     * as in C a unary-expr is the lowest precedence expression that can
     * yield an lvalue, it is entirely equivalent to use a logor-expr as
     * the l.h.s., and that's exactly what the C++ grammar has.
     *
     * In fact, we can safely put a cond-expr on the l.h.s.  This introduces
     * an ambiguity for expressions like (a ? b : c = d).  Should this be 
     * parsed as ((a ? b : c) = d) or (a ? b : (c = d))?  Because we call
     * down to cond_expr() and this parses as much as possible, it is always
     * parsed as the latter, which means that we can never get a cond-expr 
     * on the l.h.s. of an assignment.
     */

    auto p = cond_expr(), t = peek_token();
    if ( is_assop(t) ) {
        req_lvalue(p);
        p = do_binop( p );
        p[4] = assign_expr();
        p[2] = add_ref( p[3][2] );
    }

    return p;
}

static
comma_type(lhs, rhs) {
    return rhs;
}

/* expr ::= ( assign-expr ',' )* assign-expr */
expr() {
    return bin_level( assign_expr, comma_type, ',', 0 );
}
