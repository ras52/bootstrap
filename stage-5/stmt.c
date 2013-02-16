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
    auto node = take_node(); 
    skip_node('(');
    node[1] = expr();
    skip_node(')');

    node[2] = stmt( in_loop );

    if ( token && token[0] == 'else' ) {
        skip_node('else');
        node[3] = stmt( in_loop );
    }

    return node;
}

/*  do-stmt ::= 'do' stmt 'while' '(' expr ')' ';' */
static
do_stmt() {
    auto node = take_node(); 

    node[1] = stmt(1);
    skip_node('whil');
    skip_node('(');
    node[2] = expr();
    skip_node(')');
    skip_node(';');

    return node;
}

/*  while-stmt ::= 'while' '(' expr ')' stmt */
static
while_stmt() {
    auto node = take_node();
    skip_node('(');
    node[1] = expr();
    skip_node(')');
    node[2] = stmt(1);

    return node;
}

/*  return-stmt ::= 'return' expr? ';' */
static
return_stmt() {
    auto node = take_node(); 
    if ( token[0] != ';' )
        node[1] = expr();
    skip_node(';');
    return node;
}

/*  break-stmt ::= 'break' ';' 
 *  cont-stmt  ::= 'continue ';' */
static
keywd_stmt(in_loop, keyword) {
    auto node = take_node(); 
    if (!in_loop) 
        error("%s statement outside of loop", keyword);
    skip_node(';');
    return node;
}

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
    else                  return expr_stmt();
}

block(in_loop) {
    auto sz = 4, n = vnode_new( '{}', 0, sz );

    skip_node('{');
    while ( token && token[0] != '}' )
        n = vnode_app( n, stmt(in_loop), &sz );

    skip_node('}');

    return n;
}
