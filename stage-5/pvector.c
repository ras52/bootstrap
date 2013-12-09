/* pvector.c  --  code to deal with vectors of pointers
 *  
 * Copyright (C) 2013 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */ 

/* The Makefile sticks --compat on the command line.  Remove it. */
#pragma RBC compatibility 5 

#include "pvector.h"

struct pvector*
pvec_new() {
    struct pvector* v = (struct pvector*) malloc( sizeof(struct pvector) );
    v->start = v->end = v->end_store = 0;
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
    if (v->end == v->end_store) {
        int cap = v->end - v->start;
        int new_cap = cap ? 2*cap : 8;
        v->start = (char**) realloc( v->start, sizeof(char*) * new_cap );
        v->end = v->start + cap;
        v->end_store = v->start + new_cap;
    }

    *v->end++ = elt;
}

pvec_pop(v) 
    struct pvector* v;
{
    --v->end;
}    
