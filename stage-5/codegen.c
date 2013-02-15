/* codegen.c  --  code generation pass
 *
 * Copyright (C) 2013 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */

/* The clabel_cnt is for .LC labels, used for string literals. */
static clabel_cnt = 0;

static
new_clabel() {
    return ++clabel_cnt;
}

/* The label_cnt is for .L labels, used for local jumps. */
static label_cnt = 0;

static
new_label() {
    return ++label_cnt;
}

static
is_binop(node) {
    return node[1] && node[2] && !node[3];
}

/* Code for leaf nodes in the expression parse tree: constants, strings,
 * identifers, and so on.  */
static
leaf_code(stream, node, need_lval) {
    if ( node[0] == 'num' || node[0] == 'chr' || node[0] == 'str' ) {
        if (need_lval) error("Literal used as an lvalue");

        if (node[0] == 'num') load_num(stream, node[1]);
        else if (node[0] == 'chr') load_chr(stream, node[1]);
        else if (node[0] == 'str') load_str(stream, &node[1], new_clabel);
    }

    else if ( node[0] == 'id' ) {
        auto off, is_lval = lookup_sym( &node[1], &off );
        if (need_lval && !is_lval) 
            error("Non-lvalue identifier where lvalue is required");
        if (!off) load_symbol(stream, &node[1], need_lval);
        else load_local(stream, off, need_lval);
    }
}

static
expr_code(stream, node, need_lval) {
    /* The following operators require an lvalue: ++, --, unary-&, =, @= */
    auto op = node[0], op_needs_lv = 0;
    if ( is_assop(op) || op == '[]' || op == '++' || op == '--' || 
              op == '&' && !node[2] )
        op_needs_lv = 1;

    /* The following operators result in an lvalue: *, [] */
    if (need_lval && op != '*' && op != '[]')
        error("Non-lvalue expression used as lvalue");

    if ( node[1] )
        codegen( stream, node[1], op_needs_lv );

    /* Unary prefix operators */
    if ( !node[2] ) {
        if      ( op == '+'  ) ;
        else if ( op == '-'  ) arith_neg(stream);
        else if ( op == '~'  ) bit_not(stream);
        else if ( op == '!'  ) logic_not(stream);
        else if ( op == '&'  ) ;
        else if ( op == '*'  ) dereference(stream, need_lval);
        else if ( op == '++' ) increment(stream);
        else if ( op == '--' ) decrement(stream);
    }

    /* Unary postfix operators */
    else if ( op == '++' || op == '--' ) {
        codegen( stream, node[2], 1 );

        if      ( op == '++' ) postfix_inc(stream);
        else if ( op == '--' ) postfix_dec(stream);
    }

    /* These operators are handled separately because of short-circuiting */
    else if ( op == '&&' || op == '||' ) {
        auto l = new_label();
        if ( op == '&&' ) branch_ifz( stream, l );
        else branch_ifnz( stream, l );
        codegen( stream, node[2], 0 );
        emit_label( stream, l );
        cast_bool( stream );
    }
    
    /* The ternary operator also short-circuits */
    else if ( op == '?:' ) {
        auto l1 = new_label(), l2 = new_label();

        branch_ifz( stream, l1 );
        codegen( stream, node[2], 0 );
        branch( stream, l2 );
        
        emit_label( stream, l1 );
        codegen( stream, node[3], 0 );
        emit_label( stream, l2 );
    }

    /* Binary operators */
    else {
        asm_push( stream );
        codegen( stream, node[2], 0);

        if      ( op == '[]'  ) pop_subscr(stream, need_lval);
        else if ( op == '*'   ) pop_mult(stream, 0);
        else if ( op == '/'   ) pop_div(stream, 0);
        else if ( op == '%'   ) pop_mod(stream, 0);
        else if ( op == '+'   ) pop_add(stream, 0);
        else if ( op == '-'   ) pop_sub(stream, 0);
        else if ( op == '<<'  ) pop_lshift(stream, 0);
        else if ( op == '>>'  ) pop_rshift(stream, 0);
        else if ( op == '<'   ) pop_lt(stream);
        else if ( op == '>'   ) pop_gt(stream);
        else if ( op == '<='  ) pop_le(stream);
        else if ( op == '>='  ) pop_ge(stream);
        else if ( op == '=='  ) pop_eq(stream);
        else if ( op == '!='  ) pop_ne(stream);
        else if ( op == '&'   ) pop_bitand(stream, 0);
        else if ( op == '|'   ) pop_bitor(stream, 0);
        else if ( op == '^'   ) pop_bitxor(stream, 0);
        else if ( op == '='   ) pop_assign(stream);
        else if ( op == '*='  ) pop_mult(stream, 1);
        else if ( op == '/='  ) pop_div(stream, 1);
        else if ( op == '%='  ) pop_mod(stream, 1);
        else if ( op == '+='  ) pop_add(stream, 1);
        else if ( op == '-='  ) pop_sub(stream, 1);
        else if ( op == '<<=' ) pop_lshift(stream, 1);
        else if ( op == '>>=' ) pop_rshift(stream, 1);
        else if ( op == '&='  ) pop_bitand(stream, 1);
        else if ( op == '|='  ) pop_bitor(stream, 1);
        else if ( op == '^='  ) pop_bitxor(stream, 1);

        /* The assignment operators are implemented to return lvalues, but
         * the C standard says they're not lvalues.  (The C++ standard has
         * them as lvalues.)  */
        if ( is_assop(op) )
            dereference(stream, 0);
    }
}

static
do_call(stream, node, need_lval) {
    auto i = node[2];
    while ( i ) {
        codegen( stream, node[ 3 + --i ], 0 );
        asm_push( stream );
    }
    if ( node[1][0] != 'id' )
        error( "Expression found where function name was expected" );
    asm_call( stream, &node[1][1], node[2] );
}

static
do_block(stream, node) {
    auto i = 0;
    while ( i < node[2] )
        codegen( stream, node[ 3 + i++ ], 0 );
}    
    
codegen(stream, node, need_lval) {
    auto op = node[0];
    if ( op == '()' )
        do_call( stream, node, need_lval );
    else if ( op == '{}' )
        do_block( stream, node );
    else if ( is_op(node[0]) )
        expr_code( stream, node, need_lval );
    else
        return leaf_code( stream, node, need_lval );
}
