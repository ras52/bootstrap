/* expr.c  --  code to parse expressions
 *
 * Copyright (C) 2013, 2014 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */

/*  constant    ::= int-const | char-const      TODO:  float, enum  */
/*  primry-expr ::= identifier | constant | string-lit | '(' expression ')' */
static
primry_expr(req_const) {
    extern struct node *expr();
    auto t = peek_token();

    if ( t == '(' ) {
        auto struct node *n;
        skip_node('(');
        n = expr(req_const); 
        skip_node(')');
        return n;
    }
    else if ( t == 'num' || t == 'str' )
        return take_node(0);

    else if ( t == 'id' ) {
        auto struct node *n = take_node(0);
        if ( is_typedef( node_str(n) ) )
            error("Invalid use of typedef name: %s", node_str(n));

        /* If the identifier is undeclared, the type will be null. 
         * This happens when calling undeclared functions. */
        set_type( n, lookup_type( node_str(n) ) ); 
        if ( node_type(n) ) add_ref( node_type(n) );
        return n;
    }

    else if ( t == 'chr' ) {
        auto struct node *n = take_node(0);
        /* Convert it into a integer literal: in C, char lits have type int. */
        set_code( n, 'num' );
        set_op( n, 0, parse_chr( node_str(n) ) );
        return n;
    }

    else {
        /* Give a nicer error message if we hit _Pragma("RBC end_expr") */
        if ( t == 'prgm' ) {
            auto struct node *n = get_node();
            if ( node_arity(n) == 2 && node_streq( node_op(n, 0), "RBC" ) 
                 && node_streq( node_op(n, 1), "end_expr" ) )
                error("Unexpected end of expression");
        }

        error("Unexpected token '%Mc' while parsing expression", t);
    }
}

/*  arg-list ::= ( expr ( ',' expr )* )? */
static
arg_list(req_const, fn) {
    extern struct node *vnode_app();
    auto struct node *p = new_node('()', 0);
    p = vnode_app( p, fn );

    if ( peek_token() == ')' ) 
        return p;

    while (1) {
        req_token();
        p = vnode_app( p, assign_expr( req_const ) );

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
    auto struct node *node = take_node(2);
    set_op( node, 0, lhs );
    req_token();
    return node;
}

/*  member-seq  ::= ( '->' | '.' ) identifier
 *  postfx-seq  ::= '(' arg-list ')' | '[' expr ']' | '++' | '--' | member-seq
 *  postfx-expr ::= primry-expr postfix-seq* */
static
postfx_expr(req_const) {
    extern struct node *do_binop();
    extern struct node *arg_list();
    auto struct node *p = primry_expr(req_const);
    auto int t;

    while (1) {
        req_token();
        t = peek_token();

        /* The only valid use of an undeclared symbol is in a simple function 
         * call.   In that case, foo() is valid, but, say, (foo)() is not. */
        if ( t != '(' && node_code(p) == 'id' && !is_declared( node_str(p) ) ) 
            error( "Undefined symbol '%s'", node_str(p) );


        if ( t == '++' || t == '--' ) {
            if ( req_const )
                error("Increment not permitted in constant expression");

            /* Postfix ++ and -- are marked as binary operators.  This is 
             * what distinguishes them from the prefix versions which are 
             * marked as unary.  This is inspired by C++'s idea of
             * T operator++(T&, int) vs. T& operator++(T&). */
            p = do_binop( p );

            chk_incdec(p);
        }
    
        else if ( t == '[' ) {
            p = do_binop( p );
            set_code( p, '[]' );
            set_op( p, 1, expr( req_const ) );
            skip_node(']');

            chk_subscr(p);
        }
    
        else if ( t == '(' ) {
            if ( req_const )
                error("Function call not permitted in constant expression");

            skip_node('(');
            p = arg_list( req_const, p );
            skip_node(')');

            chk_call(p);
        }

        else if ( t == '.' || t == '->' ) {
            p = do_binop( p );
            if ( peek_token() != 'id' )
                error("Expected identifier as member name");
            set_op( p, 1, take_node(0) );

            chk_member(p);
        }

        else break;
    }

    return p;
}

/*  unary-op   ::= '+' | '-' | '~' | '!' | '*' | '&' | '++' | '--'
 *  unary-expr ::= unary-op unary-expr | postfx-expr 
 *                   | 'sizeof' '(' type-name ')'                           */
static
unary_expr( req_const ) {
    auto t = peek_token();

    if ( strnlen(&t, 4) == 1 && strchr("+-~!*&", t) 
         || t == '++' || t == '--' ) {
        auto struct node *p = take_node(1);
        req_token();

        /* The grammar says says the argument is sometimes a unary-expr,
         * and sometimes a cast-expr.  In practice we can universally parse 
         * it as a cast-expr.  If someone writes ++(type)(expr) then the
         * lvalue check will catch the error. */
        set_op( p, 0, cast_expr( req_const ) );

        if (t == '+' || t == '-' || t == '~')
            set_type( p, add_ref( prom_type( node_type( node_op( p, 0 ) ) ) ) );

        else if (t == '!')
            set_type( p, add_ref( implct_int() ) );

        else if (t == '&')
            chk_addr(p);

        else if (t == '++' || t == '--') {
            if ( req_const )
                error("Increment not permitted in constant expression");
            chk_incdec(p);
        }

        else if (t == '*')
            chk_deref(p);
        return p;
    }

    else if ( t == 'size' ) {
        extern struct node *type_name();
        /* We'll overwrite the node with a literal containing the type size.
         * Note that the 'num' node abuses op[0] to contain the integer 
         * value; hence arity = 0. */
        auto struct node *p = take_node(0), *decl; 
        skip_node('(');
        decl = type_name();
        skip_node(')');

        set_code( p, 'num' );
        set_type( p, add_ref( size_t_type() ) );
        set_op( p, 0, type_size( node_type( decl ) ) );

        free_node(decl);
        return p;
    }

    else return postfx_expr( req_const );
}

/* cast-expr ::= '(' type-name ')' cast-expr | unary-expr */
static
cast_expr(req_const) {
    if ( peek_token() == '(' ) {
        auto struct node *brack = take_node(0);

        /* We don't yet know that we have a cast expression.  It might be 
         * the opening '(' of a primary-expr.    */
        if ( !is_dclspec() ) {
            /* We can't just call skip_token('('); expr(); skip_token(')');
             * beacuse that doesn't cope with (*x)++ where we need the 
             * call stack to handle the postfix operator on the way back 
             * down the call stack. */
            unget_token(brack);            
            return unary_expr( req_const );
        }
        else {
            extern struct node *type_name();
            auto struct node* type = type_name();
            skip_node(')');

            /* Use a unary-plus, as that's the identity op. */
            set_code( brack, '+' );
            set_arity( brack, 1 );
            set_type( brack, add_ref( node_type( type ) ) );
            set_op( brack, 0, cast_expr( req_const ) );

            free_node(type);
            return brack;
        }
    }

    else return unary_expr( req_const );
}

static
test_level( t, ap ) {
    /* In our implementation, va_copy is simple copy and was done by the 
     * function call.  We can therefore safely use AP directly. 
     * __va_arg() really returns a void*, but the va_arg macro would cast
     * it to the multicharacter (i.e. int) pointer. */
    extern int *__va_arg();
    auto arg;
    while ( arg = *__va_arg( &ap, 4 ) ) 
        if ( arg == t )
            return 1;
    return 0;
}

/* node* bin_level( node* (*chain)(int req_const), int req_const, 
 *                  void (*check)(node*), ... );
 *
 * Handle a standard left-to-right binary operator precedence level.  
 * Call CHAIN() to parse the operands, so CHAIN should be a pointer 
 * to the parser function for a higher precedence level.  Then call
 * CHECK(node) on the resultant node, which should determine the type 
 * of expression.
 *
 * It is a variadic function, and the varargs are operator tokens in 
 * the predence level, followed by a 0 end marker. */
static
bin_level( chain, req_const, check ) 
    int (*chain)(), (*check)();
{
    extern struct node *do_binop();
    auto struct node *p = chain( req_const );
    auto ap; __va_start( &ap, &check, 4 );

    /* Left-to-right associatibity: a, b, c == (a, b), c */
    while ( test_level( peek_token(), ap ) ) { 
        p = do_binop( p );
        set_op( p, 1, chain( req_const ) );
        check(p);
    }

    return p;
}

/* mult-op   ::= '*' | '/' | '%'
 * mult-expr ::= cast-expr ( mult-op cast-expr )* */
static
mult_expr(req_const) {
    extern chk_mult();
    return bin_level( cast_expr, req_const, chk_mult, '*', '/', '%', 0 );
}

/* add-op   ::= '+' | '-'
 * add-expr ::= mult-expr ( add-op mult-expr )* */
static
add_expr(req_const) {
    extern chk_add();
    return bin_level( mult_expr, req_const, chk_add, '+', '-', 0 );
}

/* shift-op   ::= '<<' | '>>'
 * shift-expr ::= add-expr ( shift-op add-expr )* */
static
shift_expr(req_const) {
    extern chk_shift();
    return bin_level( add_expr, req_const, chk_shift, '<<', '>>', 0 );
}

/* rel-op   ::= '<' | '>' | '<=' | '>='
 * rel-expr ::= shift-expr ( rel-op shift-expr )* */
static
rel_expr(req_const) {
    extern chk_cmp();
    return bin_level( shift_expr, req_const, chk_cmp, '<', '<=', '>', '>=', 0 );
}

/* eq-op   ::= '==' | '!='
 * eq-expr ::= rel-expr ( eq-op rel-expr )* */
static
eq_expr(req_const) {
    extern chk_cmp();
    return bin_level( rel_expr, req_const, chk_cmp, '==', '!=', 0 );
}

/* bitand-expr ::= eq-expr ( '&' eq-expr )* */
static
bitand_expr(req_const) {
    extern chk_bitop();
    return bin_level( eq_expr, req_const, chk_bitop, '&', 0 );
}

/* bitxor-expr ::= bitand-expr ( '^' binand-expr )* */
static
bitxor_expr(req_const) {
    extern chk_bitop();
    return bin_level( bitand_expr, req_const, chk_bitop, '^', 0 );
}

/* bitor-expr ::= bitxor-expr ( '|' binand-expr )* */
static
bitor_expr(req_const) {
    extern chk_bitop();
    return bin_level( bitxor_expr, req_const, chk_bitop, '|', 0 );
}

/* logand-expr ::= bitor-expr ( '&&' binand-expr )* */
static
logand_expr(req_const) {
    extern chk_int();
    return bin_level( bitor_expr, req_const, chk_int, '&&', 0 );
}

/* logor-expr ::= logand-expr ( '||' binand-expr )* */
static
logor_expr(req_const) {
    extern chk_int();
    return bin_level( logand_expr, req_const, chk_int, '||', 0 );
}

/* cond-expr ::= logor-expr ( '?' expr ':' assign-expr )? */
static
cond_expr(req_const) {
    extern struct node *do_binop();
    auto struct node *p = logor_expr( req_const );

    if ( peek_token() == '?' ) {
        p = do_binop( p );
        set_code( p, '?:' );
        set_arity( p, 3 );
        set_op( p, 1, expr( req_const ) );
        skip_node(':');
        req_token();
        set_op( p, 2, assign_expr( req_const ) );
        set_type( p, add_ref( implct_int() ) );
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
assign_expr(req_const) {
    extern struct node *cond_expr();
    extern struct node *do_binop();

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

    auto struct node *p = cond_expr( req_const );
    if ( is_assop( peek_token() ) ) {
        if ( req_const )
            error("Assignment not permitted in constant expression");
        p = do_binop( p );
        set_op( p, 1, assign_expr( req_const ) );
        chk_assign(p);
    }

    return p;
}

/* expr ::= ( assign-expr ',' )* assign-expr */
expr(req_const) {
    extern struct node *assign_expr();
    extern struct node *do_binop();

    auto struct node *p = assign_expr( req_const );

    /* Left-to-right associatibity: a, b, c == (a, b), c */
    while ( peek_token() == ',' ) { 
        if ( req_const )
            error("Comma operator not permitted in constant expression");
        p = do_binop( p );
        set_op( p, 1, assign_expr( req_const ) );
        chk_comma(p);
    }

    return p;
}
