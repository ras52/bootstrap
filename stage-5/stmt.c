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
if_stmt() {
    /* Take over the keyword node */
    auto node = take_node(); 
    skip_node('(');
    node[1] = expr();
    skip_node(')');

    node[2] = stmt();

    if ( token && token[0] == 'else' ) {
        skip_node('else');
        node[3] = stmt();
    }

    return node;
}

/*  do-stmt ::= 'do' stmt 'while' '(' expr ')' ';' */
static
do_stmt() {
    auto node = take_node(); 

    node[1] = stmt();
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
    node[2] = stmt();

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
keywd_stmt() {
    auto node = take_node(); 
    skip_node(';');
    return node;
}

stmt() {
    auto t;
    req_token(token);
    t = token[0];

    if (t == '{')
        return block();
    else if (t == 'if')
        return if_stmt();
    else if (t == 'do')
        return do_stmt();
    else if (t == 'whil')
        return while_stmt();
    else if (t == 'retu')
        return return_stmt();
    else if (t == 'brea' || t == 'cont')
        return keywd_stmt();
    else
        return expr_stmt();
}

block() {
    auto sz = 4, n = vnode_new( '{}', 0, sz );

    skip_node('{');
    while ( token && token[0] != '}' )
        n = vnode_app( n, stmt(), &sz );

    skip_node('}');

    return n;
}
