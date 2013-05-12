/* node.c  --  code for manipulating AST nodes
 *
 * Copyright (C) 2013 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */

static
rc_count = 0;

/* Allocate SZ bytes of memory, adding a reference-counted header. */
static
rc_alloc(sz) {
    auto ptr = malloc(8 + sz);
    ++rc_count;
    ptr[0] = 1;  /* the reference count */
    ptr[1] = sz; /* the capacity */
    return &ptr[2];
}

/* Unconditionally unallocate PTR which is memory allocated by rc_alloc. */
static
rc_free(ptr) {
    --rc_count;
    free( &ptr[-2] );
}

/* Diagnostic routine to check that all nodes have been unallocated. */
rc_done() {
    if (rc_count) {
        extern stderr;
        fprintf(stderr, "Internal error: program leaked %d objects\n",
                rc_count);
    }
}

/* Wrapper around realloc to work with pointers returned by rc_alloc. */
static
rc_realloc(old_ptr, sz) {
    auto new_ptr;
    old_ptr = &old_ptr[-2];

    /* We cannot currently handle reallocating if there are multiple copies.  
     * What should it do?  If the address changes, we need to update all
     * the references, but we cannot do that.  So we'd have to create a unique
     * clone first, but then it's not really shared.  Best to prohibit it. */
    if ( *old_ptr > 1 )
        abort();

    new_ptr = realloc( old_ptr, sz + 8 );
    new_ptr[1] = sz;
    return &new_ptr[2];
}

/* Increment the reference count on a pointer */
add_ref(ptr) {
    ++ptr[-2];
    return ptr;
}

/* Allocate a new node of type TYPE. */
new_node(type) {
    /* struct node { int type; int nops; node* type; node* op[4]; } 
     *
     * For binary operators, op[0] is the lhs and op[1] the rhs; for unary 
     * prefix operators, only op[0] is used; and for unary postfix only 
     * op[1] is used. 
     * 
     * The scanner never reads a ternary operator (because ?: has two separate
     * lexical elements), but we generate '?:' nodes in the expression parser
     * and want a uniform interface.  Similarly, the 'for' node is a quaternary
     * "operator" (init, test, incr, stmt). */
    auto n = rc_alloc(28);

    n[0] = type; n[1] = 0;

    /* The type and payload (operands) will get filled in by the parser */
    memset( &n[2], 0, 20 );

    return n;
}

/* Unallocate a node, NODE, created by new_node(). */
free_node(node) {
    auto i = 0;

    if (!node) return;
    /* Trap for double delete */ 
    else if (node[-2] == 0) abort();
    /* If the reference count doesn't drop to zero, do nothing. */
    else if (--node[-2]) return;

    free_node( node[2] );

    while ( i < node[1] ) 
        free_node( node[ 3 + i++ ] );

    rc_free(node);
}

/* If SIZE is equal to the capacity of NODE, then reallocate it with twice
 * capacity, and return the new node. */
grow_node(node, size) {
    /* 12 is the size of the node before the payload. */

    if ( size + 12 == node[-1] ) {
        size *= 2;
        return rc_realloc( node, size + 12 );
    }

    return node;
}

/* Append node N to the vector node V, growing the vector if necessary, 
 * and returning the (possibly reallocated) vector. */
vnode_app( v, n ) {
    v = grow_node(v, v[1] * 4);
    v[ 3 + v[1]++ ] = n;
    return v;
}


