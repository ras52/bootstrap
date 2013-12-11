/* pvector.c  --  code to deal with vectors of pointers
 *  
 * Copyright (C) 2013 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */ 

/* The Makefile sticks --compat on the command line.  Remove it. */
#pragma RBC compatibility 5 

/* We would like to #include "pvector.h" here, but we don't so that it can
 * be used in the implementation of the preprocessor.  Instead, repeat the
 * definition here. */
struct pvector {
    char **start, **end, **end_store;
};

struct pvector*
pvec_new() {
    struct pvector* v = (struct pvector*) malloc( sizeof(struct pvector) );
    int cap = 8;
    v->start = v->end = (char**) malloc( sizeof(char*) * cap );
    *v->end = 0;  /* null termination */
    v->end_store = v->start + cap;
    return v;
}

pvec_delete(v) 
    struct pvector* v;
{
    if (v) {
        free( v->start );
        free( v );
    }
}

pvec_push(v, elt)
    struct pvector* v;
    char* elt;
{
    /* Overwrite the null termination: which means we're guaranteed to
     * have space at this point. */
    *v->end++ = elt;

    if (v->end == v->end_store) {
        /* We need to reallocate now to push the null terminator */
        int cap = v->end - v->start;
        v->start = (char**) realloc( v->start, sizeof(char*) * 2*cap );
        v->end = v->start + cap;
        v->end_store = v->start + 2*cap;
    }

    *v->end = 0;
}

char* 
pvec_pop(v) 
    struct pvector* v;
{
    char* last = *--v->end;
    *v->end = 0;
    return last;
}
