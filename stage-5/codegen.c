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
named_label(name, is_defn) {
    if ( save_label( name, 0 ) )
        return set_label( name, new_label() );
    else
        return get_label( name );
}

/* Code for leaf nodes in the expression parse tree: constants, strings,
 * identifers, and so on.  */
static
leaf_code(stream, node, need_lval) {
    if ( node[0] == 'num' || node[0] == 'chr' || node[0] == 'str' ) {
        if (node[0] == 'num') load_num(stream, node[2]);
        else if (node[0] == 'chr') load_chr(stream, &node[2]);
        else if (node[0] == 'str') load_str(stream, &node[2], new_clabel());
    }

    else if ( node[0] == 'id' ) {
        auto off, is_lval = lookup_sym( &node[2], &off );
        auto need_addr = (is_lval == need_lval);

        if (!off) load_symbol(stream, &node[2], need_addr);
        else load_local(stream, off, need_addr);
    }

    else int_error("Unknown token '%Mc' in parse tree", node[0]);
}

static
unary_post(stream, node, need_lval) {
    expr_code( stream, node[2], 1 );

    if      ( node[0] == '++' ) postfix_inc(stream);
    else if ( node[0] == '--' ) postfix_dec(stream);
    else int_error( "Unknown operator: '%Mc'", node[0] );
}

static
unary_pre(stream, node, need_lval) {
    auto op = node[0];
    auto op_needs_lv = op == '++' || op == '--' || op == '&';
    expr_code( stream, node[2], op_needs_lv );

    if      ( op == '+'  ) ;
    else if ( op == '-'  ) arith_neg(stream);
    else if ( op == '~'  ) bit_not(stream);
    else if ( op == '!'  ) logic_not(stream);
    else if ( op == '&'  ) ;
    else if ( op == '*'  ) dereference(stream, need_lval);
    else if ( op == '++' ) increment(stream);
    else if ( op == '--' ) decrement(stream);
    else int_error("Unknown operator: '%Mc'", op);
}

static
logical_bin(stream, node) {
    auto l = new_label();

    expr_code( stream, node[2], 0 );
    if ( node[0] == '&&' ) branch_ifz( stream, l );
    else branch_ifnz( stream, l );

    expr_code( stream, node[3], 0 );
    emit_label( stream, l );
    cast_bool( stream );
}

static
conditional(stream, node) {
    auto l1 = new_label(), l2 = new_label();

    expr_code( stream, node[2], 0 );

    branch_ifz( stream, l1 );
    expr_code( stream, node[3], 0 );
    branch( stream, l2 );
    
    emit_label( stream, l1 );
    expr_code( stream, node[4], 0 );
    emit_label( stream, l2 );
}

static
subscript(stream, node, need_lval) {
    auto type = expr_type( node[2] );  /* type being subscripted */

    /* For (hopefully temporary) compatibility with stage-4, we allow an 
     * implicit int to be dereferenced as an int*. */
    extern compat_flag;
    auto elt_sz = compat_flag && type == implct_int() ? 4 
        : type_size( type[2] );        /* remove * or [] from type */

    pop_subscr(stream, elt_sz, need_lval);
}

static
binary_op(stream, node, need_lval) {
    auto op = node[0];
    expr_code( stream, node[2], is_assop(op) );

    /* Discard the l.h.s. of the , operator. */
    if ( op != ',' ) asm_push( stream );

    expr_code( stream, node[3], 0);

    if      ( op == '[]'  ) subscript(stream, node, need_lval);
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

    else if ( op != ','   ) int_error("Unknown operator: '%Mc'", op);

    /* The assignment operators are implemented to return lvalues, but
     * the C standard says they're not lvalues.  (The C++ standard has
     * them as lvalues.)  */
    if ( is_assop(op) )
        dereference(stream, 0);
}

static
opexpr_code(stream, node, need_lval) {
    auto op = node[0];

    /* Unary prefix operators */
    if ( node[1] == 1 )
        unary_pre( stream, node, need_lval );

    /* Unary postfix operators (marked binary in the tree) */
    else if ( op == '++' || op == '--' )
        unary_post( stream, node, need_lval );

    /* These operators are handled separately because of short-circuiting */
    else if ( op == '&&' || op == '||' )
        logical_bin( stream, node );
    
    /* Binary operators */
    else if ( node[1] == 2 )
        binary_op( stream, node, need_lval );

    /* The ternary operator also short-circuits */
    else if ( op == '?:' )
        conditional( stream, node );

    else
        int_error( "Unknown operator: '%Mc'", node[0] );
}

static
do_call(stream, node, need_lval) {
    auto args = node[1] - 1, i = args;
    while ( i ) {
        expr_code( stream, node[ 3 + --i ], 0 );
        asm_push( stream );
    }

    /* If we're calling an identifier that's not local and isn't an lval,
     * it must either be an extern function or an extern array.  The latter
     * would be a syntax error but for the present lack of a type system
     * So we assume it's a function and do a direct call if possible. */
    if ( node[2][0] == 'id' ) {
        auto off, is_lval = lookup_sym( &node[2][2], &off );
        if (!off && !is_lval)
            return asm_call( stream, &node[2][2], args*4 );
    }

    /* And fall back to an indirect call with CALL *%eax. */
    expr_code( stream, node[2], 0 );
    call_ptr( stream, args*4 );
}

static
expr_code(stream, node, need_lval) {
    auto op = node[0];
    if ( op == '()' )
        do_call( stream, node, need_lval );
    else if ( is_op(node[0]) )
        opexpr_code( stream, node, need_lval );
    else
        return leaf_code( stream, node, need_lval );
}

static
return_stmt(stream, node, ret) {
    if ( node[2] ) expr_code( stream, node[2], 0 );
    branch( stream, ret );
}

static
goto_stmt(stream, node) {
    branch( stream, named_label( &node[2][2], 0 ) );
}

static
if_stmt(stream, node, brk, cont, ret) {
    auto l1 = new_label(), l2 = l1;

    expr_code( stream, node[2], 0 );
    
    branch_ifz( stream, l1 );
    stmt_code( stream, node[3], brk, cont, ret );

    if ( node[4] ) {
        l2 = new_label();
        branch( stream, l2 );
        emit_label( stream, l1 );
        stmt_code( stream, node[4], brk, cont, ret );
    }

    emit_label( stream, l2 );
}

static
while_stmt(stream, node, brk, cont, ret) {
    cont = new_label();
    emit_label( stream, cont );

    expr_code( stream, node[2], 0 );
    brk = new_label();
    branch_ifz( stream, brk );

    stmt_code( stream, node[3], brk, cont, ret );
    branch( stream, cont );
    emit_label( stream, brk );
}

static
do_stmt(stream, node, brk, cont, ret) {
    auto start = new_label();
    emit_label( stream, start );

    cont = new_label();
    brk = new_label();
    
    stmt_code( stream, node[2], brk, cont, ret );

    emit_label( stream, cont );
    expr_code( stream, node[3], 0 );
    branch_ifnz( stream, start );
    emit_label( stream, brk );
}

static
for_stmt(stream, node, brk, cont, ret) {
    if (node[2])
        expr_code( stream, node[2], 0 );

    auto start = new_label();
    emit_label( stream, start );

    brk = new_label();
    if (node[3]) {
        expr_code( stream, node[3], 0 );
        branch_ifz( stream, brk );
    }

    cont = new_label();
    stmt_code( stream, node[5], brk, cont, ret );
    
    emit_label( stream, cont );
    if (node[4])
        expr_code( stream, node[4], 0 );

    branch( stream, start );
    emit_label( stream, brk );
}

static
case_stmt(stream, node, brk, cont, ret) {
    emit_label( stream, node[4] );
    stmt_code( stream, node[3], brk, cont, ret );
}

static
switch_stmt(stream, node, brk, cont, ret) {
    auto i = 0, def;
    expr_code( stream, node[2], 0 );
    def = brk = new_label();

    if (node[4][0] != 'swtb') int_error("No table for switch statement");

    while ( i < node[4][1] ) {
        auto c = node[4][2 + i++];

        /* Make asm labels for each case label.  We store these as integers 
         * in the unused 3rd operator slot of the case node. */
        c[4] = new_label();

        if (c[0] == 'case')
            /* Case labels have to be integers */
            branch_eq_n( stream, c[2][2], c[4] );
        else 
            def = c[4];
    }
    
    branch( stream, def ); 
    stmt_code( stream, node[3], brk, cont, ret );
    emit_label( stream, brk );
}

static
extern_decl(stream, decl) {
    save_sym( decl, 0 );
}

static
auto_decl(stream, decl) {
    auto sz = decl_var(decl);
    auto offset = frame_off();

    /* Is there an initaliser */
    if ( decl[4] ) {
        auto init = decl[4];
        if ( init[0] == '{}' ) {
            auto j = 0;
            while ( j < init[1] ) {
                expr_code( stream, init[2 + j], 0 );
                save_local( stream, offset + j*4 );
                ++j;
            }

            /* Check for an initialisation list that's not long enough.
             * If it's not, the standard requires zero initialisation of
             * the extra members. C99:6.7.8/21 */
            if ( j < sz/4 ) {
                load_zero( stream );
                while ( j < sz/4 ) {
                    save_local( stream, offset + j*4 );
                    ++j;
                }
            }
        }
        /* Scalar initialisation */
        else {
            expr_code( stream, init, 0 );
            save_local( stream, offset );
        }
    }
}

static
declaration(stream, node) {
    auto i = 2;
    while ( i < node[1] ) {
        auto decl = node[ 2 + i++ ];
        if ( node[2] && node[2][0] == 'exte' )
            extern_decl( stream, decl );
        else
            auto_decl( stream, decl );
    }
}

static
label_stmt(stream, node, brk, cont, ret) {
    emit_label( stream, named_label( &node[2][2], 1 ) );
    stmt_code( stream, node[3], brk, cont, ret );
}

static
stmt_code(stream, node, brk, cont, ret) {
    /* Handle the null expression. */
    if ( !node ) return;

    auto op = node[0];

    if      ( op == 'dcls' ) declaration( stream, node );

    else if ( op == 'brea' ) branch( stream, brk );
    else if ( op == 'cont' ) branch( stream, cont ); 
    else if ( op == 'retu' ) return_stmt( stream, node, ret );
    else if ( op == 'goto' ) goto_stmt( stream, node );

    else if ( op == 'if'   ) if_stmt( stream, node, brk, cont, ret );
    else if ( op == 'whil' ) while_stmt( stream, node, brk, cont, ret );
    else if ( op == 'for'  ) for_stmt( stream, node, brk, cont, ret );
    else if ( op == 'do'   ) do_stmt( stream, node, brk, cont, ret );
    else if ( op == 'swit' ) switch_stmt( stream, node, brk, cont, ret );
    else if ( op == 'case' 
          ||  op == 'defa' ) case_stmt( stream, node, brk, cont, ret );
    
    else if ( op == ':'    ) label_stmt( stream, node, brk, cont, ret );
    else if ( op == '{}'   ) do_block( stream, node, brk, cont, ret );
    else                     expr_code( stream, node, 0 );
}

static
do_block(stream, node, brk, cont, ret) {
    auto i = 0;
    start_block();
    while ( i < node[1] )
        stmt_code( stream, node[ 2 + i++ ], brk, cont, ret );
    end_block();
}

static
set_storage(stream, storage, name) {
    if (storage == 'stat')
        local_decl(stream, name);
    else
        globl_decl(stream, name);
}

static
fn_decl(stream, name, decl, block, frame_sz) {
    auto ret = new_label();
    start_fn(decl);
    prolog(stream, name, frame_sz);
    do_block(stream, block, -1, -1, ret);
    emit_label( stream, ret );
    epilog(stream, frame_sz);
    end_fn();
}

static
int_decl(stream, name, init ) {
    data_decl(stream, name);
    if (!init)
        int_decl_n(stream, 0);
    else if (init[0] == 'num')
        int_decl_n(stream, init[2]);
    else
        int_decl_s(stream, &init[2]);
}

/* Global-scope array declaration */
static
array_decl(stream, decl) {
    auto init = decl[4], sz = type_size( decl[2] );
    data_decl(stream, &decl[3][2]);

    if (init) {
        auto i = 0;
        while ( i < init[1] ) {
            auto ival = init[ 2 + i++ ];
            if (ival[0] == 'num')
                int_decl_n(stream, ival[2]);
            else
                int_decl_s(stream, &ival[2]);
        }

        if ( 4 * init[1] < sz )
            zero_direct( stream, sz - 4*init[1] );
    }

    save_sym( decl, 0 );
}

/* This is only callled on top-level declarations */
codegen(stream, node) {
    auto decls = node, i = 2;
    auto storage = decls[2] ? decls[2][0] : 0;

    while ( i < node[1] ) {
        auto decl = node[ 2 + i++ ];
        auto init = decl[4];

        /* We only need to take action here if this is a definition.
         * Declarations that are not definitions have already been 
         * added to the symbol table by the parser. */
        /* TODO:  tentative definitions, duplicate definitions */
        if (init || storage == 'stat' ) {
            auto type = decl[2], name = &decl[3][2];
            set_storage( stream, storage, name );

            if (type[0] == 'dclt')
                int_decl( stream, name, init );
            else if (type[0] == '()')
                fn_decl( stream, name, type, init, decl[5] );
            else if (type[0] == '[]')
                array_decl( stream, decl );
            else 
                int_error("Unexpected node in declaration");
        }
    }
}
