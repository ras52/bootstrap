/* macros.c  --  support for preprocessor macros
 *
 * Copyright (C) 2013, 2014, 2015 Richard Smith <richard@ex-parrot.com>
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

/* Replace instances of a parameter 'id' node with a 'macp' node. */
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

/* Check that any # or ## operators are in syntactically valid places. */
static
check_hash(mace) 
    struct node *mace;
{
    int i, n;

    /* Iterate over the replacement list looking for # and ## operators. 
     * This is necessary so that we generate an error even if the macro
     * is never expanded. */
    for ( i = 0, n = mace->arity; i < n; ++i ) {
        int c = mace->ops[i]->code;
        if ( c == '#' ) {
            if ( i+1 == n || mace->ops[i+1]->code != 'macp' )
                error("Expected macro parameter after # operator");
        }
        else if ( c == '##' ) {
            if ( i+1 == n || i == 0 )
                error("Argument missing to operator ##");
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

    /* Ensure any # or ## operators are syntactically valid. */
    check_hash( macd->ops[2] );

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
is_cpp_prgm(n)
    struct node *n;
{
    return n && n->code == 'prgm' && n->arity == 3 
        && node_streq( n->ops[0], "RBC" )
        && ( node_streq( n->ops[1], "cpp_mask" ) || 
             node_streq( n->ops[1], "cpp_unmask" ) );
}

static
node_concat( node_ptr, len_ptr, node )
    struct node **node_ptr, *node;
    int *len_ptr;
{
    int c = node->code;

    /* XXX We don't escape a 'str' or 'chr' node, because we escape the
     * whole string later.  Ideally this will change when string nodes
     * contain their value rather the quoted and escaped lexical values. */
    if ( c == 'id' || c == 'ppno' || c == 'str' || c == 'chr' ) {
        char *s = node_str(node); 
        node_strcat( node_ptr, len_ptr, s, strlen(s) );   
    }
    else
        node_strcat( node_ptr, len_ptr, &c, strnlen(&c, 4) );
}

static
struct node *
stringify(arg) 
    struct node *arg;
{
    struct node *str = new_node('str', 0), *str2;
    int i, l = 0;

    for ( i = 0; i != arg->arity; ++i ) {
        struct node *node = arg->ops[i];

        /* Don't stringify _Pragma("RBC cpp_mask"): otherwise it's an op. */
        if ( is_cpp_prgm(node) ) continue;

        if (l) node_lchar( &str, &l, ' ' );
        node_concat( &str, &l, node );
    }
    node_lchar( &str, &l, 0 );  /* NUL termination */
 
    /* XXX Now escape and quote the whole string.  This will go if we 
     * make the 'str' node contain its value. */
    str2 = esc_strnode( node_str(str) );
    free_node(str);
    return str2;
}

/* Check that NODE is a valid pp-token (e.g. following concatenation) and 
 * set its code accordingly. */
static
check_pptok(node) 
    struct node *node;
{
    char *s = node_str(node);
    int c = rchar(s, 0);

    /* A . could be the start of a ppnumber, or a '.' operator.  Handle it
     * first so we can continue with the isdigit loop below.  */
    if ( c == '.' && isdigit(s[1]) ) ++s;

    if ( isidchar1(c) ) {
        /* Once encoding prefixes for strings are supported, this needs 
         * handle.   E.g.  L ## "foo". */
        while ( c = rchar(++s, 0) )
            if ( !isidchar(c) )
                return 0;
        node->code = 'id';
    }
    else if ( isdigit(c) ) {
        while ( c = rchar(++s, 0) ) {
            if ( c != '.' && !isalnum(c) )
                return 0;
            /* 'E' and 'e' are for exponents, and can have a sign suffix.
             * C99 adds 'P' and 'p'. */
            if ( (c == 'e' || c == 'E') && (s[1] == '+' || s[1] == '-') )
                ++s;
        }
        node->code = 'ppno';
    }
    else if ( c == '\'' || c == '\"' ) {
        /* We're only called following concatenation, but one half may be 
         * empty; all we have to do is check for trailing garbage. 
         * (This is no longer true with C++11's user-defined literals.) */
        char q = c;
        while ( c = *++s ) {
            if ( c == q && s[1] ) return 0; /* An unescaped internal quote */
            else if ( c == '\\' && s[1] ) ++s;
        }
        node->code = q == '"' ? 'str' : 'chr';
    }
    else {
        int op = c;
        /* Maximum operator length is 3 (e.g. <<=). */
        if ( strlen(s) >= 4 ) return 0;

        /* Careful!  A one or two character operator might have garbage 
         * in the last byte(s) of its string. */
        if ( c = *s++ ) {
            lchar( &op, 1, c ); 
            if ( c = *s++ ) lchar( &op, 2, c );
        }

        /* Must allow generation of ## */
        if ( !is_op(op) && op != '#' && op != '##' ) 
            return 0;
        node->code = op;
    }

    return 1;
}

/* If *RHS_PTR is a node, then treat NODE as the left-hand argument to a 
 * ## operator, and concatenate them. */
static
do_concat(expansion, i) 
    struct node *expansion;
{
    struct node **lhs_p = 0, **rhs_p = 0;
    struct node *lhs, *rhs, *conc, *null = 0;
    int j;

    /* Locate the previous and subsequent token in the expansion. */
    j = i; do 
        lhs = *( lhs_p = &expansion->ops[j-1] );
    while ( !lhs && --j );

    j = i+1; do 
        rhs = *( rhs_p = &expansion->ops[j] );
    while ( !rhs && ++j < expansion->arity );

    /* If they are parameter expansions, find the last and/or first token
     * of the expansion proper (i.e. excluding any _Pragma("RBC cpp_mask")s); 
     * or leave LHS / RHS null as "a placemarker preprocessing token" if 
     * the expansion is empty. */
    if (lhs && lhs->code == 'mace') {
        for (j = lhs->arity; j; --j) {
            lhs = *( lhs_p = &lhs->ops[j-1] );
            if (lhs && !is_cpp_prgm(lhs)) goto got_lhs;
        }
        lhs = *( lhs_p = &null );
      got_lhs:;
    }

    if (rhs && rhs->code == 'mace') {
        for (j = 0; j < rhs->arity; ++j) {
            rhs = *( rhs_p = &rhs->ops[j] );
            if (rhs && !is_cpp_prgm(rhs)) goto got_rhs;
        }
        rhs = *( rhs_p = &null );
      got_rhs:;
    }
   
    /* Handle the cases when we're just concatening zero or one token. */
    lhs = *lhs_p; rhs = *rhs_p;
    if (!lhs && !rhs) conc = 0;
    else if (!lhs) conc = add_ref(rhs); 
    else if (!rhs) conc = add_ref(lhs);
    else {
        /* Use a temporary node of for the concatenated text. */
        int l = 0;
        conc = new_node('str', 0);

        node_concat( &conc, &l, lhs );
        node_concat( &conc, &l, rhs );
        node_lchar( &conc, &l, 0 );  /* NUL termination */

        /* Set its type */
        if ( !check_pptok( conc ) )
            error( "Concatention results in invalid preprocessing token: %s",
                   node_str(conc) );
    }

    /* Overwrite the ## token with the concatenated token. */
    free_node( expansion->ops[i] );
    expansion->ops[i] = conc;

    /* And delete the tokens that were used to generate it. */
    if (*lhs_p) { free_node(*lhs_p); *lhs_p = 0; }
    if (*rhs_p) { free_node(*rhs_p); *rhs_p = 0; }
}

/* Expand MACD using arguments ARGS. */
static
do_expand(macd, args)
    struct node *macd, *args;
{
    int i, j, n;
    char *name = node_str(macd->ops[0]);

    /* This is the replacement list in the definition. */
    struct node *mace = macd->ops[2];

    struct node *expansion = new_node('mace', 0);

    for ( i = 0; i < mace->arity; ++i ) {
        struct node *src = mace->ops[i];

        /* Substitute arguments for parameters */
        if ( src->code == 'macp' ) {
            int p = (int) src->ops[0], j;

            if ( !args ) int_error("Unexpected macro parameter");

            src = args->ops[p]; /* A 'mace' node containing the args. */

            /* Is it the argument of the stringification operator? */
            if ( i && mace->ops[i-1]->code == '#' )
                expansion = vnode_app( expansion, stringify(src) );

            else {
                /* We clone the argument list so we can overwrite it: 
                 * this is required so that a parameter can be concatenated
                 * more than once. */
                struct node *clone = new_node('mace', 0);
                for ( j = 0; j < src->arity; ++j ) 
                    clone = vnode_app( clone, add_ref(src->ops[j] ) );
                expansion = vnode_app( expansion, clone );
            }
        }

        /* The # operator is handled above. */
        else if ( src->code != '#' )
            expansion = vnode_app( expansion, add_ref(src) );
    }

    /* Handle any ## operators.  They will replace some nodes with NULL. */
    for ( i = 0; i < expansion->arity; ++i ) {
        struct node *node = expansion->ops[i];
        if ( node && node->code == '##' )
            do_concat( expansion, i );
    }

    /* Push a _Pragma("RBC cpp_unmask $name") node (for the end), then
     * push back the expansion in reverse order, and finally mask it.
     * This implements the standard behaviour that macros cannot recursively
     * reference themselves.  This is used in #define errno errno.
     * The [3] slot is abused to store an is-masked flag.  As arity is 3,
     * it is never freed. */
    set_token( mk_prgm_rbc( macd->ops[0], "cpp_unmask" ) ); 

    for ( n = expansion->arity; n; --n ) {
        int j;
        struct node *node = expansion->ops[n-1];
        /* We clone nodes, rather than using add_ref(), because the expression 
         * parser expectes to modify them. */
        if (node) {
            /* Flatten the result of parameter expansion */
            if (node->code == 'mace') {
                for ( j = node->arity; j; --j )
                    if ( node->ops[j-1] )
                        unget_token( clone_node( node->ops[j-1] ) );
            }
            else 
                unget_token( clone_node(node) );
        }
    }
    unget_token( mk_prgm_rbc( macd->ops[0], "cpp_mask" ) );

    free_node( expansion );
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
    while ( is_cpp_prgm(n) ) {
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
