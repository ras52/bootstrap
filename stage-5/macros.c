/* macros.c  --  support for preprocessor macros
 *
 * Copyright (C) 2013, 2014 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */

struct node* get_word();
struct node* next();
struct node* new_node();
struct node* vnode_app();
char* node_str();

/* A macro definition is just a 'macd' node with 3 operands: 
 * [0] name, [1] unused (reserved for params), [2] expansion;
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

/* Handle a #define directive. */
defn_direct(stream) {
    struct node* macd = new_node('macd', 3);

    int c = skip_hwhite(stream);

    if ( !isidchar1(c) )
        error("Expected identifier in #define directive");

    macd->ops[0] = get_word( stream, c );

    /* TODO:  Require whitespace before the expansion list */
    macd->ops[2] = new_node('mace', 0);
    pp_slurp(stream, &macd->ops[2], 0);

    /* TODO:  We're allowed repeat definitions if they're identical. */
    if ( get_macro( node_str( macd->ops[0] ) ) )
        error("Duplicate definition of %s", node_str( macd->ops[0] ) );

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
mk_unmask(strnode)
    struct node *strnode;
{
    /* We overwrite the head node on the stack (which will be the macro being
     * expanded) with a _Pragma("RBC cpp_unmask $name") node. */
    struct node *node = new_node( 'prgm', 3 );
    node->ops[0] = new_strnode('id', "RBC");
    node->ops[1] = new_strnode('id', "cpp_unmask");
    node->ops[2] = add_ref(strnode);
    return node;
}

do_expand(name)
    char* name;
{
    int n;
    struct node *macd = get_macro(name), *mace;

    /* Mask the macro, and push a _Pragma("RBC cpp_unmask $name") node.
     * This implements the standard behaviour that macros cannot recursively
     * reference themselves.  This is used in #define errno errno.
     * The [3] slot is abused to store an is-masked flag.  As arity is 3,
     * it is never freed. */
    if ( !macd || macd->ops[3] ) 
        int_error("Attempt to expand undefined macro: %s", name);
    macd->ops[3] = (struct node*) 1;
    set_token( mk_unmask( macd->ops[0] ) ); 

    /* By pushing the tokens back in this way, we ensure that we rescan 
     * the expanded token sequence. */
    mace = macd->ops[2];
    for ( n = mace->arity; n; --n ) 
        unget_token( add_ref( mace->ops[n-1] ) );
}

/* Implements the defined() preprocessor function. */
pp_defined(name)
    char* name;
{
    return get_macro(name) ? 1 : 0;
}

can_expand(name)
    char* name;
{
    struct node *macro = get_macro(name);
    /* Slot [3] is used as an is-masked flag. */
    return macro && !macro->ops[3];
}

cpp_unmask(name) 
    char* name;
{
    struct node *macro = get_macro(name);
    if ( !macro )
        error("Unable to unmask non-existent macro '%s'", name);
    macro->ops[3] = 0;
}

fini_macros() {
    struct node **i = macro_vec.start, **e = macro_vec.end;
    for (; i != e; ++i )
        free_node(*i);
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
