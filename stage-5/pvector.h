#ifndef RBC5_PVECTOR_INCLUDED
#define RBC5_PVECTOR_INCLUDED

struct pvector {
    char **start, **end, **end_store;
};

struct pvector* pvec_new();

#endif
