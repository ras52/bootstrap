/* macros.c  --  support for preprocessor macros
 *
 * Copyright (C) 2013, 2014, 2015, 2016, 2018 
 * Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */

struct node* get_word();
struct node* new_node();
struct node* vnode_app();
struct node* vnode_prep();
struct node* vnode_copy();
char* node_str();
free_node();

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
macp_arg(macp, args) 
    struct node *macp, *args;
{
     /* The number is in the macp [0] slot (which is okay as arity is 0). 
      * The parameter number is one-based, and the macro name is put in the 
      * args->ops[0] slot. */
     return args->ops[ (int) macp->ops[0] ];
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
check_hash(macd) 
    struct node *macd;
{
    int i, n;
    struct node *mace = macd->ops[2];

    /* Iterate over the replacement list looking for # and ## operators. 
     * This is necessary so that we generate an error even if the macro
     * is never expanded. */
    for ( i = 0, n = mace->arity; i < n; ++i ) {
        int c = mace->ops[i]->code;

        /* "Each # preprocessing token in the replacement list for a 
         * function-like macro shall be followed by a parameter as the 
         * next preprocessing token in the replacement list." [C90 6.8.3.2] */
        if ( c == '#' && macd->ops[1] ) {
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
     * It's not clear that this is correct in C90, but we treat C99 as
     * a clarification to C90 rather than a change in behaviour. */
    else if ( !isspace(c) ) 
        error("Expected whitespace before macro replacement list");

    macd->ops[2] = new_node('mace', 0);
    pp_slurp(stream, &macd->ops[2], 0);

    /* TODO:  We're allowed repeat definitions if they're identical. 
     * This needs better whitespace preservation than we currently have. */
    if ( get_macro(name) )
        error("Duplicate definition of %s", name);

    /* Locate macro parameters in the replacement list */
    if ( macd->ops[1] ) 
        find_params(macd);

    /* Ensure any # or ## operators are syntactically valid. */
    check_hash( macd );

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
mk_prgm_rbc(strnode, type)
    struct node *strnode;
    char *type;
{
    /* Used to generate nodes like _Pragma("RBC cpp_unmask $name"). */
    struct node *node = new_node( 'prgm', 3 );
    node->ops[0] = new_strnode('id', "RBC");
    node->ops[1] = new_strnode('id', type);
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
        return new_strnode('ppno', buf);
    }
    
    else if (strcmp(name, "__FILE__") == 0)
        return esc_strnode( getfilename() );
    
    else
        int_error("Unhandled built-in");    
}

static
is_rbc_prgm(n)
    struct node *n;
{
    return n && n->code == 'prgm' && n->arity >= 2 
        && node_streq( n->ops[0], "RBC" );
}

static
is_cpp_prgm(n)
    struct node *n;
{
    return is_rbc_prgm(n)
        && ( node_streq( n->ops[1], "cpp_mask" ) || 
             node_streq( n->ops[1], "cpp_unmask" ) );
}

/* This is also used in generating a header name to include. */
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

/* Pass over MACD handling all # operators. */
static 
struct node *
do_stringfy(mace, args)
    struct node *mace, *args;
{
    struct node *expansion;
    int i;

    /* Don't do bother cloning the list if there are none */
    for ( i = 0; i < mace->arity; ++i ) 
        if ( mace->ops[i]->code == '#' ) break;
    if (i == mace->arity) return mace;

    expansion = new_node('mace', 0);

    for ( i = 0; i < mace->arity; ++i ) {
        struct node *node = mace->ops[i];

        /* Is it argument of a stringification operator? */
        if ( node->code == '#' ) {
            if (!args) int_error("Unexpected macro parameter");
            if ( i+1 == mace->arity || mace->ops[i+1]->code != 'macp' )
                int_error("Expected macro parameter");

            struct node* arg = macp_arg( mace->ops[i+1], args );
            expansion = vnode_app( expansion, stringify(arg) );

            ++i;  /* skip the 'macp' */
        }

        /* Pass through other tokens. */
        else expansion = vnode_app( expansion, add_ref(node) );
    }

    free_node(mace);
    return expansion;
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
        while ( c = rchar(++s, 0) ) {
            /* Look for an unescaped internal quote */
            if ( c == q && rchar(s,1) ) return 0; 
            else if ( c == '\\' && rchar(s,1) ) ++s;
        }
        node->code = q == '"' ? 'str' : 'chr';
    }
    else {
        int op = c;
        /* Maximum operator length is 3 (e.g. <<=). */
        if ( strlen(s) >= 4 ) return 0;

        /* Careful!  A one or two character operator might have garbage 
         * in the last byte(s) of its string. */
        if ( c = rchar(++s, 0) ) {
            lchar( &op, 1, c ); 
            if ( c = rchar(++s, 0) ) lchar( &op, 2, c );
        }

        /* Must allow generation of ## */
        if ( !is_op(op) && op != '#' && op != '##' ) 
            return 0;
        node->code = op;
    }

    return 1;
}

static
struct node *
do_concat(mace, args) 
    struct node *mace, *args;
{
    int i = 0;

    while (1) {
        struct node *expansion, *lhs, *lhs_val, *rhs, *rhs_val, *conc;
        int j, k;
        int no_lhs, no_rhs;

         for ( ; i < mace->arity; ++i )
            if ( mace->ops[i]->code == '##' )
                break;

        if ( i == mace->arity ) 
            return mace;

        if ( i == 0 || i == mace->arity-1 )
            int_error("Incorrectly positioned ##");


        expansion = new_node('mace', 0);
        lhs = lhs_val = mace->ops[i-1];
        rhs = rhs_val = mace->ops[i+1];

         /* Copy preceeding nodes */
        expansion = vnode_copy( expansion, mace, 0, i-1 );
    
        /* If they are parameter expansions, find the last and/or first token
         * of the expansion proper (i.e. excluding any _Pragma cpp_masks); 
         * or leave LHS_VAL null as "a placemarker preprocessing token" if 
         * the expansion is empty. */
        if (lhs->code == 'macp' && args) {
            struct node* arg = macp_arg(lhs, args);
            lhs_val = 0;  /* This is a placemarker */
    
            for (j = arg->arity; j; --j) {
                struct node* n = arg->ops[j-1];
                if (!is_cpp_prgm(n)) { lhs_val = n; break; }
            }
    
            /* Expand the parameter without doing macro expansion, leaving the
             * the final token (at index j) for the concatenation. */
            expansion = vnode_copy( expansion, arg, 0, j-1 );
            expansion = vnode_copy( expansion, arg, j, -1 );
        }
    
        /* Repeat for the RHS, but don't yet copy the unused tokens. */
        if (rhs->code == 'macp' && args) {
            struct node* arg = macp_arg(rhs, args);
            rhs_val = 0;
    
            for (j = 0; j < arg->arity; ++j) {
                struct node* n = arg->ops[j];
                if (!is_cpp_prgm(n)) { rhs_val = n; break; }
            }
        }
    
        /* Handle the cases when we're just concatening zero or one token.
         * Placemarker ('plmk') nodes are used to represent the empty nodes. */
        no_lhs = (!lhs_val || lhs_val->code == 'plmk');
        no_rhs = (!rhs_val || rhs_val->code == 'plmk');
       
        if (no_lhs && no_rhs) conc = new_node('plmk', 0);
        else if (no_lhs) conc = add_ref(rhs_val); 
        else if (no_rhs) conc = add_ref(lhs_val);
    
        /* Do the actual concatenation */
        else {
            int l = 0;
            conc = new_node('str', 0);
            node_concat( &conc, &l, lhs_val );
            node_concat( &conc, &l, rhs_val );
            node_lchar( &conc, &l, 0 );  /* NUL termination */
   
            /* Set its type */
            if ( !check_pptok( conc ) )
                error( "Concatention results in invalid token: %s",
                       node_str(conc) );
        }
    
        expansion = vnode_app( expansion, conc );
    
        /* Expand the RHS parameter without doing macro expansion, leaving the
         * the first token (at index j) for the concatenation. */
        if (rhs->code == 'macp' && args) {
            struct node* arg = macp_arg(rhs, args);
            expansion = vnode_copy( expansion, arg, 0, j );
            expansion = vnode_copy( expansion, arg, j+1, -1 );
        }
  
        /* Start here looking for the next ## */ 
        i = expansion->arity;

        /* Copy trailing nodes */
        expansion = vnode_copy( expansion, mace, i+2, -1 );
    
        free_node(mace);
        mace = expansion;
    }
}

struct vnode_iterator {
    struct node *vnode;
    int pos;
};

static vnode_next(iterator)
    struct vnode_iterator *iterator;
{
    if (iterator->pos < iterator->vnode->arity) {
       struct node *n = add_ref( iterator->vnode->ops[ iterator->pos ] ); 
       ++iterator->pos;
       return n;
    }
    else return 0;
}

/* Expand ARG and append it to VNODE 
 *
 * This called during macro expansion.  While copying the replacement list
 * to the output (in VNODE) we've encountered a macro parameter (denoted
 * by a 'macp' node.  We've looked up the parameter and found the 
 * corresponding argument (a 'mace' passed in ARG).  Macro expansion hasn't 
 * yet been done on the argument, so we do that here. 
 * 
 * We are not being passed ownership of ARG, but will typically copy elements
 * from ARG to VNODE. */
static
struct node *
expand_arg(vnode, arg)
    struct node *vnode, *arg;
{
    extern struct node *do_expand();

    if (arg->code != 'mace') 
        int_error("Macro argument expansion attempted on node type '%Mc'\n",
                  arg->code);

    int i;
    for ( i = 0; i < arg->arity; ++i ) {
        struct node *node = arg->ops[i];

        if ( node->code == 'id' ) {
            /* This may increment i if a macro is found. */
            struct node *expansion;
            struct vnode_iterator it;
            it.vnode = arg; it.pos = i+1;

            /* Pass free_node as the unget_fn because vnode_next simply does
             * an add_ref, and if do_expand only calls unget_fn if it fails
             * (i.e. returns 0).  In that case we ignore it.pos, so all we
             * need to do is decrement the ref count again. */
            expansion = do_expand( add_ref(node), vnode_next, &it, free_node);

            if (expansion) {
                /* Only preserve the increment of i if it succeeded. 
                 * The for loop is about to call ++i. */
                i = it.pos - 1;
                /* Re-scan for further macros  */
                vnode = expand_arg( vnode, expansion );
                free_node(expansion);
                continue;
            }
            /* We've called add_ref(), above, but do_expand didn't take it. */
            else free_node(node);
        }
        
        /* Process any _Pragma cpp_mask macros to handle masking */        
        if ( node->code == 'prgm' ) cpp_pragma(node);
        vnode = vnode_app( vnode, add_ref(node) );
    }
    return vnode;
}

/* Substitute any remaining parameters in MACE using ARGS */
static
struct node *
do_subst(mace, args)
    struct node *mace, *args;
{
    struct node *expansion = new_node('mace', 0);
    int i, j;

    for ( i = 0; i < mace->arity; ++i ) {
        struct node *node = mace->ops[i];

        /* Carry out argument expansion on any remaining parameters */
        if ( node->code == 'macp' )
            expansion = expand_arg( expansion, macp_arg( node, args ) );

        /* Placemarker nodes now get dropped */
        else if ( node->code == 'plmk' )
            ;

        /* Pass through other tokens. */
        else expansion = vnode_app( expansion, add_ref(node) );
    }

    free_node(mace);
    return expansion;
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
        /* if ( (*i)->ops[3] ) 
            int_error( "Macro '%s' left masked", node_str( (*i)->ops[0] ) );*/
        free_node(*i);
    }
}

/* Parse a -D command-line option */
parse_d_opt(arg) 
    char* arg;
{
    extern struct node* next();

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

/* Fix ARG to be a valid macro argument, and return its new value. */
static
struct node *
fix_arg(arg)
    struct node *arg;
{
    /* Currently the only purpose of this function is to handle unbalanced
     * _Pragma("RBC cpp_mask") type pragmas.  These can arise in code like
     * 
     *     #define f(a) f(2 * (a))
     *     #define g f
     *     #define h g(~
     *     h 5);
     *
     * ... when f is expanded, its argument is 
     *
     *     ~ _Pragma("RBC cpp_unmask h") 5
     *
     * We detect such cases and prepend the necessary cpp_mask
     * or append the necessary cpp_unmask to balance it out. */
    int i, j;

  restart:
    for (i=0; i<arg->arity; ++i) {
        struct node *n = arg->ops[i];
        if ( !is_rbc_prgm(n) ) continue;

        else if ( node_streq( n->ops[1], "cpp_unmask" ) ) {
            char *name = node_str( n->ops[2] );
            for (j=0; j<i; ++j) {
                struct node *m = arg->ops[j];
                if ( is_rbc_prgm(m) && node_streq( m->ops[1], "cpp_mask" )
                     && node_streq( m->ops[2], name ) )
                    goto cont;
            }
            arg = vnode_prep( arg, mk_prgm_rbc( n->ops[2], "cpp_mask" ) );
            goto restart;
        }

        else if ( node_streq( n->ops[1], "cpp_mask" ) ) {
            char *name = node_str( n->ops[2] );
            for (j=i+1; j<arg->arity; ++j) {
                struct node *m = arg->ops[j];
                if ( is_rbc_prgm(m) && node_streq( m->ops[1], "cpp_unmask" )
                     && node_streq( m->ops[2], name ) )
                    goto cont;
            }
            arg = vnode_app( arg, mk_prgm_rbc( n->ops[2], "cpp_unmask" ) );
            goto restart;
        }
      cont:;
    }

    return arg;
}

static
struct node *
macro_args(name_node, next_fn, next_fn_arg, unget_fn)
    struct node *name_node;
    struct node *(*next_fn)();
    *(*unget_fn)();
{
    struct node *masks = new_node('mace',0);
    struct node *macro = get_macro( node_str(name_node) );
    struct node *n, *args, *arg = 0;
    int pdepth = 0, n_params, n_args, i;

    /* We take ownership of NAME_NODE unless we return 0. */
    
    /* Skip any _Pragma("RBC cpp_mask $n") directive. */
    while ( is_cpp_prgm(n = next_fn(next_fn_arg)) )
        masks = vnode_app( masks, add_ref(n) );

    if ( !n || n->code != '(' ) { 
        /* Restore the pragmas: decrement arity to prevent freeing */
        while ( masks->arity )
            unget_fn( masks->ops[ --masks->arity ] );
  
        /* Give ownership of NAME_NODE back to the stack. */
        /* free_node(name_node); */

        free_node(masks);
        return 0;
    }

    /* This is definitely a function-like macro invocation */

    /* Process the pragmas */
    for ( i=0; i<masks->arity; ++i )
        cpp_pragma( masks->ops[i] );

    /* This is where we finally skip the '(' */
    free_node(n);
    free_node(masks);

    n = next_fn(next_fn_arg); 

    /* Give ownership of the NAME_NODE to ARGS which we eventually return. */
    args = vnode_app( new_node('()', 0), name_node );

    while (1) {
        /* We don't know whether this is an EOF or end of line: that 
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

        /* pdepth tracks the depth of parentheses: commas are only 
         * separators when outside parentheses. */
        if ( n->code == ',' && pdepth == 0 ) {
            arg = fix_arg(arg);
            args = vnode_app( args, arg );
            arg = new_node('mace', 0);
            free_node(n);
        }

        /* Expansion cannot yet be done on macros in arguments because 
         * we don't know whether expansion is suppressed by a # or ##;
         * the argument might even be used twice: once with and once
         * without expansion. */
        else
            arg = vnode_app( arg, n );

        n = next_fn(next_fn_arg);
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

    if (arg) { 
        arg = fix_arg(arg);
        args = vnode_app( args, arg ); 
        ++n_args; 
    }

    if ( n_params != n_args )
        error( "Macro '%s' expects %d arguments but called with %d arguments",
               node_str(name_node), n_params, n_args );

    return check_node(args);
}

/* If this return non-NULL, it takes ownership of NODE. 
 * XXX I don't see how that can be true. */
static
struct node *
do_expand(node, next_fn, next_fn_arg, unget_fn)
    struct node *node;
    struct node *(*next_fn)();
    int *(*unget_fn)();
{
    char *name = node_str(node);
    struct node *macd = get_macro(name);
    struct node *expansion, *args;

    /* All builtins are object-like, currently, and as we're in control
     * of how they expand, we don't bother with cpp_mask and cpp_unmask. */
    if ( is_builtin(name) ) {
        expansion = vnode_app( new_node('mace', 0), do_builtin(name) );
        free_node(node);
        return check_node(expansion);
    }

    /* If there's no macro of this name, or if it is masked, return 0 to 
     * indicate no expansion has been done.  (NB: slot [3] is used as an
     * is-masked flag manipulated by _Pragma cpp_mask and cpp_unmask.) */
    if (!macd || macd->ops[3])
        return 0;

    if (!macd->ops[2]) int_error("Macro '%s' found with no expansion", name);

    expansion = new_node('mace', 0);

    /* Push a _Pragma("RBC cpp_mask $name") node to implement the standard 
     * behaviour that macros cannot recursively reference themselves.  This 
     * is used in #define errno errno.  */
    expansion = vnode_app( expansion, mk_prgm_rbc( macd->ops[0], "cpp_mask" ) );

    /* Is it an object-like macro?  (NB: slot [1] is the arguments.) */
    if ( ! macd->ops[1] ) {
        struct node *mace = add_ref(macd->ops[2]);

        mace = do_concat(mace, 0);

        expansion = vnode_copy( expansion, mace, 0, -1 );
        free_node(mace);
        free_node(node);
    }

    /* Function-like macros are only expanded if the next pp token is an open 
     * bracket.  This allows the (fn)(a, b) syntax to be used to suppress 
     * macro expansion of a macro that hides a function of the same name. */
    else if ( args = macro_args(node, next_fn, next_fn_arg, unget_fn) ) {
        struct node *mace = add_ref(macd->ops[2]);

        mace = do_stringfy(mace, args);
        mace = do_concat(mace, args);
        mace = do_subst(mace, args);

        expansion = vnode_copy( expansion, mace, 0, -1 );
        free_node(mace);
        free_node(args);
    }

    /* The remaining case is a function-like macro name not invoked as such. */
    else {
        /* If we get here, macro_args() returned 0, meaning it didn't take
         * ownership of NODE. */
        free_node(expansion);
        return 0;
    }

    /* Unmask the macro again. 
     * Note: NODE and NAME may both may point to deallocated memmory now.
     * but MACD will also have the same name in ops[0], its name slot. */
    expansion = vnode_app( expansion, 
                           mk_prgm_rbc( macd->ops[0], "cpp_unmask" ) );
    return check_node(expansion);
}

/* Called with the name as the current token.  If the current token is
 * macro which can be expanded. */
try_expand(node, next_fn)
    struct node *node;
    struct node *(*next_fn)();
{
    extern unget_token();

    /* This point gives us exclusive ownership of the token, without 
     * calling next(), as would happen if we called take_token().*/
    struct node *node = add_ref(get_node());
    set_token(0);

    struct node *res = do_expand(node, next_fn, 0, unget_token);

    if (res) {
        int i;

        /* This should never happen because even empty expansions have
         * _Pragma masks and unmasks in them. */
        if (res->arity == 0) int_error("Pushing back empty vnode");
    
        i = res->arity - 1;
        set_token( add_ref(res->ops[i]) ); 
    
        for ( ; i; --i ) {
            struct node *n = res->ops[i-1];

            /* We clone nodes, rather than using add_ref(), because the 
             * expression parser expectes to modify them. */
            if (n) unget_token( clone_node(n) );
        }
     
        free_node(res);
        return 1;
    }
    else {
        if (get_node()) 
            int_error("Stack not restored after failed macro expansion");
        set_token(node);
        return 0;
    }
}
