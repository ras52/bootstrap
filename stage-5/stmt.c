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
if_stmt(in_loop) {
    /* Take over the keyword node */
    auto node = take_node(3); 
    skip_node('(');
    node[2] = expr();
    skip_node(')');

    node[3] = stmt( in_loop );

    if ( token && token[0] == 'else' ) {
        skip_node('else');
        node[4] = stmt( in_loop );
    }

    return node;
}

/*  do-stmt ::= 'do' stmt 'while' '(' expr ')' ';' */
static
do_stmt() {
    auto node = take_node(2); 

    node[2] = stmt(1);
    skip_node('whil');
    skip_node('(');
    node[3] = expr();
    skip_node(')');
    skip_node(';');

    return node;
}

/*  while-stmt ::= 'while' '(' expr ')' stmt */
static
while_stmt() {
    auto node = take_node(2);
    skip_node('(');
    node[2] = expr();
    skip_node(')');
    node[3] = stmt(1);

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

/*  break-stmt ::= 'break' ';' 
 *  cont-stmt  ::= 'continue ';' */
static
keywd_stmt(in_loop, keyword) {
    auto node = take_node(0); 
    if (!in_loop) 
        error("%s statement outside of loop", keyword);
    skip_node(';');
    return node;
}

static
init_array(elts, req_const) {
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

static
obj_decl(decl, req_const) {
    if (token && token[0] == '[') {
        auto array = take_node(2);
        array[0] = '[]';
        if (token[0] != 'num')
            error("Expected number as array size");
        array[3] = take_node(0);
        decl[2] = array;
        skip_node(']');
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

/*  init-decl ::= name ( init-int | '[' number ']' init-array )
 *  auto-stmt ::= 'auto' init-decl ( ',' init-decl )* ';' */
static
auto_stmt() {
    /* The 'auto' node is a wrapper vnode, and each element is an assignment */
    auto sz = 4, vnode = take_node(0);

    while (1) {
        auto decl = node_new('decl');
        if ( decl == vnode ) _exit();
        decl[1] = 3; /* args are type, name, init */

        decl[3] = take_node(0);
        if (decl[3][0] != 'id') error("Expected identifier in declaration");

        obj_decl(decl, 0);
        vnode = vnode_app(vnode, decl, &sz);
        if (token && token[0] == ';')
            break;
        skip_node(',');
    }
    skip_node(';');
    return vnode;
}

static
stmt(in_loop) {
    auto t;
    req_token(token);
    t = token[0];

    if      (t == '{'   ) return block( in_loop );
    else if (t == 'if'  ) return if_stmt( in_loop );
    else if (t == 'do'  ) return do_stmt();
    else if (t == 'whil') return while_stmt();
    else if (t == 'retu') return return_stmt();
    else if (t == 'brea') return keywd_stmt( in_loop, "break"    );
    else if (t == 'cont') return keywd_stmt( in_loop, "continue" );
    else if (t == 'auto') return auto_stmt();
    else                  return expr_stmt();
}

block(in_loop) {
    auto sz = 4, n = node_new('{}');

    skip_node('{');
    while ( token && token[0] != '}' )
        n = vnode_app( n, stmt(in_loop), &sz );

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
    skip_node(')');

    decl[4] = block(0);
    
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
        decl[1] = 3; /* args are type, fn or name, init.  fn type is '()' */

        decl[3] = take_node(0);
        if (decl[3][0] != 'id') error("Expected identifier in declaration");

        if (token[0] == '(') 
            return fn_decl( vnode, decl );

        obj_decl(decl, 1);
        vnode = vnode_app(vnode, decl, &sz);
        if (token && token[0] == ';')
            break;
        skip_node(',');
    }

    skip_node(';'); 
    return vnode;
}
