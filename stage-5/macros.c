/* macros.c  --  support for preprocessor macros
 *
 * Copyright (C) 2013 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */

struct node* get_word();
struct node* next();
struct node* new_node();
struct node* vnode_app();

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

/* Handle a #define directive. */
defn_direct(stream) {
    struct node* macd = new_node('macd', 4);

    int c = skip_hwhite(stream);

    if ( !isidchar1(c) )
        error("Expected identifier in #define directive");

    macd->ops[1] = get_word( stream, c );
    macd->ops[3] = new_node('mace', 0);

    while (1) {
        ungetc( c = skip_hwhite(stream), stream );
        if ( c == -1 || c == '\n' ) break;
        macd->ops[3] = vnode_app( macd->ops[3], next() );
    }

    vec_grow(macd);
}

do_expand(macd)
    struct node* macd;
{
    struct node* mace = macd->ops[3];
    int n;
    for ( n = mace->arity; n; --n ) 
        unget_token( add_ref( mace->ops[n-1] ) );
}

expand(name) {
    struct node **i = macro_vec.start, **e = macro_vec.end;
    for (; i != e; ++i )
        if ( strcmp( node_str( (*i)->ops[1] ), name ) == 0 ) {
            do_expand(*i);
            return 1;
        }
    return 0;
}

fini_macros() {
    struct node **i = macro_vec.start, **e = macro_vec.end;
    for (; i != e; ++i )
        free_node(*i);
}
