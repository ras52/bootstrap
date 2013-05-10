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
    node[3] = expr();
    skip_node(')');

    node[4] = stmt( fn, loop, swtch );

    if ( peek_token() == 'else' ) {
        skip_node('else');
        node[5] = stmt( fn, loop, swtch );
    }

    return node;
}

/*  do-stmt ::= 'do' stmt 'while' '(' expr ')' ';' */
static
do_stmt( fn, swtch ) {
    auto node = take_node(2); 
    node[3] = stmt( fn, node, swtch );

    skip_node('whil');
    skip_node('(');
    node[4] = expr();
    skip_node(')');
    skip_node(';');

    return node;
}

/*  while-stmt ::= 'while' '(' expr ')' stmt */
static
while_stmt( fn, swtch ) {
    auto node = take_node(2);
    skip_node('(');
    node[3] = expr();
    skip_node(')');

    node[4] = stmt( fn, node, swtch );
    return node;
}

/*  for-stmt ::= 'for' '(' expr? ';' expr? ';' expr? ')' stmt */
static
for_stmt( fn, swtch ) {
    auto node = take_node(4);

    skip_node('(');
    if ( peek_token() != ';' ) node[3] = expr();
    skip_node(';');
    if ( peek_token() != ';' ) node[4] = expr();
    skip_node(';');
    if ( peek_token() != ')' ) node[5] = expr();
    skip_node(')');

    node[6] = stmt( fn, node, swtch );
    return node;
}

/*  switch-stmt ::= 'switch' '(' expr ')' stmt */
static
switch_stmt( fn, loop ) {
    auto node = take_node(3);
    skip_node('(');
    node[3] = expr();
    skip_node(')');

    /* The 3rd "operand" is the switch table which is built up as case and
     * default statements are encountered.  */
    node[5] = new_node('swtb');

    node[4] = stmt( fn, loop, node );
    return node;
}

/*  goto-stmt ::= 'goto' label ';' */
static
goto_stmt() {
    auto node = take_node(1); 

    if ( peek_token() != 'id' )
        error("Expected an identifier at label in goto statement");
    node[3] = take_node(0);
    
    save_label( &node[3][3], 0 );

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
        node[3] = take_node(0);
    }

    /* Append the case label to the switch table so it can do the forward
     * jump.  Note this is a duplicate reference of the case label, hence
     * calling add_ref(). */
    swtch[5] = vnode_app( swtch[4], add_ref(node) );

    skip_node(':');
    node[4] = stmt( fn, loop, swtch );

    return node;
}

/*  return-stmt ::= 'return' expr? ';' */
static
return_stmt() {
    auto node = take_node(1); 
    if ( peek_token() != ';' )
        node[3] = expr();
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
    auto init, elts;
    if (peek_token() != '{')
        error("Expected array initialiser");

    /* TODO:  This is valid: auto x[] = { 1, 2, 3 }; */
    if ( !type[4] ) 
        error("Initialising array of unknown size");
    else if ( type[4][0] != 'num' )
        int_error("Array size not an integer");

    elts = type[4][3];

    init = take_node(0);
    init[0] = '{}';
    while (1) {
        auto elt = req_const ? constant() : assign_expr();
        if (!elts--) 
            error("Too many array initialisers");
        init = vnode_app( init, elt );
        if (peek_token() == '}')
            break;
        skip_node(',');
    }

    skip_node('}');
    return init;
}

/*  primry-decl ::= name */
static
primry_decl(decl) {
    decl[4] = take_node(0);
    if (decl[4][0] != 'id') error("Expected identifier in declaration");
}

/*  postfx-decl ::= primry-decl ( '[' number ']' | '(' param-list ')' )? */
static
postfx_decl(decl) {
    primry_decl(decl);

    /* Iterate through the declarator postfixes building up the type.  
     * The innermost part of the type is the root, so, for example, 
     * foo[3][5] is an array of 3 elements.  Once we've parsed a postfix
     * on to the root of the decl tree, we therefore never alter that
     * and DECL tracks up to the lowest alterable point. */
    while (1) {
        auto postfix, t = peek_token();

        if (t == '[') {
            postfix = take_node(2);
            postfix[0] = '[]';
            if ( peek_token() == 'num' )
                postfix[4] = take_node(0);
            else
                error( "Declaration of an array with an unknown size" );
            skip_node(']');

        }

        else if (t == '(') {
            skip_node('(');
            postfix = param_list();
            skip_node(')');
        }

        else break;

        postfix[3] = decl[3];
        decl = decl[3] = postfix;
    }
}

/*  prefix-decl ::= '*'* postfx-decl */
static
prefix_decl(decl) {
    while (peek_token() == '*') {
        auto p = take_node(1);
        p[3] = decl[3];
        decl[3] = p;
    }

    postfx_decl(decl);
}

/*  Parse a declarator into a new 'decl' node.
 *
 *  declarator ::= postfx-decl */
static
declarator(dclt) {
    auto decl = new_node('decl');
    decl[1] = 3; /* the operands are: [3] type, [4] name, [5] init.
                  * Note the [6] slot is used for fuction stack size. */

    decl[3] = add_ref(dclt);
    prefix_decl(decl);

    /* Also store the type in decl[2], the node type field */
    decl[2] = add_ref(decl[3]);

    return decl;
}

/* Test whether the current token is a decl spec. */
static
is_dclspec(t) {
    return t == 'stat' || t == 'exte' || t == 'auto' || t == 'regi' ||
        t == 'char' || t == 'int' || t == 'shor' || t == 'long' ||
        t == 'sign' || t == 'unsi';
}

static
set_dclt(decls, field) {
    if (!decls[4]) {
        decls[4] = new_node('dclt');    
        decls[4][1] = 3;  /* arity */
    }

    else if ( decls[4][field] )
        error("Invalid combination of type specifiers");

    decls[4][field] = take_node(0);
}

/*  Parse a list of declaration specifiers, and return them in a 'dcls' node.
 *  Errors are given for invalid combinations.
 * 
 *  decl-specs   ::= ( storage-spec | type-spec )*
 *  storage-spec ::= 'extern' | 'static' | 'auto' | 'register'
 *  type-spec    ::= 'int' | 'char' | 'long' | 'short' | 'signed' | 'unsigned'
 */
static
decl_specs() {
    auto decls = new_node('dcls');
    decls[1] = 2;  /* arity */

    /* The dcls node has ops:  
     *   dcls[3] = storage class:     { extern, static, auto }
     *   dcls[4] = a dclt node:
     *   dcls[5...] = the declarators
     *
     * The dclt node has ops:
     *   dclt[3] = base of the type   { char, int }    --- always present
     *   dclt[4] = length modifier    { long, short }
     *   dclt[5] = signed modifier    { signed, unsigned }  */

    while (1) {
        auto t = peek_token();

        if ( t == 'stat' || t == 'exte' || t == 'auto' || t == 'regi' ) {
            if ( decls[3] )
                error("Multiple storage class specifiers found");

            decls[3] = take_node(0);
        }

        else if ( t == 'char' || t == 'int' )
            set_dclt(decls, 3);
        else if ( t == 'shor' || t == 'long' )
            set_dclt(decls, 4); /* TODO:  long long will become valid */
        else if ( t == 'sign' || t == 'unsi' )
            set_dclt(decls, 5);

        else break;
    }

    /* Sanity check and normalise the decl-spec combos. 
     * We don't check that at least one decl-spec is present because 
     * (i) implicit int on functions, and (ii) the caller has checked.  */

    if ( !decls[4] ) decls[4] = implct_int();
    else if ( !decls[4][3] ) decls[4][3] = new_node('int');
   
    if ( decls[4][3][0] == 'char' && decls[4][4] )
        error("Invalid combination of type specifiers");

    if ( decls[4][3][0] == 'int' && decls[4][5] == 'sign' ) {
        free_node( decls[4][5] );
        decls[4][5] = 0;
    }

    return decls;
}

/*  Parse a declaration inside function FN or at global scope (in which case
 *  FN is null), and add it to the VNODE representing a storage class 
 *  ('extern' or 'auto'), or a 'dcls' vnode if the storage class is implicit.  
 *  Current token is the first token of the declaration, e.g. 'auto'.
 *
 *  intialiser     ::= init-array | assign-expr
 *  init-decl      ::= declarator ( '=' initialiser )?
 *  init-decl-list ::= init-decl ( ',' init-decl )*
 *  declaration    ::= decl-specs ( init-decl-list ';' | declarator block )
 */
static
declaration( fn ) {
    auto decls = decl_specs();

    if ( !fn && decls[3] && ( decls[3][0] == 'auto' || decls[3][0] == 'regi' ) )
        error("Automatic variables are not allowed at file scope");

    /* TODO: Handle block-scope statics */
    if ( fn && decls[3] && decls[3][0] == 'stat' )
        error("Block-scope statics are not supported");

    while (1) {
        auto decl = declarator(decls[4]), name = &decl[4][3];
        
        /* Automatic declarations need space reserving in the frame. */
        if ( decls[3] && ( decls[3][0] == 'auto' || decls[3][0] == 'regi' ) ) {
            /* TODO:  This is valid: auto x[] = { 1, 2, 3 }; */
            if (decl[2] && decl[2][0] == '[]' && !decl[2][4])
                error("Automatic array of unknown size");

            chk_dup_sym( name, 0 );
            decl_var(decl);
        }
        /* External declarations still need storing in the symbol table */
        else {
            /* TODO:  Check for duplicate defintions at file scope */
            if (fn) chk_dup_sym( name, 1 );
            save_sym( decl, 0 );
        }

        /* Handle function definitions */
        if ( is_dclspec( peek_token() ) || peek_token() == '{' ) {
            if (fn) 
                error("Nested functions are not allowed: missing semicolon?");
            else if ( decl[2] && decl[2][0] == '()' ) {
                /* The standard doesn't allow things like:  int i, f() { }  
                 * From a grammar point of view, this is an arbitrary 
                 * restriction, but sensible from a sanity p.o.v. */
                if ( decls[1] > 2 )
                    error("Function definition not allowed here");

                fn_defn( decl );
                return vnode_app( decls, decl );
            }
            else
                error("Non-function declaration with function body");
        }

        /* Handle initialisers */
        if ( peek_token() == '=' ) {
            auto type = decl[2];
            skip_node('=');
            if ( type && type[0] == '[]' )
                decl[5] = init_array( type, !fn );
            else if ( type && type[0] == '()' )
                error("Function declarations cannot have initialiser lists");
            else
                decl[5] = fn ? assign_expr() : constant();
        }

        decls = vnode_app( decls, decl );
        if ( peek_token() == ';') break;
        skip_node(',');
    }
    skip_node(';');
    return decls;
}

/* label-stmt ::= identifier ':' stmt */
static
label_stmt( fn, loop, swtch ) {
    auto label, name = take_node(0);

    if (peek_token() != ':')
        int_error("Unexpected token '%Mc' found ending label", peek_token());
    label = take_node(2);
    label[3] = name;
    label[4] = stmt( fn, loop, swtch );

    save_label( &name[3], 1 );

    return label;
}

/*  stmt-or-decl ::= stmt | ( 'auto' | 'extern' ) decl-list */
static
stmt_or_dcl( fn, loop, swtch ) {
    if ( is_dclspec( peek_token() ) ) 
        return declaration( fn );
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

/* block ::= '{' stmt-or-decl* '}' */
block( fn, loop, swtch ) {
    auto n = new_node('{}');

    start_block();
    skip_node('{');

    while ( peek_token() != '}' )
        n = vnode_app( n, stmt_or_dcl( fn, loop, swtch ) );

    end_block();
    skip_node('}');

    return n;
}

/*  param-list ::= ( identifier ( ',' identifier )* )? */
static
param_list() {
    auto  p = new_node('()');
    p = vnode_app( p, 0 );  /* Place holder for return type */

    if ( peek_token() == ')' ) 
        return p;

    while (1) {
        req_token();
        if (peek_token() != 'id')
            error("Expected identifier in function parameter list");
        p = vnode_app( p, take_node(0) );

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

/* Scan through FN_DECL for an undeclared parameter matching PDECL, and 
 * replace it with the declaration. */
static
replc_param( fn_decl, pdecl ) {
    auto i = 1;
    
    while ( i < fn_decl[1] ) {
        auto n = fn_decl[ 3 + i ], name;

        if ( n[0] == 'id' ) name = n;
        else if ( n[0] == 'decl' ) name = n[4];
        else int_error("Unexpected node in parameter list");

        if ( strcmp( &name[3], &pdecl[4][3] ) == 0 ) {
            if ( n[0] == 'decl' ) 
                error("Multiple declarations for parameter '%s'", &name[2]);

            free_node( fn_decl[3+i] );
            fn_decl[3+i] = pdecl;
            return;
        }

        ++i;
    }

    error("No parameter called '%s'", &pdecl[4][3]);
}

/* Scan through FN_DECL for any undeclared parameters, and give implicit int
 * declarations to them. */
static
fini_params( fn_decl ) {
    auto i = 1;

    while ( i < fn_decl[1] ) {
        auto n = fn_decl[3+i];

        if ( n[0] == 'id' ) {
            auto decl = new_node('decl');
            decl[1] = 3; /* operands are: type, name, init. */
            decl[2] = add_ref( implct_int() );
            decl[3] = add_ref( decl[2] );
            decl[4] = n;
            fn_decl[3+i] = decl;
        }

        ++i;
    }
}

static
fn_defn( decl ) {
    /* Handle K&R style parameter declarations */
    while ( peek_token() != '{' ) {
        auto pdecls = decl_specs();
   
        if ( pdecls[3] && pdecls[3][0] != 'regi' )
            error("Storage specifiers not allowed on parameter declarations");

        while (1) {
            auto pdecl = declarator( pdecls[4] ), name = &pdecl[4][3];
       
            if (pdecl[2][0] == '()')
                error("Parameter declared as a function");

            /* TODO:  arrays get promoted to pointers, but they're not yet
             * yet supported either. */
            if (pdecl[2][0] == '[]')
                error("Arrays not supported as parameter");
 
            replc_param( decl[2], pdecl ); /* Takes ownership of pdecl */
            if ( peek_token() == ';') break;
            skip_node(',');
        }
        skip_node(';');
        free_node(pdecls);
    }
    fini_params( decl[2] );

    start_fn( decl[2] );
    decl[5] = block( decl, 0, 0 );

    /* We store the frame size in decl[6], which is where the 4th node arg
     * would go, but we've set arity=3 so it's unused space. */
    decl[6] = end_fn();
}

top_level() {
    return declaration( 0 );
}
