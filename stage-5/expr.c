/* expr.c  --  code to parse expressions
 *
 * Copyright (C) 2013 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */

/*  constant    ::= int-const | char-const      TODO:  float, enum  
 *  primry-expr ::= identifier | constant | string-lit | '(' expression ')' */
static
primry_expr() {
    auto n, t = token[0];
    if ( t == 'num' || t == 'chr' || t == 'id' | t == 'str' )
        n = take_node(0);

    else if ( t == '(' ) {
        skip_node('(');
        n = expr(); 
        skip_node(')');
    }
    else
        error("Unexpected token '%Mc' while parsing expression", t);
    
    return n;
}

/* Append node N to the vector node N, which is of size *SZ_PTR, growing
 * the vector if necessary, and returning the (possibly reallocated) vector. */
vnode_app( v, n, sz_ptr ) {
    if ( v[1] == *sz_ptr ) {
        *sz_ptr *= 2;
        v = realloc( v, (2 + *sz_ptr) * 4 );
    }

    v[ 2 + v[1]++ ] = n;

    return v;
}

/*  arg-list ::= ( expr ( ',' expr )* )? */
static
arg_list(fn) {
    auto sz = 4, p = node_new('()');
    p = vnode_app( p, fn, &sz );

    if ( token && token[0] == ')' ) 
        return p;

    while (1) {
        req_token( token );
        p = vnode_app( p, assign_expr(), &sz );

        /* It would be easier to code for an optional ',' at the end, but
         * the standard doesn't allow for that. */
        req_token( token );
        if ( token[0] == ',' )
            skip_node(',');
        else if ( token[0] == ')' )
            break;
    }

    return p;
}

/* Use OP as the node, set TOKEN->op0 = LHS, and check a rhs exists
 * by calling next(). Return OP.  */
static
do_binop(lhs, op) {
    op[1] = 2;
    op[2] = lhs;
    req_token( next() );
    return op;
}

/*  postfx-seq  ::= '(' arg-list ')' | '[' expr ']' | '++' | '--'   TODO . ->
 *  postfx-expr ::= primry-expr postfix-seq* */
static
postfx_expr() {
    auto p = primry_expr(), t;

    if (!token) return p;

    while (1) {
        if (!token) break;
        t = token[0];

        if ( t == '++' || t == '--' ) {
            req_lvalue(p);

            /* Postfix ++ and -- are marked as binary operators.  This is 
             * what distinguishes them from the prefix versions which are 
             * marked as unary.  This is inspired by C++'s idea of
             * T operator++(T&, int) vs. T& operator++(T&). */
            token[2] = p;
            p = take_node(2);
        }
    
        else if ( t == '[' ) {
            p = do_binop(p, token);
            p[0] = '[]';
            p[3] = expr();
            skip_node(']');
        }
    
        else if ( t == '(' ) {
            /* We don't currently support function pointers */
            if ( p[0] != 'id' )
                error( "Expression found where function name was expected" );
            skip_node('(');
            p = arg_list(p);
            skip_node(')');
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
    auto t = token[0];

    if ( strnlen(&t, 4) == 1 && strchr("+-~!*&", t) 
         || t == '++' || t == '--' ) {
        auto p = take_node(1);
        req_token(token);
        p[2] = unary_expr();
        if (t == '++' || t == '--' || t == '&')
            req_lvalue( p[2] );
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

/* mult-op   ::= '*' | '/' | '%'
   mult-expr ::= cast-expr ( mult-op cast-expr )* */
static
mult_expr() {
    auto p = cast_expr();

    while ( token 
            && ( token[0] == '*' || token[0] == '/' || token[0] == '%' ) ) {
        p = do_binop( p, token );
        p[3] = cast_expr();
    }

    return p;
}

/* add-op   ::= '+' | '-'
   add-expr ::= mult-expr ( add-op mult-expr )* */
static
add_expr() {
    auto p = mult_expr();

    while ( token && ( token[0] == '+' || token[0] == '-' ) ) {
        p = do_binop( p, token );
        p[3] = mult_expr();
    }

    return p;
}

/* shift-op   ::= '<<' | '>>'
   shift-expr ::= add-expr ( shift-op add-expr )* */
static
shift_expr() {
    auto p = add_expr();

    while ( token && ( token[0] == '<<' || token[0] == '>>' ) ) {
        p = do_binop( p, token );
        p[3] = add_expr();
    }

    return p;
}

/* rel-op   ::= '<' | '>' | '<=' | '>='
   rel-expr ::= shift-expr ( rel-op shift-expr )* */
static
rel_expr() {
    auto p = shift_expr();

    while ( token && ( token[0] == '<' || token[0] == '<=' || 
                       token[0] == '>' || token[0] == '>=' ) ) {
        p = do_binop( p, token );
        p[3] = shift_expr();
    }

    return p;
}

/* eq-op   ::= '==' | '!='
   eq-expr ::= rel-expr ( eq-op rel-expr )* */
static
eq_expr() {
    auto p = rel_expr();

    while ( token && ( token[0] == '==' || token[0] == '!=' ) ) {
        p = do_binop( p, token );
        p[3] = rel_expr();
    }

    return p;
}

/* bitand-expr ::= eq-expr ( '&' eq-expr )* */
static
bitand_expr() {
    auto p = eq_expr();

    while ( token && token[0] == '&' ) {
        p = do_binop( p, token );
        p[3] = eq_expr();
    }

    return p;
}

/* bitxor-expr ::= bitand-expr ( '^' binand-expr )* */
static
bitxor_expr() {
    auto p = bitand_expr();

    while ( token && token[0] == '^' ) {
        p = do_binop( p, token );
        p[3] = bitand_expr();
    }

    return p;
}

/* bitor-expr ::= bitxor-expr ( '|' binand-expr )* */
static
bitor_expr() {
    auto p = bitxor_expr();

    while ( token && token[0] == '|' ) {
        p = do_binop( p, token );
        p[3] = bitxor_expr();
    }

    return p;
}

/* logand-expr ::= bitor-expr ( '&&' binand-expr )* */
static
logand_expr() {
    auto p = bitor_expr();

    while ( token && token[0] == '&&' ) {
        p = do_binop( p, token );
        p[3] = bitor_expr();
    }

    return p;
}

/* logor-expr ::= logand-expr ( '||' binand-expr )* */
static
logor_expr() {
    auto p = logand_expr();

    while ( token && token[0] == '||' ) {
        p = do_binop( p, token );
        p[3] = logand_expr();
    }

    return p;
}

/* cond-expr ::= logor-expr ( '?' expr ':' assign-expr )? */
static
cond_expr() {
    auto p = logor_expr();

    if ( token && token[0] == '?' ) {
        p = do_binop( p, token );
        p[0] = '?:';  p[1] = 3;
        p[3] = expr();
        skip_node(':');
        req_token( token );
        p[4] = assign_expr();
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

    auto p = cond_expr();

    if ( token ) {
        auto t = token[0];
        if ( is_assop(t) ) {
            req_lvalue(p);
            p = do_binop( p, token );
            p[3] = assign_expr();
        }
    }

    return p;
}

/* expr ::= ( assign-expr ',' )* assign-expr */
expr() {
    auto p = assign_expr();

    /* Left-to-right associatibity: a, b, c == (a, b), c */
    while ( token && token[0] == ',' ) {
        p = do_binop( p, token );
        p[3] = assign_expr();
    }

    return p;
}
