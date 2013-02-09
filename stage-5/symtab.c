/* symtab.c  --  code to manipulate the symbol table
 *
 * Copyright (C) 2013 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */

static symtab[3] = { 0, 0, 0 };  /* start, end, end_store */

static scope_id = 0;

/*  struct sym_entry {
 *      char sym[12];
 *      int  frame_off;     [3]
 *      int  scope_id;      [4]
 *      int  lval;          [5]
 *      int  size;          [6]
 *  };                              size = 28
 */

/* Initialise the symbol table */
init_symtab() {
    auto sz = 1 * 28;               /* sizeof(sym_entry) */
    auto p = malloc(sz);
    symtab[0] = symtab[1] = p;
    symtab[2] = p + sz;
}

/* Check whether the symbol table is full, and if so double its capacity. 
 * Returns the end pointer  */
static
grow_symtab() {
    if ( symtab[1] == symtab[2] ) {
        auto sz = symtab[1] - symtab[0];
        auto p = realloc( symtab[0], 2*sz );
        symtab[0] = p;
        symtab[1] = p + sz;
        symtab[2] = p + 2*sz;
    }
    return symtab[1];
}

/* Save the symbol */
save_sym( name, frame_off, lval, sym_size ) {
    auto e = grow_symtab();
    strcpy( e, name );
    e[3] = frame_off;
    e[4] = scope_id;
    e[5] = lval;
    e[6] = sym_size; 
    symtab[1] += 28;  /* sizeof(sym_entry) */
}

/* Called on parsing '{' or otherwise starting a new nested scope. */
new_scope() {
    ++scope_id;
}

/* Called on parsing '}' or similar to remove symbols from the table.
 * Returns number of bytes that need removing from the stack. */
end_scope() {
    auto e = symtab[0], sz = 0;
    while ( e < symtab[1] ) {
        if ( e[4] == scope_id ) break;
        e += 28;    /* sizeof(sym_entry) */
    }

    /* Anything at e or beyond is about to go out of scope.  
     * Add up the size. */
    if ( e < symtab[1] ) {
        auto f = e;
        while ( e < symtab[1] ) {
            sz += e[6];
            e += 28;
        }
        symtab[1] = f;
    }

    --scope_id;
    return sz;
}

/* Return the lvalue flag for the symbol NAME, or 1 if it is 
 * not defined (as we assume external symbols are lvalues).  
 * Also set *OFF_PTR to the symbol table offset of the symbol, or
 * 0 if it is not defined (as 0 is not a valid offset because 
 * 0(%ebp) is the calling frame's base pointer.) */
lookup_sym( name, off_ptr ) {
    auto e = symtab[0];
    while ( e < symtab[1] ) {
        if ( strcmp( e, name ) == 0 ) {
            *off_ptr = e[3];
            return e[5];
        }
        e += 28;    /* sizeof(sym_entry) */
    }

    /* Symbol not found */
    *off_ptr = 0;
    return 1;
}

