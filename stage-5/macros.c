/* macros.c  --  support for preprocessor macros
 *
 * Copyright (C) 2013 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */

struct node* get_word();
struct node* next();
struct node* new_node();
struct node* vnode_app();
char* node_str();

/* A macro definition is just a 'macd' node with operands: 
 * [0] unused, [1] name, [2] unused (reserved for params), [3] expansion
 * and expansion is a 'mace' vnode all of whose operands are tokens
 * in the expansion.  The node struct is documented in nodenew.c.
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
        if ( strcmp( node_str( (*i)->ops[1] ), name ) == 0 )
            return *i;
    return 0;
}

/* Handle a #define directive. */
defn_direct(stream) {
    struct node* macd = new_node('macd', 4);

    int c = skip_hwhite(stream);

    if ( !isidchar1(c) )
        error("Expected identifier in #define directive");

    macd->ops[1] = get_word( stream, c );
    macd->ops[3] = new_node('mace', 0);

    while (1) {
        /* We need to explicitly call skip_hwhite() here because next()
         * will call skip_white(), and we musn't allow it to skip a '\n'. */
        ungetc( c = skip_hwhite(stream), stream );
        if ( c == '\n' ) break;
        macd->ops[3] = vnode_app( macd->ops[3], next() );
    }

    /* TODO:  We're allowed repeat definitions if they're identical. */
    if ( get_macro( node_str( macd->ops[1] ) ) )
        error("Duplicate definition of %s", node_str( macd->ops[1] ) );

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

    for (; i != e; ++i )
        if ( strcmp( node_str( (*i)->ops[1] ), name ) == 0 ) {
            /* Remove defn from the macro_vec */
            free_node(*i);  *i = 0;
            memmove( i, i+1, (e-i-1) * sizeof(struct node*) );
            --macro_vec.end;
            break;
        }

    free_node(tok);
}

do_expand(macd)
    struct node* macd;
{
    struct node* mace = macd->ops[3];
    int n;
    /* By pushing the tokens back in this way, we ensure that we rescan 
     * the expanded token sequence.   TODO The rescan is not standard-
     * compliant, because we don't hide the current macro name, which we
     * should.  Perhaps we'll do with by pusing a _Pragma( cpp-hide, name )
     * and _Pragma( cpp-restore, name ).  */
    for ( n = mace->arity; n; --n ) 
        unget_token( add_ref( mace->ops[n-1] ) );
}

/* Implements the defined() preprocessor function. */
pp_defined(name)
    char* name;
{
    return get_macro(name) ? 1 : 0;
}

expand(name)
    char* name;
{
    struct node *macro = get_macro(name);
    if (macro) { do_expand(macro); return 1; }
    return 0;
}

fini_macros() {
    struct node **i = macro_vec.start, **e = macro_vec.end;
    for (; i != e; ++i )
        free_node(*i);
}
