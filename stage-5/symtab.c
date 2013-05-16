/* symtab.c  --  code to manipulate the symbol table
 *
 * Copyright (C) 2013 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */

/* This table contains the namespace of ordinary identifiers */
static symtab[3] = { 0, 0, 0 };  /* start, end, end_store */
/*  struct sym_entry {
 *      char  sym[12];
 *      int   frame_off;     [3]     -- 0 is used for undefined symbols
 *      int   scope_id;      [4]
 *      node* decl;          [5]
 *  };                              size = 24 = 6*4
 */

/* This table contains the namespace of label names */
static labtab[3] = { 0, 0, 0 };  /* start, end, end_store */
/*  struct lab_entry {
 *      char sym[12];
 *      bool defined;       [3]
 *      int  label_num;     [4]
 *  };                              size = 20 = 5*4
 */ 

static scope_id = 0;

static frame_size = 0;

/* Initialise a table */
static
init_tab( tab, entry_size ) {
    auto sz = 1 * entry_size;
    auto p = malloc(sz);
    tab[0] = tab[1] = p;
    tab[2] = p + sz;
}

static
fini_tab( tab ) {
    free( tab[0] );
    tab[0] = tab[1] = tab[2] = 0;    
}

/* Check whether the symbol table is full, and if so double its capacity. 
 * Copied NAME into the entry, increment the table's end pointer by
 * ENTRY_SIZE, and return the entry pointer.   */
static
grow_tab( tab, entry_size, name ) {
    auto sz = tab[1] - tab[0];
    auto p = tab[0];
    if ( tab[1] == tab[2] ) {
        p = realloc( tab[0], 2*sz );
        tab[0] = p;
        tab[1] = p + sz;
        tab[2] = p + 2*sz;
    }
    strcpy( tab[1], name );
    tab[1] += entry_size;
    return p + sz;
}

/* Lookup NAME in TAB and return a pointer to the table entry, 
 * or NULL if not found. */
static
lookup_tab( tab, entry_size, name ) {
    auto e = tab[1];
    while (e > tab[0]) {
        e -= entry_size;
        if ( strcmp( e, name ) == 0 )
            return e;
    }
    return 0;
}

init_symtab() {
    init_tab( symtab, 24 );         /* sizeof(sym_entry) */
    init_tab( labtab, 20 );         /* sizeof(lab_entry) */
}

fini_symtab() {
    /* Calling end_scope destroys the type nodes in the symbol table. */
    end_scope();

    fini_tab( symtab );
    fini_tab( labtab );
}

/* Define a label, possibly implicitly in the case of a forwards goto. 
 * Return 1 if it is a new label. */
save_label( name, is_defn ) {
    auto *e = lookup_tab( labtab, 20, name ); /* sizeof(lab_entry) */
    auto rv = 1;
    if (e) {
        if ( is_defn ) {
            if (e[3]) error( "Multiple definition of label '%s'", name );
            e[3] = is_defn;
        }
        rv = 0;
    }
    else {
        e = grow_tab( labtab, 20, name ); /* sizeof(lab_entry) */
        e[3] = is_defn;
        e[4] = 0;
    }
    return rv;
}

/* Store (and return) the label number for an already-saved label. */
set_label( name, num ) {
    auto e = lookup_tab( labtab, 20, name ); /* sizeof(lab_entry) */
    if (!e) int_error("Trying to set an undefined label");
    if (e[4]) int_error("Trying to set an already set label");
    e[4] = num;
    return num;
}

/* Return the label number for an already-set label. */
get_label( name ) {
    auto e = lookup_tab( labtab, 20, name ); /* sizeof(lab_entry) */
    if (!e) int_error("Trying to set an undefined label");
    if (!e[4]) int_error("Trying to get an unset label");
    return e[4];
}

/* Define a symbol, possibly implicitly by calling a function */
save_sym( decl, frame_off ) {
    auto name = &decl[4][3];
    auto e = grow_tab( symtab, 24, name );   /* sizeof(sym_entry) */
    e[3] = frame_off;
    e[4] = scope_id;
    e[5] = add_ref(decl);
}

/* Called on parsing '{' or otherwise starting a new nested scope. */
static
new_scope() {
    ++scope_id;
}

/* Promote SIZE to the amount of space it takes up on the stack. */
static
promote_sz(size) {
    return (size + 3) & ~3;
}

/* Called on parsing '}' or similar to remove symbols from the table.
 * Returns number of bytes that need removing from the stack. */
static
end_scope() {
    auto e = symtab[0], sz = 0;
    while ( e < symtab[1] ) {
        if ( e[4] == scope_id ) break;
        e += 24;    /* sizeof(sym_entry) */
    }

    /* Anything at e or beyond is about to go out of scope.  
     * Add up the stack space used by automatic symbols. */
    if ( e < symtab[1] ) {
        auto f = e;
        while ( e < symtab[1] ) {
            /* The only symbols in this scope with an offset in e[3] 
             * will be automatic variables. */
            if (e[3]) sz += promote_sz( type_size( e[5][2] ) );
            free_node(e[5]);
            e += 24;    /* sizeof(sym_entry) */
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
    auto e = lookup_tab( symtab, 24, name );    /* sizeof(sym_entry) */
    if (e) {
        *off_ptr = e[3];
        return is_lv_decl(e[5][2]);
    }

    /* Symbol not found -- default to rvalues e.g. functions */
    *off_ptr = 0;
    return 0;
}

/* Check whether a symbol NAME has been declared */
is_declared( name ) {
    auto e = lookup_tab( symtab, 24, name );    /* sizeof(sym_entry) */
    return e ? 1 : 0;
}

/* Lookup the type node for a name */
lookup_type( name ) {
    auto e = lookup_tab( symtab, 24, name );    /* sizeof(sym_entry) */
    return e ? e[5][2] : 0;
}

/* Lookup the storage type for a name */
lookup_strg( name ) {
    auto e = lookup_tab( symtab, 24, name );    /* sizeof(sym_entry) */
    return e ? e[5][3] : 0;
}

/* Check whether another symbol called NAME exists in the current scope,
 * and if necessary give an error. */
chk_dup_sym( name, has_linkage ) {
    auto e = lookup_tab( symtab, 24, name );    /* sizeof(sym_entry) */
    if (e && e[4] == scope_id) {
        /* If both definitions are definitions with linkage (meaning extern
         * definitions) then we allow it.  Once we have types, we should 
         * check for compatibility before allowing it.  Otherwise, we issue
         * an error. */
        if ( !(has_linkage && e[3] == 0) )
            error("Multiple definitions of '%s' within the same block", name);
    }
}

/* Bytes of local variables on the stack. */
static cur_offset = 0;

frame_off() {
    return -cur_offset; 
}

static
is_lv_decl(type) {
    if ( type && type[0] == '[]' ) return 0;
    if ( type && type[0] == '()' ) return 0;
    else return 1;
}

decl_var(decl) {
    auto sz = type_size( decl[2] );
    cur_offset += promote_sz(sz);
    if ( decl[4][0] != 'id' )
        int_error("Expected identifier in auto decl");
    save_sym( decl, -cur_offset );
    if (cur_offset > frame_size)
        frame_size = cur_offset;
    return sz;
}

start_fn(decl) {
    auto i = 1, off = 8;
    cur_offset = frame_size = 0;
    new_scope();

    /* Inject the parameters into the scope */
    while ( i < decl[1] ) {
        auto n = decl[ 3 + i++ ], sz;
        if (n[0] != 'decl')
            int_error("Unexpected token found as function param");
        sz = type_size(n[2]);
        save_sym( n, off );
        off += promote_sz(sz);
    }
}

end_fn() {
    end_scope();

    /* Drop all labels */
    labtab[1] = labtab[0];

    return frame_size;
}

start_block() {
    new_scope();
}

end_block() {
    cur_offset -= end_scope();
}
