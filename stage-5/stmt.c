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

/*  init-decl ::= name ( init-int | '[' number ']' init-array )
 *  auto-stmt ::= 'auto' init-decl ( ',' init-decl )* ';' */
static
auto_stmt() {
    /* The 'auto' node is a wrapper vnode, and each element is an assignment */
    auto sz = 4, vnode = take_node(0);
    while (1) {
        auto decl = node_new('decl');   
        decl[1] = 2; /* args are name, init */

        decl[2] = take_node(0);
        if (decl[2][0] != 'id') error("Expected identifier in declaration");

        /* TODO: arrays */

        if (token && token[0] == '=') {
            skip_node('=');
            decl[3] = assign_expr();
        }

        vnode_app(vnode, decl, &sz);
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
param_list(fn) {
    auto sz = 4, p = node_new('()');
    vnode_app( p, fn, &sz );

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

top_level() {
    /* Set mode = 1 if we're parsing a function definition, and 2 otherwise. */
    auto mode = 0;

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
        decl[1] = 2; /* args are fn or name, init.  fn type is '()' */

        decl[2] = take_node(0);
        if (decl[2][0] != 'id') error("Expected identifier in declaration");

        /* TODO: arrays */

        if (token[0] == '(') {
            skip_node('(');
            decl[2] = param_list( decl[2] );
            skip_node(')');

            if (mode == 2 && token && token[0] == '{')
                error("Function definition not allowed here");

            decl[3] = block(0);
            mode = 1;
        }
    
        else if (token[0] == '=') {
            skip_node('=');
            decl[3] = constant();
        }

        if (!mode) mode = 2;  /* Not a function definition */

        vnode_app(vnode, decl, &sz);
        if (mode == 1 || token && token[0] == ';')
            break;
        skip_node(',');
    }

    if (mode == 2)
        skip_node(';'); 
    return vnode;
}
