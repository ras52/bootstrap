/* stmt.c  --  code to parse statements
 *
 * Copyright (C) 2013 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */

/* expr-stmt ::= expr? ';'  */
static
expr_stmt() {
    auto n = 0;
    if ( token[0] != ';' )
        n = expr();
    skip_node(';');
    return n;
}

/* if-stmt ::= 'if' '(' expr ')' stmt ( 'else' stmt )? */
static
if_stmt( fn, loop, swtch ) {
    auto node = take_node(3); 
    skip_node('(');
    node[2] = expr();
    skip_node(')');

    node[3] = stmt( fn, loop, swtch );

    if ( token && token[0] == 'else' ) {
        skip_node('else');
        node[4] = stmt( fn, loop, swtch );
    }

    return node;
}

/*  do-stmt ::= 'do' stmt 'while' '(' expr ')' ';' */
static
do_stmt( fn, swtch ) {
    auto node = take_node(2); 
    node[2] = stmt( fn, node, swtch );

    skip_node('whil');
    skip_node('(');
    node[3] = expr();
    skip_node(')');
    skip_node(';');

    return node;
}

/*  while-stmt ::= 'while' '(' expr ')' stmt */
static
while_stmt( fn, swtch ) {
    auto node = take_node(2);
    skip_node('(');
    node[2] = expr();
    skip_node(')');

    node[3] = stmt( fn, node, swtch );
    return node;
}

/*  for-stmt ::= 'for' '(' expr? ';' expr? ';' expr? ')' stmt */
static
for_stmt( fn, swtch ) {
    auto node = take_node(4);

    skip_node('(');
    if ( token && token[0] != ';' ) node[2] = expr();
    skip_node(';');
    if ( token && token[0] != ';' ) node[3] = expr();
    skip_node(';');
    if ( token && token[0] != ';' ) node[4] = expr();
    skip_node(')');

    node[5] = stmt( fn, node, swtch );
    return node;
}

/*  switch-stmt ::= 'switch' '(' expr ')' stmt */
static
switch_stmt( fn, loop ) {
    auto node = take_node(3);
    skip_node('(');
    node[2] = expr();
    skip_node(')');

    /* The 3rd argument is the switch table which is built up as case and
     * default statements are encountered.  The 4th argument is an integer
     * which is the capacity (i.e. allocated size) of the switch table;
     * because it's not a node, it's not included in the node's arity,
     * which was set to 3 by take_node. */
    node[4] = node_new('swtb');
    node[5] = 4;

    node[3] = stmt( fn, loop, node );
    return node;
}

/*  goto-stmt ::= 'goto' label ';' */
static
goto_stmt() {
    auto node = take_node(1); 

    if ( token[0] != 'id' )
        error("Expected an identifier at label in goto statement");
    node[2] = take_node(0);
    
    save_label( &node[2][2], 0 );

    return node;
}

/*  case-stmt ::= ( 'case' constant-expr | 'default' ) ':' stmt */
static
case_stmt( fn, loop, swtch ) {
    auto node = take_node(2);

    if ( !swtch )
        error( "%scase label found outside of switch statement",
               node[0] == 'case' ? "" : "default " );

    if (node[0] == 'case') {
        if (token[0] != 'num')
            error("Expected number as case label");
        node[2] = take_node(0);
    }

    /* Append the case label to the switch table (whose size, as an integer, 
     * is in the 4th operand slot of the switch).  Note that the switch table
     * is treated as weak links, so we won't double free the case label. */
    swtch[4] = vnode_app( swtch[4], node, &swtch[5] );

    skip_node(':');
    node[3] = stmt( fn, loop, swtch );

    return node;
}

/*  return-stmt ::= 'return' expr? ';' */
static
return_stmt() {
    auto node = take_node(1); 
    if ( token[0] != ';' )
        node[2] = expr();
    skip_node(';');
    return node;
}

/*  break-stmt ::= 'break' ';' */
static
break_stmt( loop, swtch ) {
    auto node = take_node(0); 
    if (!loop && !swtch) error("break outside of loop or switch");
    skip_node(';');
    return node;
}

/*  cont-stmt  ::= 'continue ';' */
static
cont_stmt( loop ) {
    auto node = take_node(0); 
    if (!loop) error("continue outside of loop");
    skip_node(';');
    return node;
}

static
init_array( elts, req_const ) {
    auto init, sz = 4;
    if (!token || token[0] != '{')
        error("Expected array initialiser");
    
    init = take_node(0);
    init[0] = '{}';
    while (1) {
        auto elt = req_const ? constant() : assign_expr();
        if (!elts--) 
            error("Too many array initialisers");
        init = vnode_app( init, elt, &sz );
        if (token && token[0] == '}')
            break;
        skip_node(',');
    }

    skip_node('}');
    return init;
}

/*  init-decl ::= name ( init-int | '[' number ']' init-array ) */
static
init_decl( fn, decl, req_const ) {
    if (token && token[0] == '[') {
        auto array = take_node(2);
        array[0] = '[]';
        if (token[0] != 'num')
            error("Expected number as array size");
        array[3] = take_node(0);
        decl[2] = array;
        skip_node(']');
    }

    /* If we're in a function, then it's an auto, and so it needs putting in 
     * scope.  This will need revising once we have function-scope statics. */
    if (fn) {
        decl_var(decl);

        auto foff = -frame_off();
        if (foff > fn[5])
            fn[5] = foff;
    }

    if (token && token[0] == '=') {
        auto type = decl[2];
        skip_node('=');
        if ( type && type[0] == '[]' )
            decl[4] = init_array( type[3][2], req_const );
        else
            decl[4] = req_const ? constant() : assign_expr();
    }
}

/*  auto-decl ::= 'auto' init-decl ( ',' init-decl )* ';' */
static
auto_decl( fn ) {
    /* The 'auto' node is a wrapper vnode, and each element is an assignment */
    auto sz = 4, vnode = take_node(0);

    while (1) {
        auto decl = node_new('decl');
        if ( decl == vnode ) _exit();
        decl[1] = 3; /* args are type, name, init */

        decl[3] = take_node(0);
        if (decl[3][0] != 'id') error("Expected identifier in declaration");

        init_decl( fn, decl, 0 );
        vnode = vnode_app(vnode, decl, &sz);
        if (token && token[0] == ';')
            break;
        skip_node(',');
    }
    skip_node(';');
    return vnode;
}

/* label-stmt ::= identifier ':' stmt */
static
label_stmt( fn, loop, swtch ) {
    auto label, name = take_node(0);

    if (token[0] != ':')
        int_error("Unexpected token '%Mc' found ending label", token[0]);
    label = take_node(2);
    label[2] = name;
    label[3] = stmt( fn, loop, swtch );

    save_label( &name[2], 1 );

    return label;
}

/* stmt-or-decl ::= stmt | auto-decl */
static
stmt_or_dcl( fn, loop, swtch ) {
    auto t;
    req_token(token);
    t = token[0];

    if (t == 'auto') return auto_decl( fn );
    else return stmt( fn, loop, swtch );
}

static
stmt( fn, loop, swtch ) {
    auto t;
    req_token(token);
    t = token[0];

    if      (t == '{'   ) return block( fn, loop, swtch );
    else if (t == 'if'  ) return if_stmt( fn, loop, swtch );
    else if (t == 'do'  ) return do_stmt( fn, swtch );
    else if (t == 'whil') return while_stmt( fn, swtch );
    else if (t == 'for' ) return for_stmt( fn, swtch );
    else if (t == 'swit') return switch_stmt( fn, loop );
    else if (t == 'goto') return goto_stmt();
    else if (t == 'retu') return return_stmt();
    else if (t == 'brea') return break_stmt( loop, swtch );
    else if (t == 'cont') return cont_stmt( loop );
    else if (t == 'case' 
          || t == 'defa') return case_stmt( fn, loop, swtch );

    /* We either have a labelled statement or we have a (possibly null)
     * expression statement.  Both can start with an identifier, so
     * we use peek_char() to look at the next non-whitespace character.
     * This works because the only use of : is in label or the second
     * part of a ternary, and we cannot get the ternary use here.  Note
     * that C++'s :: scope operator will break this. */
    else {
        if (t == 'id' && peek_char() == ':') 
            return label_stmt( fn, loop, swtch );
        else 
            return expr_stmt();
    }
}

block( fn, loop, swtch ) {
    auto sz = 4, n = node_new('{}');

    start_block();
    skip_node('{');
    while ( token && token[0] != '}' )
        n = vnode_app( n, stmt_or_dcl( fn, loop, swtch ), &sz );

    end_block();
    skip_node('}');

    return n;
}

/*  param-list ::= ( identifier ( ',' identifier )* )? */
static
param_list() {
    auto sz = 4, p = node_new('()');
    p = vnode_app( p, 0, &sz );  /* Place holder for return type */

    if ( token && token[0] == ')' ) 
        return p;

    while (1) {
        req_token( token );
        if (token[0] != 'id')
            error("Expected identifier in function parameter list");
        p = vnode_app( p, take_node(0), &sz );

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

static
constant() {
    auto t = token[0];
    if ( t == 'num' || t == 'chr' || t == 'id' | t == 'str' )
        return take_node(0);
    else
        error("Unexpected token '%Mc' while parsing expression", t);
}

static
fn_decl( vnode, decl ) {
    /* The standard doesn't allow things like:  int i, f() { }  
     * From a grammar point of view, this is an arbitrary restriction. */
    if ( vnode[1] )
        error("Function definition not allowed here");

    skip_node('(');
    decl[2] = param_list();
    start_fn(decl[2]);
    skip_node(')');

    /* We store the frame size in decl[5], which is where the 4th node arg
     * would go, but we've set arity=3 so it's unused space. */
    decl[5] = 0;
    decl[4] = block( decl, 0, 0 );

    end_fn();
   
    /* vnode is the storage 'static' or 'extern' */ 
    vnode[1] = 1;
    vnode[2] = decl;
    return vnode;
}

top_level() {
    /* We use an 'exte' or 'stat' declaration, depending on whether it's 
     * internal (declared "static") or external (the default).  The parse
     * tree is compatible with the parse tree for 'auto'. */
    auto sz = 4, vnode;
    if (token[0] == 'stat')
        vnode = take_node(0);
    else 
        vnode = node_new('exte');

    while (1) {
        auto decl = node_new('decl');
        decl[1] = 3; /* args are type, name, init.  type is '()' for fn */

        decl[3] = take_node(0);
        if (decl[3][0] != 'id') error("Expected identifier in declaration");

        if (token[0] == '(') 
            return fn_decl( vnode, decl );

        init_decl( 0, decl, 1 );
        vnode = vnode_app(vnode, decl, &sz);
        if (token && token[0] == ';')
            break;
        skip_node(',');
    }

    skip_node(';'); 
    return vnode;
}
