/* codegen.c  --  code generation pass
 *
 * Copyright (C) 2013, 2014 Richard Smith <richard@ex-parrot.com>
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
    if (node[0] == 'num')
        load_num(stream, node[3], is_unsigned(node[2]));

    else if (node[0] == 'chr')
        load_chr(stream, &node[3]);

    else if (node[0] == 'str') {
        auto lc = new_clabel();
        defn_str(stream, &node[3], lc);
        load_str(stream, lc);
    }

    else if ( node[0] == 'id' ) {
        auto off, is_lval = lookup_sym( &node[3], &off );
        auto need_addr = (is_lval == need_lval);

        if (!off) load_symbol(stream, &node[3], need_addr);
        else load_local(stream, off, need_addr);
    }

    else int_error("Unknown token '%Mc' in parse tree", node[0]);
}

static
ptr_inc_sz(type) {
    if ( type[0] == '[]' || type[0] == '*' )
        return type_size( type[3] );
    else
        return 1;
}

static
unary_post(stream, node, need_lval) {
    auto sz = type_size(node[2]), elt_sz = ptr_inc_sz(node[2]);

    expr_code( stream, node[3], 1 );

    if      ( node[0] == '++' ) postfix_inc( stream, sz,  elt_sz );
    else if ( node[0] == '--' ) postfix_inc( stream, sz, -elt_sz );
    else int_error( "Unknown operator: '%Mc'", node[0] );
}

static
member_bin(stream, node, need_lval) {
extern stderr;
    expr_code( stream, node[3], node[0] == '.' );

    /* This is a bit of a fudge.  In 'struct foo { int x[2] };', x is an
     * address even though it's not an lvalue, per se. */
    if ( node[2][0] == '[]' ) need_lval = 1;

    /* We've stored the member offset as an integer in node[5] */
    mem_access( stream, node[5], need_lval );
}

static
unary_pre(stream, node, need_lval) {
    auto op = node[0];
    auto op_needs_lv = op == '++' || op == '--' || op == '&';
    auto sz = type_size(node[2]);
    expr_code( stream, node[3], op_needs_lv );

    if ( op == '+'  ) {
        /* Arrays degrade to pointers. */
        auto src_sz;
        if ( node[3][2][0] == '[]' ) src_sz = 4;
        else src_sz = type_size(node[3][2]);
        promote(stream, node[2][5], src_sz, sz);
    }
    else if ( op == '-'  ) arith_neg(stream);
    else if ( op == '~'  ) bit_not(stream);
    else if ( op == '!'  ) logic_not(stream, type_size(node[3][2]));
    else if ( op == '&'  ) ;
    else if ( op == '*'  ) dereference(stream, sz, need_lval);
    else if ( op == '++' ) increment(stream, sz,  ptr_inc_sz(node[2]) );
    else if ( op == '--' ) increment(stream, sz, -ptr_inc_sz(node[2]) );
    else int_error("Unknown operator: '%Mc'", op);
}

static
logical_bin(stream, node) {
    auto l = new_label();
    auto sz = type_size( node[3][2] );

    expr_code( stream, node[3], 0 );
    if ( node[0] == '&&' ) branch_ifz( stream, sz, l );
    else branch_ifnz( stream, sz, l );

    expr_code( stream, node[4], 0 );
    emit_label( stream, l );
    cast_bool( stream );
}

/* This handles the ternary operator */
static
conditional(stream, node) {
    auto l1 = new_label(), l2 = new_label();

    expr_code( stream, node[3], 0 );

    branch_ifz( stream, type_size(node[3][2]), l1 );
    expr_code( stream, node[4], 0 );
    branch( stream, l2 );
    
    emit_label( stream, l1 );
    expr_code( stream, node[5], 0 );
    emit_label( stream, l2 );
}

static
subscript(stream, node, need_lval) {
    auto type = node[3][2];  /* type being subscripted */

    /* For (hopefully temporary) compatibility with stage-4, we allow an 
     * implicit int to be dereferenced as an int*. */
    extern compat_flag;
    auto elt_sz = compat_flag && type == implct_int() ? 4 
        : type_size( type[3] );        /* remove * or [] from type */

    auto sz = type_size( node[2] );

    scale_elt(stream, elt_sz, 1);
    pop_add(stream, 0, 4);
    dereference(stream, sz, need_lval);
}

static
add_op(stream, node, is_ass, argsz) {
    auto op = node[0];

    /* Pointer plus integer needs scaling.  We canonicalised this during
     * parsing so the pointer is always the first arg which is in node[3]. */
    if ( is_pointer( node[3][2] ) && is_integral( node[4][2] ) )
        scale_elt( stream, type_size( node[3][2][3] ), 1 );
   
    if ( op == '-' || op == '-=' )
        pop_sub(stream, is_ass, argsz);
    else 
        pop_add(stream, is_ass, argsz);

    /* Pointer minus pointer needs scaling down. */
    if ( is_pointer( node[3][2] ) && is_pointer( node[4][2] ) )
        scale_elt( stream, type_size( node[3][2][3] ), -1 );
}

static
cmp_op(stream, node) {
    auto op = node[0];

    auto type; /* By this point the types must be compatible */
    if ( node[3][2][0] == '*' ) type = node[3][2];
    else type = usual_conv( node[3][2], node[4][2] );

    auto is_unsgn = is_unsigned(type);
    auto sz = type_size(type);

    expr_code( stream, node[3], 0 );
    promote( stream, is_unsgn, type_size( node[3][2] ), sz );
    asm_push( stream );
    expr_code( stream, node[4], 0 );
    promote( stream, is_unsgn, type_size( node[4][2] ), sz );

    if      ( op == '<'   ) pop_lt(stream, sz, is_unsgn);
    else if ( op == '>'   ) pop_gt(stream, sz, is_unsgn);
    else if ( op == '<='  ) pop_le(stream, sz, is_unsgn);
    else if ( op == '>='  ) pop_ge(stream, sz, is_unsgn);
    else if ( op == '=='  ) pop_eq(stream, sz);
    else if ( op == '!='  ) pop_ne(stream, sz);
}

static
is_unsigned(type) {
    return type[0] == 'dclt' && type[5] && type[5][0] == 'unsi';
}

static
binary_op(stream, node, need_lval) {
    auto op = node[0], sz = type_size( node[2] );

    expr_code( stream, node[3], is_assop(op) );
    asm_push( stream );
    expr_code( stream, node[4], 0 );

    if      ( op == '[]'  ) subscript(stream, node, need_lval);
    else if ( op == '*'   ) pop_mult(stream, 0, is_unsigned(node[2]));
    else if ( op == '/'   ) pop_div(stream, 0, is_unsigned(node[2]));
    else if ( op == '%'   ) pop_mod(stream, 0, is_unsigned(node[2]));
    else if ( op == '+'   ) add_op(stream, node, 0, sz);
    else if ( op == '-'   ) add_op(stream, node, 0, sz);
    else if ( op == '<<'  ) pop_lshift(stream, 0);
    else if ( op == '>>'  ) pop_rshift(stream, 0);
    else if ( op == '&'   ) pop_bitand(stream, 0, sz);
    else if ( op == '|'   ) pop_bitor(stream, 0, sz);
    else if ( op == '^'   ) pop_bitxor(stream, 0, sz);

    else if ( op == '='   ) pop_assign(stream, sz);
    else if ( op == '*='  ) pop_mult(stream, 1, is_unsigned(node[2]));
    else if ( op == '/='  ) pop_div(stream, 1, is_unsigned(node[2]));
    else if ( op == '%='  ) pop_mod(stream, 1, is_unsigned(node[2]));
    else if ( op == '+='  ) add_op(stream, node, 1, sz);
    else if ( op == '-='  ) add_op(stream, node, 1, sz);
    else if ( op == '<<=' ) pop_lshift(stream, 1);
    else if ( op == '>>=' ) pop_rshift(stream, 1);
    else if ( op == '&='  ) pop_bitand(stream, 1, sz);
    else if ( op == '|='  ) pop_bitor(stream, 1, sz);
    else if ( op == '^='  ) pop_bitxor(stream, 1, sz);

    else int_error("Unknown operator: '%Mc'", op);

    /* The assignment operators are implemented to return lvalues, but
     * the C standard says they're not lvalues.  (The C++ standard has
     * them as lvalues.)  */
    if ( is_assop(op) )
        dereference(stream, type_size(node[2]), 0);
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

    /* And these are because they don't evaluate their second argument */
    else if ( op == '->' || op == '.' )
        member_bin( stream, node, need_lval );
   
    else if ( op == '<' || op == '>' || op == '<=' || op == '>='
              || op == '==' || op == '!=' )
        cmp_op( stream, node );

    else if ( op == ',' ) {
        expr_code( stream, node[3], 0 );
        expr_code( stream, node[4], 0 );
    }

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
        expr_code( stream, node[3+i], 0 );
        /* If no prototype is present (as is the case here as we don't support
         * them), we must do integer promotions on the arguments. */
        auto sz = type_size( node[3+i][2] );
        if (sz < 4) promote(stream, node[3+i][2][5], sz, 4);
        asm_push( stream );
        --i;
    }

    /* If we're calling an identifier that's not local and isn't an lval,
     * it must either be an extern function or an extern array.  The latter
     * would be a syntax error but for the present lack of a type system
     * So we assume it's a function and do a direct call if possible. */
    if ( node[3][0] == 'id' ) {
        auto off, is_lval = lookup_sym( &node[3][3], &off );
        if (!off && !is_lval)
            return asm_call( stream, &node[3][3], args*4 );
    }

    /* And fall back to an indirect call with CALL *%eax. */
    expr_code( stream, node[3], 0 );
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
    if ( node[3] ) expr_code( stream, node[3], 0 );
    branch( stream, ret );
}

static
goto_stmt(stream, node) {
    branch( stream, named_label( &node[3][3], 0 ) );
}

static
if_stmt(stream, node, brk, cont, ret) {
    auto l1 = new_label(), l2 = l1;

    expr_code( stream, node[3], 0 );

    /* In principle we should promote this to an integer, but it's more 
     * efficient to do the test on the actual type: e.g. instead of generating
     * MOVZVL %al, %eax; TESTL %eax, %eax; we just generatd TESTB %al, %a; */
    
    branch_ifz( stream, type_size(node[3][2]), l1 );
    stmt_code( stream, node[4], brk, cont, ret );

    if ( node[5] ) {
        l2 = new_label();
        branch( stream, l2 );
        emit_label( stream, l1 );
        stmt_code( stream, node[5], brk, cont, ret );
    }

    emit_label( stream, l2 );
}

static
while_stmt(stream, node, brk, cont, ret) {
    cont = new_label();
    emit_label( stream, cont );

    expr_code( stream, node[3], 0 );
    brk = new_label();
    branch_ifz( stream, type_size(node[3][2]), brk );

    stmt_code( stream, node[4], brk, cont, ret );
    branch( stream, cont );
    emit_label( stream, brk );
}

static
do_stmt(stream, node, brk, cont, ret) {
    auto start = new_label();
    emit_label( stream, start );

    cont = new_label();
    brk = new_label();
    
    stmt_code( stream, node[3], brk, cont, ret );

    emit_label( stream, cont );
    expr_code( stream, node[4], 0 );
    branch_ifnz( stream, type_size(node[4][2]), start );
    emit_label( stream, brk );
}

static
for_stmt(stream, node, brk, cont, ret) {
    if (node[3])
        expr_code( stream, node[3], 0 );

    auto start = new_label();
    emit_label( stream, start );

    brk = new_label();
    if (node[4]) {
        expr_code( stream, node[4], 0 );
        branch_ifz( stream, type_size(node[4][2]), brk );
    }

    cont = new_label();
    stmt_code( stream, node[6], brk, cont, ret );
    
    emit_label( stream, cont );
    if (node[5])
        expr_code( stream, node[5], 0 );

    branch( stream, start );
    emit_label( stream, brk );
}

static
case_stmt(stream, node, brk, cont, ret) {
    /* The case labels were written into the unused 3rd operator slot of 
     * the case node by switch_stmt(), below. */
    emit_label( stream, node[5] );
    stmt_code( stream, node[4], brk, cont, ret );
}

static
switch_stmt(stream, node, brk, cont, ret) {
    auto i = 0, def;
    expr_code( stream, node[3], 0 );
    def = brk = new_label();

    if (node[5][0] != 'swtb')
        int_error("No table for switch statement");

    while ( i < node[5][1] ) {
        auto c = node[5][3 + i++];

        /* Make asm labels for each case label.  We store these as integers 
         * in the unused 3rd operator slot of the case node. */
        c[5] = new_label();

        if (c[0] == 'case')
            /* Case labels have to be integers */
            branch_eq_n( stream, c[3][3], c[5] );
        else 
            def = c[5];
    }
    
    branch( stream, def ); 
    stmt_code( stream, node[4], brk, cont, ret );
    emit_label( stream, brk );
}

static
auto_decl(stream, decl) {
    auto offset, init;

    decl_var(decl);
    offset = frame_off();
    init = decl[5];

    /* Is there an initaliser */
    if ( init ) {
        if ( init[0] == '{}' ) {
            auto j = 0, elt_sz, n_elts;
            if ( decl[2][0] == '[]' ) {
                elt_sz = type_size( decl[2][3] );
                n_elts = decl[2][4][3];
            } 
            else {
                elt_sz = type_size( decl[2] );
                n_elts = 1;
            }

            while ( j < init[1] ) {
                expr_code( stream, init[3 + j], 0 );
                save_local( stream, offset + j*elt_sz, elt_sz );
                ++j;
            }

            /* Check for an initialisation list that's not long enough.
             * If it's not, the standard requires zero initialisation of
             * the extra members. C99:6.7.8/21 */
            if ( j < n_elts ) {
                load_zero( stream );
                while ( j < n_elts ) {
                    save_local( stream, offset + j*elt_sz, elt_sz );
                    ++j;
                }
            }
        }
        /* Scalar initialisation */
        else {
            expr_code( stream, init, 0 );
            save_local( stream, offset, type_size(decl[2]) );
        }
    }
}

static
declaration(stream, node) {
    auto i = 2;
    while ( i < node[1] ) {
        auto decl = node[ 3 + i++ ];
        if ( node[3] && node[3][0] == 'exte' )
            extern_decl( decl );
        else
            auto_decl( stream, decl );
    }
}

static
label_stmt(stream, node, brk, cont, ret) {
    emit_label( stream, named_label( &node[3][3], 1 ) );
    stmt_code( stream, node[4], brk, cont, ret );
}

static
stmt_code(stream, node, brk, cont, ret) {
    /* Handle the null expression. */
    if ( !node ) return;

    auto op = node[0];

    /* For debugging purposes, put a blank line between each statement. */
    fputs("\n", stream);

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
        stmt_code( stream, node[ 3 + i++ ], brk, cont, ret );
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
        int_decl_n(stream, init[3]);
    else
        int_decl_s(stream, &init[3]);
}

/* Global-scope array declaration */
static
array_decl(stream, decl) {
    auto init = decl[5];
    auto lcvec = 0;
    auto sz = type_size( decl[2] );

    /* If the array initialiser contains strings, we have to first emit the 
     * strings and generate .LC numbers for them, which we store in lcvec. */
    if (init) {
        auto i = 0;
        while ( i < init[1] ) {
            auto ival = init[ 3 + i++ ];
            auto t = ival[0];
            if (t == 'str') {
                if (!lcvec) {
                    lcvec = malloc(4 * init[1]);  /* 4 == sizeof(int) */
                    memset( lcvec, 0, 4*init[1] );
                }
                lcvec[i] = new_clabel();
                defn_str(stream, &ival[3], lcvec[i]);
            }
        }
    }

    data_decl(stream, &decl[4][3]);

    if (init) {
        auto i = 0;
        while ( i < init[1] ) {
            auto ival = init[ 3 + i++ ];
            auto t = ival[0];
            if (t == 'num')
                int_decl_n(stream, ival[3]);
            else if (t == 'id')
                int_decl_s(stream, &ival[3]);
            else if (t == 'str')
                int_decl_lc(stream, lcvec[i]);
            else
                int_error("Unknown element in global-scope array: %Mc", t);
        }

        if ( 4 * init[1] < sz )
            zero_direct( stream, sz - 4*init[1] );
    }
    else 
        zero_direct( stream, sz );

    extern_decl( decl );
    free(lcvec);
}

static
struct_decl(stream, decl) {
    auto name = &decl[4][3], init = decl[5], sz = type_size( decl[2] );

    if ( init )
        int_error("Struct initialiser not supported");

    data_decl( stream, name );
    zero_direct( stream, sz );
}

/* This is only callled on top-level declarations */
codegen(stream, node) {
    auto decls = node, i = 2;
    auto storage = decls[3] ? decls[3][0] : 0;

    while ( i < node[1] ) {
        auto decl = node[ 3 + i++ ];
        auto init = decl[5];

        /* We only need to take action here if this is a definition.
         * Declarations that are not definitions have already been 
         * added to the symbol table by the parser. */
        /* TODO:  tentative definitions, duplicate definitions */
        if (init || storage == 'stat' ) {
            /* For debugging purposes, put a blank line between declarations. */
            fputs("\n", stream);

            auto type = decl[2], name = &decl[4][3];
            set_storage( stream, storage, name );

            if (type[0] == 'dclt' || type[0] == '*')
                int_decl( stream, name, init );
            else if (type[0] == '()')
                fn_decl( stream, name, type, init, decl[6] );
            else if (type[0] == '[]')
                array_decl( stream, decl );
            else if (type[0] == 'stru')
                struct_decl( stream, decl );
            else 
                int_error("Unexpected node in declaration %Mc", type[0]);
        }
    }
}
