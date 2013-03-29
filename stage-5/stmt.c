/* stmt.c  --  code to parse statements
 *
 * Copyright (C) 2013 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */

/* expr-stmt ::= expr? ';'  */
static
expr_stmt() {
    auto n = 0;
    if ( peek_token() != ';' )
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

    if ( peek_token() == 'else' ) {
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
    if ( peek_token() != ';' ) node[2] = expr();
    skip_node(';');
    if ( peek_token() != ';' ) node[3] = expr();
    skip_node(';');
    if ( peek_token() != ')' ) node[4] = expr();
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

    if ( peek_token() != 'id' )
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
        if (peek_token() != 'num')
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
    if ( peek_token() != ';' )
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

/*  init-array ::= '{' assign-expr ( ',' assign-expr )* '}' */
static
init_array( type, req_const ) {
    auto init, sz = 4, elts;
    if (peek_token() != '{')
        error("Expected array initialiser");

    /* TODO:  This is valid: auto x[] = { 1, 2, 3 }; */
    if ( !type[3] ) 
        error("Initialising array of unknown size");

    elts = type[3][2];

    init = take_node(0);
    init[0] = '{}';
    while (1) {
        auto elt = req_const ? constant() : assign_expr();
        if (!elts--) 
            error("Too many array initialisers");
        init = vnode_app( init, elt, &sz );
        if (peek_token() == '}')
            break;
        skip_node(',');
    }

    skip_node('}');
    return init;
}

/*  declarator ::= name ( '[' number ']' | '(' param-list ')' )? */
static
declarator() {
    auto decl = node_new('decl');
    decl[1] = 3; /* args are type, name, init */

    decl[3] = take_node(0);
    if (decl[3][0] != 'id') error("Expected identifier in declaration");

    if (peek_token() == '[') {
        auto bounds = take_node(2);
        bounds[0] = '[]';
        if ( peek_token() == 'num' )
            bounds[3] = take_node(0);
        decl[2] = bounds;
        skip_node(']');
    }

    else if (peek_token() == '(') {
        skip_node('(');
        decl[2] = param_list();
        skip_node(')');
    }

    return decl;
}

/*  Parse a declaration inside function FN, and add it to the VNODE 
 *  representing a storage class ('extern' or 'auto'), or a 'dcls' vnode
 *  if the storage class is implicit.  Current token is an identifier. 
 *
 *  declaration ::= declarator ( '=' ( init-array | assign-expr ) )?
 *  decl-list   ::= declaration ( ',' declaration )* ';'  | declarator block
 */
static
decl_list( vnode, fn ) {
    auto sz = 4;

    while (1) {
        auto decl = declarator(), name = &decl[3][2];
        
        /* Automatic declarations need space reserving in the frame. */
        if ( vnode[0] == 'auto' ) {
            /* TODO:  This is valid: auto x[] = { 1, 2, 3 }; */
            if (decl[2] && decl[2][0] == '[]' && !decl[2][3])
                error("Automatic array of unknown size");

            chk_dup_sym( name, 0 );
            decl_var(decl);
        }
        /* External declarations still need storing in the symbol table */
        else {
            /* TODO:  Check for duplicate defintions at file scope */
            if (fn) chk_dup_sym( name, 1 );
            save_sym( name, 0, is_lv_decl(decl), 0 );
        }

        /* Handle function definitions */
        if ( peek_token() == '{' ) {
            if (fn) 
                error("Nested functions are not allowed");
            else if ( decl[2] && decl[2][0] == '()' )
                fn_defn( vnode, decl );
            else
                error("Non-function declaration with function body");
            return vnode;
        }

        /* Handle initialisers */
        if ( peek_token() == '=' ) {
            auto type = decl[2];
            skip_node('=');
            if ( type && type[0] == '[]' )
                decl[4] = init_array( type, !fn );
            else if ( type && type[0] == '()' )
                error("Function declarations cannot have initialiser lists");
            else
                decl[4] = fn ? assign_expr() : constant();
        }

        vnode = vnode_app( vnode, decl, &sz );
        if ( peek_token() == ';') break;
        skip_node(',');
    }
    skip_node(';');
    return vnode;
}

/* label-stmt ::= identifier ':' stmt */
static
label_stmt( fn, loop, swtch ) {
    auto label, name = take_node(0);

    if (peek_token() != ':')
        int_error("Unexpected token '%Mc' found ending label", peek_token());
    label = take_node(2);
    label[2] = name;
    label[3] = stmt( fn, loop, swtch );

    save_label( &name[2], 1 );

    return label;
}

/*  stmt-or-decl ::= stmt | ( 'auto' | 'extern' ) decl-list */
static
stmt_or_dcl( fn, loop, swtch ) {
    auto t = peek_token();
    if (t == 'auto' || t == 'exte') 
        return decl_list( take_node(0), fn );
    else
        return stmt( fn, loop, swtch );
}

static
stmt( fn, loop, swtch ) {
    auto t;
    req_token();
    t = peek_token();

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

    while ( peek_token() != '}' )
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

    if ( peek_token() == ')' ) 
        return p;

    while (1) {
        req_token();
        if (peek_token() != 'id')
            error("Expected identifier in function parameter list");
        p = vnode_app( p, take_node(0), &sz );

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

static
constant() {
    auto t = peek_token();
    if ( t == 'num' || t == 'chr' || t == 'id' | t == 'str' )
        return take_node(0);
    else
        error("Unexpected token '%Mc' while parsing expression", t);
}

static
fn_defn( vnode, decl ) {
    /* The standard doesn't allow things like:  int i, f() { }  
     * From a grammar point of view, this is an arbitrary restriction. */
    if ( vnode[1] )
        error("Function definition not allowed here");

    start_fn(decl[2]);
    decl[4] = block( decl, 0, 0 );

    /* We store the frame size in decl[5], which is where the 4th node arg
     * would go, but we've set arity=3 so it's unused space. */
    decl[5] = end_fn();
   
    /* vnode is the storage 'static' or 'extern' */ 
    vnode[1] = 1;
    vnode[2] = decl;
    return vnode;
}

top_level() {
    auto t = peek_token();

    /* If there's a storage class, use that for the vnode */
    if (t == 'stat' || t == 'exte')
        return decl_list( take_node(0), 0 );

    /* Otherwise use 'dcls' for the default behaviour.  (We cannot use
     * extern for the default, as "extern int i; and int i;" are not the 
     * same thing: the former is an external declaration; the latter is
     * a tentative definition [C90 6.7.2].)  */
    else 
        return decl_list( node_new('dcls'), 0 );
}
