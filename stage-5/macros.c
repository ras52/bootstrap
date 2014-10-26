/* macros.c  --  support for preprocessor macros
 *
 * Copyright (C) 2013, 2014 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */

struct node* get_word();
struct node* next();
struct node* get_node();
struct node* new_node();
struct node* vnode_app();
char* node_str();

/* A macro definition is just a 'macd' node with 3 operands: 
 * [0] name, [1] is the parameter list, [2] expansion;
 * slot [3] is (ab)used for an is-masked flag.  The expansion op
 * is a 'mace' vnode all of whose operands are tokens in the expansion.  
 * The node struct is documented in nodenew.c.
 * If only we had a preprocessor to avoid these duplications ... :-) */
struct node {
    int code;           /* character code for the node, e.g. '+' or 'if'. */
    int arity;          /* the number of nodes in the ops[] array. */
    struct node* type;  /* Always NULL in the preprocessor. */
    struct node* ops[4];
};

/* This is a dynamic array of node*s. */
static
struct macro_vec {
    struct node** start;
    struct node** end;
    struct node** end_store;
} macro_vec;

static
vec_grow(macro)
    struct node* macro;
{
    int sz = macro_vec.end - macro_vec.start;
    if ( macro_vec.end_store == macro_vec.end ) {
        int new_sz = sz ? 2*sz : 1;
        struct node** p
            = realloc( macro_vec.start, new_sz * sizeof(struct node*) );
        macro_vec.start = p;
        macro_vec.end = p + sz;
        macro_vec.end_store = p + new_sz;
    }
    *macro_vec.end++ = macro;
}

static
struct node*
get_macro(name)
    char* name;
{
    struct node **i = macro_vec.start, **e = macro_vec.end;
    for (; i != e; ++i )
        if ( node_streq( (*i)->ops[0], name ) )
            return *i;
    return 0;
}

static
is_builtin(name) {
    return strcmp( "__FILE__", name ) == 0 || strcmp( "__LINE__", name ) == 0;
}

static
struct node *
ident_list(stream) {
    int c;
    struct node *p = new_node('()', 0);
    p = vnode_app( p, 0 );  /* Macros don't have return type */

    fgetc(stream);  /* skip ')' */
    c = skip_hwhite(stream);
    if ( c == ')' ) 
        return p;

    while (1) {
        if ( !isidchar1(c) )
            error( "Expected identifier as macro parameter" );
        p = vnode_app( p, get_word( stream, c ) );
    
        c = skip_hwhite(stream);
        if ( c == ')' ) break;
        else if ( c != ',' ) 
            error("Expected comma separating macro parameters");

        c = skip_hwhite(stream);
    }

    /* We've just called skip_hwhite() whhich as returned ')', which means
     * that character has been read and not ungetc()'d. */
    return p;
}

static
find_params(macd) 
    struct node *macd;
{
    int i, j;
    struct node *mace = macd->ops[2], *params = macd->ops[1];

    /* Iterate over the replacement list looking for identifiers */
    for ( i = 0; i < mace->arity; ++i ) {
        struct node *n = mace->ops[i];
        if ( n->code == 'id' ) {
            /* Iterate over the parameter list checking N is a parameter */
            for ( j = 1; j < params->arity; ++j ) {
                struct node *p = params->ops[j];
                if ( strcmp( node_str(n), node_str(p) ) == 0 ) {
                    /* Replace the 'id' node with a parameter node, 'macp'.
                     * This has arity 0, and abuses slot [0] for the parameter
                     * number. */
                    free_node(n);
                    n = mace->ops[i] = new_node('macp', 0);
                    n->ops[0] = (struct node*) j;
                    break;
                }
            }
        }
    }
}

/* Handle a #define directive. */
defn_direct(stream) {
    struct node *macd = new_node('macd', 3);
    char *name;

    int c = skip_hwhite(stream), i;

    if ( !isidchar1(c) )
        error("Expected identifier in #define directive");

    name = node_str( macd->ops[0] = get_word( stream, c ) );

    if ( strcmp(name, "defined") == 0 || is_builtin(name) )
        error("Cannot redefine '%s'", name);

    /* We need to test here for *any* whitespace, so we bypass the normal 
     * test routine with skip_white(). */
    ungetc( c = fgetc(stream), stream );

    /* lparen doesn't permit preceding whitespace */
    if ( c == '(' )
        macd->ops[1] = ident_list(stream);

    /* Require whitespace (incl. '\n') before the expansion list.  
     * XXX It's not clear to me that this is correct in C90. */
    else if ( !isspace(c) ) 
        error("Expected whitespace before macro replacement list");

    macd->ops[2] = new_node('mace', 0);
    pp_slurp(stream, &macd->ops[2], 0);

    /* TODO:  We're allowed repeat definitions if they're identical. */
    if ( get_macro(name) )
        error("Duplicate definition of %s", name);

    /* Locate macro parameters in the replacement list */
    if ( macd->ops[1] ) 
        find_params(macd);

    vec_grow(macd);
}

/* Handle an #undef directive */
unde_direct(stream) {
    struct node **i = macro_vec.start, **e = macro_vec.end;
    struct node *tok; 
    char* name;
    int c = skip_hwhite(stream);

    if ( !isidchar1(c) )
        error("Expected identifier in #define directive");
    tok = get_word( stream, c );
    name = node_str(tok);

    if ( strcmp(name, "defined") == 0 || is_builtin(name) )
        error("Cannot undefine '%s'", name);

    for (; i != e; ++i )
        if ( node_streq( (*i)->ops[0], name ) ) {
            /* Remove defn from the macro_vec */
            free_node(*i);  *i = 0;
            memmove( i, i+1, (e-i-1) * sizeof(struct node*) );
            --macro_vec.end;
            break;
        }

    free_node(tok);
}

struct node* new_strnode();
struct node* add_ref();

static
mk_prgm_rbc(strnode, name)
    struct node *strnode;
    char *name;
{
    /* Used to generate nodes like _Pragma("RBC cpp_unmask $name"). */
    struct node *node = new_node( 'prgm', 3 );
    node->ops[0] = new_strnode('id', "RBC");
    node->ops[1] = new_strnode('id', name);
    node->ops[2] = add_ref(strnode);
    return node;
}

static
clone_node(src)
    struct node *src;
{
    if (!src) 
        return src;

    else if ( src->code == 'str' || src->code == 'chr' ||
              src->code == 'ppno' || src->code == 'id' )
        return new_strnode( src->code, node_str(src) );

    else if ( src->arity == 0 ) 
        return new_node( src->code, 0 );

    else if ( src->arity <= 4 ) {
        struct node *dest = new_node( src->code, src->arity );

        if ( src->code != 'prgm' ) 
            int_error( "Cloning node type %Mc\n", src->code );

        /* Shallow copy */
        int n;
        for ( n = 0; n < src->arity; ++n )
            dest->ops[n] = add_ref( src->ops[n] );
        for ( ; n < 4; ++n )
            dest->ops[n] = src->ops[n];
        return dest;
    }

    else int_error("Cloning a vnode");
}

static
struct node *
esc_strnode(str)
    char *str;
{
    struct node *node = new_node('str', 0);
    int i = 0, c;

    node_lchar( &node, &i, '"' );
    while (c = (int)*str++) {
        int e = 0;
        if ( c == '\n' ) e = 'n';
        else if ( c == '\t' ) e = 't';
        else if ( c == '\0' ) e = '0';
        else if ( c == '"' || c == '\\' ) e = c;
        if (e) {
            node_lchar( &node, &i, '\\' );
            node_lchar( &node, &i, e );
        } else 
            node_lchar( &node, &i, c );
    }
    node_lchar( &node, &i, '"' );

    return node;
}

static
do_builtin(name) 
    char *name;
{
    /* These all expand to a single pp-token. */
    struct node *res;

    if (strcmp(name, "__LINE__") == 0) {
        char buf[16] = {0}; 
        snprintf(buf, 16, "%d", get_line());
        res = new_strnode('ppno', buf);
    }
    
    else if (strcmp(name, "__FILE__") == 0)
        res = esc_strnode( getfilename() );
    
    else
        int_error("Unhandled built-in");
    
    set_token(res);
}

static
do_expand(macd, args)
    struct node *macd, *args;
{
    int n;
    struct node *mace;
    char *name = node_str(macd->ops[0]);

    /* Mask the macro, and push a _Pragma("RBC cpp_unmask $name") node.
     * This implements the standard behaviour that macros cannot recursively
     * reference themselves.  This is used in #define errno errno.
     * The [3] slot is abused to store an is-masked flag.  As arity is 3,
     * it is never freed. */
    set_token( mk_prgm_rbc( macd->ops[0], "cpp_unmask" ) ); 

    /* By pushing the tokens back in this way, we ensure that we rescan 
     * the expanded token sequence.  We clone them, rather than using 
     * add_ref(), because the expression parser expectes to modify them. */
    mace = macd->ops[2];
    for ( n = mace->arity; n; --n ) {
        struct node *src = mace->ops[n-1];

        /* Substitute arguments for parameters */
        if ( src->code == 'macp' ) {
            int p = (int) src->ops[0], j;

            if ( !args ) int_error( 
                "Attempting argument substitution in an object-like macro" 
            );

            src = args->ops[p];
            for ( j = src->arity; j; --j ) 
                unget_token( clone_node( src->ops[j-1] ) );
        }
            
        else unget_token( clone_node(src) );
    }
    unget_token( mk_prgm_rbc( macd->ops[0], "cpp_mask" ) );
}

/* Implements the defined() preprocessor function. */
pp_defined(name)
    char* name;
{
    return is_builtin(name) || get_macro(name) ? 1 : 0;
}

cpp_setmask(name, delta) 
    char* name;
    int delta;
{
    struct node *macro = get_macro(name);
    if ( !macro )
        int_error("Unable to set mask on non-existent macro '%s'", name);
    /* The 'macd' arity is 3, so the unused [3] slot is abused to store an 
     * is-masked flag. */
    int cur = (int)macro->ops[3];
    if ( cur + delta < 0 ) 
        int_error("Macro '%s' is already unmasked", name);
    macro->ops[3] = (struct node*)( cur + delta );
}

fini_macros() {
    struct node **i = macro_vec.start, **e = macro_vec.end;
    for (; i != e; ++i ) {
        if ( (*i)->ops[3] ) 
            int_error( "Macro '%s' left masked", node_str( (*i)->ops[0] ) );
        free_node(*i);
    }
}

/* Parse a -D command-line option */
parse_d_opt(arg) 
    char* arg;
{
    struct node *macd = new_node('macd', 3), *t;
    int i = 0, c;

    macd->ops[0] = new_node('id', 0);

    /* Read up to the end of the argument or the first '=' to get the name */
    while ( ( c = rchar(arg++, 0) ) && c != '=' )
        node_lchar( &macd->ops[0], &i, c );
    node_lchar( &macd->ops[0], &i, 0 );

    /* POSIX says: If no = value is given, a value of 1 shall be used. */
    if (!c) arg = "1";
    scan_str(arg);

    macd->ops[2] = new_node('mace', 0);
    while ( t = next() )
        macd->ops[2] = vnode_app( macd->ops[2], t );
    
    vec_grow(macd);
}

/* The current token is the '('.  Read macro arguments to the corresponding 
 * ')', which is left as head of the stack. */
static
macro_args(name_node, next_fn) 
    struct node *name_node;
    struct node *(*next_fn)();
{
    char *name = node_str(name_node);
    struct node *macro = get_macro(name);
    struct node *args = new_node('()', 0), *arg = 0, *n;
    int pdepth = 0, edepth = 0, n_params, n_args;

    args = vnode_app( args, name_node );

    while (1) {
        n = get_node();

        /* We don't know whether this is a EOF or end of line: that 
         * depends on whether next_fn is pp_next or next. */
        if (!n) error("Incomplete macro argument list");

        if ( n->code == '(' ) ++pdepth;
        else if ( n->code == ')' ) {
            if ( pdepth == 0 ) break;  
            else --pdepth;
        }

        /* If the first argument is empty, e.g. in f(,x) then arg is 
         * undefined.  This is necessary because f() can be a macro
         * invocation with one empty argument or with zero arguments. */
        if (!arg) arg = new_node('mace', 0);

        if ( n->code == ',' && pdepth == 0 && edepth == 0 ) {
            args = vnode_app( args, arg );
            arg = new_node('mace', 0);
            free_node(n);
        }

        /* "A parameter in the replacement list [...] is replaced by the 
         * corresponding argument after all macros contained therein have 
         * been expanded." [C11 6.10.3.1/1]  This is doing that expansion
         * of macros in arguments. */
        else if ( n->code == 'id' && try_expand(n, next_fn, &arg) ) 
            continue;
        
        else {
            /* If it's a preprocessor pragma, it needs processing, but also
             * needs to be passed through to the main preprocessor loop. */
            if ( n->code == 'prgm' ) edepth += cpp_pragma(n);
            arg = vnode_app( arg, n );
        }

        next_fn();        
    }

    /* Note: The '()' nodes have slot zero reserved for a return type
     * or callee.  This is for consistency with elsewhere in the code.  
     * Hence using arity-1 throughout. */
    n_params = macro->ops[1]->arity-1;
    n_args = args->arity-1;

    /* We cannot distinguish between zero arguments and one empty argument,
     * so we try to DTRT. */
    if ( n_params == 1 && n_args == 0 && !arg )
        arg = new_node('mace', 0);

    if (arg) { args = vnode_app( args, arg ); ++n_args; }

    if ( n_params != n_args )
        error( "Macro '%s' expects %d arguments but called with %d arguments",
               name, n_params, n_args );

    return args;
}

static
has_brack(next_fn, arg)
    struct node *(*next_fn)();
    struct node **arg;
{
    struct node *masks = new_node('tmp',0);
    struct node *name = get_node(), *n = next_fn();
    int i;

    /* Skip any _Pragma("RBC cpp_mask $n") directive. */
    while ( n && n->code == 'prgm' && n->arity == 3 &&
            node_streq( n->ops[0], "RBC" ) && 
            ( node_streq( n->ops[1], "cpp_mask" ) || 
              node_streq( n->ops[1], "cpp_unmask" ) ) ) {
        masks = vnode_app( masks, n );
        n = next_fn();
    }

    if ( n && n->code == '(' ) { 
        /* Process the pragmas */
        for ( i=0; i<masks->arity; ++i )
            cpp_pragma( masks->ops[i] );

        /* This is where we finally skip the '(' */
        free_node(n);
        next_fn(); 

        if (arg)
            for ( i=0; i<masks->arity; ++i )
                *arg = vnode_app(*arg, add_ref( masks->ops[i] ) );

        free_node(masks);
        return 1; 
    }
    else {
        /* Restore the pragmas: decrement arity to prevent freeing */
        while ( masks->arity )
            unget_token( masks->ops[ --masks->arity ] );
        free_node(masks);
    
        unget_token(name);
        return 0;
    }
}

/* Called with the name as the current token */
try_expand(node, next_fn, arg)
    struct node *node;
    struct node *(*next_fn)();
    struct node **arg; /* Pass-through for cpp masks */
{
    char *name = node_str(node);
    struct node *macd = get_macro(name);

    /* Slot [3] is used as an is-masked flag. */
    if (!macd || macd->ops[3])
        return 0;

    /* All builtins are object-like, currently. */
    else if ( is_builtin(name) ) {
        do_builtin(name);
        return 1;
    }

    /* Slot [1] is the arguments */
    else if ( ! macd->ops[1] ) {
        /* It's a macro that will be expanded and pushed back on to
         * the parser stack by do_expand: we do that to ensure proper 
         * re-scanning. */
        do_expand(macd, 0);
        return 1;
    }

    /* Function-like macros are only expanded if the next pp token is an open 
     * bracket.  This allows the (fn)(a, b) syntax to be used to suppress 
     * macro expansion of a macro that hides a function of the same name. */
    else if ( has_brack(next_fn, arg) ) {
        struct node *args = macro_args(node, next_fn);
        do_expand(macd, args); /* will overwrite ')' */
        free_node(args);
        return 1;
    }

    /* The remaining case is a function-like macro name not invoked as such. */
    else return 0;
}
