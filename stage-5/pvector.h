/* pvector.h  --  code to deal with vectors of pointers
 *  
 * Copyright (C) 2013 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */ 

#ifndef RBC5_PVECTOR_INCLUDED
#define RBC5_PVECTOR_INCLUDED

struct pvector {
    char **start, **end, **end_store;
};

struct pvector* pvec_new();
pvec_delete();
pvec_push();
char* pvec_pop();

#endif
