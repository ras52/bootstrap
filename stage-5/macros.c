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

/* Handle a #define directive. */
defn_direct(stream) {
    struct node *macd = new_node('macd', 3);
    char *name;

    int c = skip_hwhite(stream);

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

static
clone_node(src)
    struct node *src;
{
    if (!src) 
        return src;

    else if ( src->code == 'str' || src->code == 'chr' ||
              src->code == 'ppno' || src->code == 'id' )
        return new_strnode( src->code, node_str(src) );

    else if ( src->arity <= 4 ) {
        struct node *dest = new_node( src->code, src->arity );
        /* Shallow copy of fields that shouldn't exist to catch where
         * we've abused the fields in question. */
        int n;
        for ( n = src->arity; n < 4; ++n )
            set_op( dest, n, node_op(src, n) );
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
do_expand2(name)
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

    if ( macd->ops[1] )
        error("Expansion of function-like macros is not supported");

    /* By pushing the tokens back in this way, we ensure that we rescan 
     * the expanded token sequence.  We clone them, rather than using 
     * add_ref(), because the expression parser expectes to modify them. */
    mace = macd->ops[2];
    for ( n = mace->arity; n; --n ) 
        unget_token( clone_node( mace->ops[n-1] ) );
}

do_expand(name)
    char *name;
{
    if (is_builtin(name))
        return do_builtin(name);
    else 
        return do_expand2(name);
}

/* Implements the defined() preprocessor function. */
pp_defined(name)
    char* name;
{
    return is_builtin(name) || get_macro(name) ? 1 : 0;
}

can_expand(name)
    char* name;
{
    struct node *macro = get_macro(name);
    /* Slot [3] is used as an is-masked flag. */
    return macro && !macro->ops[3] || is_builtin(name);
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
