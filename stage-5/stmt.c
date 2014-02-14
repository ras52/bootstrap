/* stmt.c  --  code to parse statements
 *
 * Copyright (C) 2013, 2014 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */

/* expr-stmt ::= expr? ';'  */
static
expr_stmt() {
    auto n = 0;
    if ( peek_token() != ';' ) {
        n = expr(0);
        if ( !n || !n[2] )
            int_error( "Expression has no type" );
    }
    skip_node(';');
    return n;
}

/* if-stmt ::= 'if' '(' expr ')' stmt ( 'else' stmt )? */
static
if_stmt( fn, loop, swtch ) {
    auto node = take_node(3); 
    skip_node('(');
    node[3] = expr(0);
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
    node[4] = expr(0);
    skip_node(')');
    skip_node(';');

    return node;
}

/*  while-stmt ::= 'while' '(' expr ')' stmt */
static
while_stmt( fn, swtch ) {
    auto node = take_node(2);
    skip_node('(');
    node[3] = expr(0);
    skip_node(')');

    node[4] = stmt( fn, node, swtch );
    return node;
}

/*  for-stmt ::= 'for' '(' expr? ';' expr? ';' expr? ')' stmt */
static
for_stmt( fn, swtch ) {
    auto node = take_node(4);

    skip_node('(');
    if ( peek_token() != ';' ) node[3] = expr(0);
    skip_node(';');
    if ( peek_token() != ';' ) node[4] = expr(0);
    skip_node(';');
    if ( peek_token() != ')' ) node[5] = expr(0);
    skip_node(')');

    node[6] = stmt( fn, node, swtch );
    return node;
}

/*  switch-stmt ::= 'switch' '(' expr ')' stmt */
static
switch_stmt( fn, loop ) {
    auto node = take_node(3);
    skip_node('(');
    node[3] = expr(0);
    skip_node(')');

    /* The 3rd "operand" is the switch table which is built up as case and
     * default statements are encountered.  */
    node[5] = new_node('swtb', 0);

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
        /* TODO expr(1) */
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
        node[3] = expr(0);
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
        auto elt = assign_expr( req_const );
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

/*  primry-decl ::= name | '(' prefix-decl ')' */
static
primry_decl(decl, phead) {
    auto t = peek_token();

    if ( t == 'id' ) {
        decl[4] = take_node(0);
    }
    else if ( t == '(') {
        skip_node('(');
        phead = prefix_decl(decl, phead);
        skip_node(')');
    }

    return phead;
}

/*  postfx-decl ::= primry-decl ( '[' number ']' | '(' param-list ')' )? */
static
postfx_decl(decl, phead) {
    phead = primry_decl(decl, phead);

    /* Iterate through the declarator postfixes building up the type.  
     * The innermost part of the type is the root, so, for example, 
     * foo[3][5] is an array of 3 elements.  Once we've parsed a postfix
     * on to the root of the decl tree, we therefore never alter that
     * and HEAD tracks up to the lowest alterable point. */
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
            /* A function declarator shall not specify a return type that is 
             * a function type or an array type. */
            if ( *phead && ( (*phead)[0] == '()' || (*phead)[0] == '[]' ) )
                error( "Invalid return type on function" );

            skip_node('(');
            postfix = param_list();
            skip_node(')');
        }

        else break;

        postfix[3] = *phead;
        *phead = postfix;
        phead = &postfix[3];
    }

    return phead;
}

/*  prefix-decl ::= '*'* postfx-decl */
static
prefix_decl(decl, phead) {
    auto base = 0, newphead = 0;

    while (peek_token() == '*') {
        auto p = take_node(1);
        if (!newphead) newphead = &p[3];
        else p[3] = base;
        base = p;
    }

    phead = postfx_decl(decl, phead);

    if (newphead) {
        *phead = base;
        return newphead;
    }
    else return phead;
}

/*  Parse a declarator into a new 'decl' node.
 *
 *  declarator ::= postfx-decl */
static
declarator(dclt) {
    /* The 'decl' node has operands: [3] storage, [4] name, [5] init.
     * The storage slot is filled in by the caller, where applicable.
     * Note the [6] slot is used as an int for the fuction stack size. */
    auto decl = new_node('decl', 3);

    auto phead = prefix_decl(decl, &decl[2]);
    *phead = add_ref(dclt);

    return decl;
}

/* Test whether the current token is a decl spec. */
is_dclspec() {
    auto t = peek_token();
    /* An identifier might a typedef-name, which is a decl-spec. */
    if ( t == 'id' ) {
        auto id = get_node();
        return is_typedef( &id[3] );
    }

    return t == 'stat' || t == 'exte' || t == 'auto' || t == 'regi' ||
         t == 'char' || t == 'int' || t == 'shor' || t == 'long' ||
         t == 'sign' || t == 'unsi' || t == 'stru';
}

static
set_dclt(decls, field) {
    if (!decls[4])
        decls[4] = new_node('dclt', 3);

    else if ( decls[4][0] != 'dclt' )
        error("Invalid combination of type specifiers");

    else if ( decls[4][field] )
        error("Invalid combination of type specifiers");

    decls[4][field] = take_node(0);
}

/*  Parse a struct tag or struct declaration.
 *
 *  struct-spec  ::= 'struct' ( name | name? '{' struct-decl-list '}' )
 */
static
struct_spec() {
    /* Struct node has one op:  stru[3] = name */
    auto stru = take_node(1), t;

    if ( peek_token() != 'id' )
        error("Expected a tag name in struct declaration");
    stru[3] = take_node(0);
    t = peek_token();
    save_tag( &stru[3][3], t == '{' );

    if ( t == '{' ) {
        skip_node('{');
        t = peek_token();
        if ( peek_token() == '}' )
            error("Empty structs are not allowed");

        /* We have defined the struct members inside the declaration() 
         * function, so there's nothing further to do here. */
        do free_node( declaration( 0, stru ) );
        while ( peek_token() != '}' );

        skip_node('}');

        /* Mark the type complete. */
        seal_tag( &stru[3][3] );
    }

    return stru;
}

/*  Parse a list of declaration specifiers, and return them in a 'dcls' node.
 *  Errors are given for invalid combinations.
 * 
 *  decl-specs   ::= ( storage-spec | type-spec | struct-spec )*
 *  storage-spec ::= 'extern' | 'static' | 'auto' | 'register' | 'typedef'
 *  type-spec    ::= 'int' | 'char' | 'long' | 'short' | 'signed' | 'unsigned'
 *                      | typedef-name
 */
static
decl_specs() {
    auto decls = new_node('dcls', 2);

    /* The dcls node has ops:  
     *   dcls[3] = storage class:     { extern, static, auto }
     *   dcls[4] = a type node:       { 'stru' or 'dclt' }
     *   dcls[5...] = the declarators
     *
     * For a basic type, the 'dclt' node, which is created on demand by 
     * set_dclt(), has ops:
     *   dclt[3] = base of the type   { char, int }    --- always present
     *   dclt[4] = length modifier    { long, short }
     *   dclt[5] = signed modifier    { signed, unsigned }  */

    while (1) {
        auto t = peek_token();

        if ( t == 'stat' || t == 'exte' || t == 'auto' || t == 'regi' || 
             t == 'type') {
            if ( decls[3] )
                error("Multiple storage class specifiers found");

            decls[3] = take_node(0);
        }

        else if ( t == 'stru' ) {
            /* 'int struct s' is never allowed. */
            if (decls[4])
                error("Invalid combination of type specifiers");

            decls[4] = struct_spec();
        }

        else if ( t == 'char' || t == 'int' )
            set_dclt(decls, 3);
        else if ( t == 'shor' || t == 'long' )
            set_dclt(decls, 4);
        else if ( t == 'sign' || t == 'unsi' )
            set_dclt(decls, 5);

        /* Typedefs are a bit tricky, because a typedef-name only interpretted
         * as a type-spec if there isn't already a type-spec. */
        else if ( t == 'id' && !decls[4] ) {
            auto id = take_node(0);
            if ( is_typedef( &id[3] ) ) {
                decls[4] = add_ref( lookup_type( &id[3] ) );
                free_node(id);
            }
            else {
                unget_token(id);
                break;
            }
        }

        else break;
    }

    /* Sanity check and normalise the decl-spec combos. 
     * We don't check that at least one decl-spec is present because 
     * (i) implicit int on functions, and (ii) the caller has checked.  */

    if ( !decls[4] )
        decls[4] = add_ref( implct_int() );

    if ( decls[4][0] == 'dclt' ) {
        if ( !decls[4][3] ) decls[4][3] = new_node('int', 0);
       
        if ( decls[4][3][0] == 'char' && decls[4][4] )
            error("Invalid combination of type specifiers");
    
        if ( decls[4][3][0] == 'int' && decls[4][5] 
               && decls[4][5][0] == 'sign' ) {
            free_node( decls[4][5] );
            decls[4][5] = 0;
        }
    }

    return decls;
}

/*  Parse a declaration inside a function FN, struct STRCT, or at global scope 
 *  (in which case bath FN and STRCT are null); add it to the VNODE 
 *  representing a storage class ('extern' or 'auto'), or a 'dcls' vnode 
 *  if the storage class is implicit.  
 *  Current token is the first token of the declaration, e.g. 'auto'.
 *
 *  intialiser     ::= init-array | assign-expr
 *  init-decl      ::= declarator ( '=' initialiser )?
 *  init-decl-list ::= init-decl ( ',' init-decl )*
 *  declaration    ::= decl-specs ( init-decl-list ';' | declarator block )
 */
static
declaration( fn, strct ) {
    auto decls = decl_specs();

    if ( strct && decls[3] )
        error("Declartion specifiers are not allowed on struct members");

    if ( !fn && decls[3] && ( decls[3][0] == 'auto' || decls[3][0] == 'regi' ) )
        error("Automatic variables are not allowed at file scope");

    /* TODO: Handle block-scope statics */
    if ( fn && decls[3] && decls[3][0] == 'stat' )
        error("Block-scope statics are not supported");

    /* Struct declarations needn't have declarators. */
    if ( !strct && peek_token() == ';' && decls[4][0] == 'stru' )
        /* stage-4 cc doesn't have goto, so use a sneaky else 
         * clause to get us to the skip_node(';') call. */ 
        ;

    else while (1) {
        auto decl = declarator(decls[4]), name; 
        if ( !decl[4] ) error("Declarator does not declare anything");
        name = &decl[4][3];

        /* Store storage specifier: particularly important for 'register'
         * so we can check it when taking the address of an identifier. */
        if ( decls[3] )
            decl[3] = add_ref( decls[3] );

        /* Automatic declarations need space reserving in the frame. 
         * A variable has automatic storage duration if (i) it is declared
         * with 'auto' or 'register', or (ii) if it declared at function scope
         * with no storage specifier and is not a function. */
        if ( decls[3] && ( decls[3][0] == 'auto' || decls[3][0] == 'regi' ) ||
             fn && !decls[3] && decl[2][0] != '()' ) {

            /* "The declaration of an identifier for a function that has block
             * scope shall have no explicit storage-class specifier other than
             * extern."  [C99 6.7.1/5] */
            if ( decl[2][0] == '()' )
                error("Invalid storage specifier on function declaration");

             /* TODO:  This is valid: auto x[] = { 1, 2, 3 }; */
            if (decl[2] && decl[2][0] == '[]' && !decl[2][4])
                error("Automatic array of unknown size");

            chk_dup_sym( name, 0 );
            decl_var(decl);
        }

        /* Typedefs are put in the symbol table too, as they're in the 
         * ordinary identifier namespace. */
        else if ( decls[3] && decls[3][0] == 'type' ) {
            chk_dup_sym( name, 0 );
            decl_type(decl);
        }

        /* External declarations still need storing in the symbol table */
        else if ( !strct ) {
            /* TODO:  Check for duplicate definitions at file scope. 
             * This is complicated by tentative definitions. */
            if (fn) chk_dup_sym( name, 1 );
            extern_decl( decl );
        }

        /* Struct members need storing in the symbol table for the struct */ 
        else {
            if ( decl[2][0] == '()' )
                error("Function declarations not permitted as struct members");

            decl_mem( &strct[3][3], decl );
        }

        /* Handle function definitions: is_dclpsec() matches a K&R parameter
         * declaration, and '{' is the start of a function body. */
        if ( is_dclspec() || peek_token() == '{' ) {
            if (fn) 
                error("Nested functions are not allowed: missing semicolon?");
            else if ( decl[2][0] == '()' ) {
                if ( decl[3] && decl[3][0] == 'type' )
                    error("Function definition not allowed here");

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
        else if ( decl[2][0] == '()' && decl[3] && decl[3][0] == 'stat' )
            /* "The declaration of an identifier for a function that has block
             * scope shall have no explicit storage-class specifier other than
             * extern."  [C99 6.7.1/5]  Static is clearly allowed on 
             * a function definition, though. */
            error("Invalid storage specifier on function declaration");
        

        /* Handle initialisers */
        if ( peek_token() == '=' ) {
            auto type = decl[2];
            skip_node('=');
            if ( strct ) 
                error("Initialiser not allowed on struct member");
            else if ( type && type[0] == '[]' )
                decl[5] = init_array( type, !fn );
            else if ( type && type[0] == '()' )
                error("Function declarations cannot have initialiser lists");
            else
                decl[5] = assign_expr(!fn);
        }

        decls = vnode_app( decls, decl );
        if ( peek_token() == ';') break;
        skip_node(',');
    }

    skip_node(';');
    return decls;
}

type_name() {
    auto decls, decl;
    decls = decl_specs();
    if ( decls[3] ) error("Declartion specifiers not allowed on type name");

    decl = declarator(decls[4]);
    if ( decl[4] ) error("Type name must not declare an object");
    if ( decl[5] ) error("Type name must not have an initialiser");

    free_node(decls);

    return decl;
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
    if ( is_dclspec() ) 
        return declaration( fn, 0 );
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

    else if (t == 'id') {
        /* We either have a labelled statement or we have a (possibly null)
         * expression statement.  Both can start with an identifier, so
         * we use peek_token() to look at the following token.
         * This works because the only use of : is in label or the second
         * part of a ternary, and we cannot get the ternary use here. */
        auto id = take_node(0);
        auto is_label = peek_token() == ':';
        unget_token(id);

        if (is_label) return label_stmt( fn, loop, swtch );
        else return expr_stmt();
    }
    else return expr_stmt();
}

/* block ::= '{' stmt-or-decl* '}' */
block( fn, loop, swtch ) {
    auto n = new_node('{}', 0);

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
    auto  p = new_node('()', 0);
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
                error("Multiple declarations for parameter '%s'", &name[3]);
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
            auto decl = new_node('decl', 3);
            decl[2] = add_ref( implct_int() );
            /* operands are: type, name, init (which is null here). */
            decl[3] = add_ref( implct_int() );
            decl[4] = n;
            fn_decl[3+i] = decl;
        }

        ++i;
    }
}

/* Handle K&R style parameter declarations */
static
knr_params( decl ) {
    while ( peek_token() != '{' ) {
        auto pdecls = decl_specs();
   
        if ( pdecls[3] && pdecls[3][0] != 'regi' )
            error("Storage specifiers not allowed on parameter declarations");

        while (1) {
            auto pdecl = declarator( pdecls[4] ), name;
            if ( !pdecl[4] ) 
                error("Parameter declarator does not declare anything");
            name = &pdecl[4][3];
       
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
}

static
fn_defn( decl ) {
    knr_params( decl );
    start_fn( decl[2] );
    decl[5] = block( decl, 0, 0 );

    /* We store the frame size in decl[6], which is where the 4th node arg
     * would go, but we've set arity=3 so it's unused space. */
    decl[6] = end_fn();
}

top_level() {
    return declaration( 0, 0 );
}
