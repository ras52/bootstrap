/* malloc.c
 *
 * Copyright (C) 2013 Richard Smith <richard@ex-parrot.com> 
 * All rights reserved.
 */


/* struct header { 
 *   size_t         size;
 *   bool           is_free;
 *   struct header* next;
 *   struct header* prev;
 *   void*          page_start;     // effectively a page id
 * }; */

static
__heap = 0;

static
__find_blk( last, size ) {
    /* Look for a block of at least SIZE bytes in the list at LAST */
    while ( last && !( last[1] && last[0] >= size ) )
        last = last[2];
    return last;
}

static
__new_blk( size ) {
    /* Allocate a new block for at least SIZE bytes and prepend to __heap */
    auto blksz = size > 0x0FEC ? size : 0x0FEC;  /* 0x0FEC == 0x1000 - 20 */
    auto p = mmap(0, blksz + 20, 0x3, 0x22, -1, 0);
    p[0] = blksz;
    p[1] = 1;
    p[2] = __heap;
    p[3] = 0;
    p[4] = p;
    if (__heap) __heap[3] = p;
    __heap = p;
    return p;
}

static
__frag( blk, size ) {
    /* If the block BLK is significantly bigger than SIZE bytes, then fragment
     * it into a block of exactly SIZE bytes and a second block for the rest */
    if ( blk[0] >= size + 20 + 4 ) {
        auto b2 = blk + size + 20;
        b2[0] = blk[0] - size - 20;
        b2[1] = 1;
        b2[2] = blk[2];
        b2[3] = blk;
        b2[4] = blk[4];
        if (b2[2]) b2[2][3] = b2;
        blk[0] = size;
        blk[2] = b2;
    }
}

static
__defrag2( blk, b2 ) {
    /* The block at BLK and the next block, which is at B2, are both empty
     * so coalesce them into a single block. */
    blk[0] += b2[0] + 20;
    blk[2] = b2[2];
    if (blk[2]) blk[2][3] = blk;
}

static
__defrag( blk ) {
    /* See whether the block at BLK can be coalesced with either neighbour */
    if ( blk[2] && blk[4] == blk[2][4] && blk[2][1] ) __defrag2( blk, blk[2] );
    if ( blk[3] && blk[4] == blk[3][4] && blk[3][1] ) __defrag2( blk[3], blk );
}

/* The C library malloc() */
malloc( size ) {
    auto p = __find_blk( __heap, size );
    if (!p) p  = __new_blk( size );
    __frag( p, size );
    p[1] = 0;
    return p + 20;
}

/* The C library free() */
free( ptr ) {
    if (ptr) {
        auto p = ptr - 20;
        if (p[1]) _error(); /* Double free */
        p[1] = 1;
        __defrag( p );
    }
}

/* The C library realloc() */
realloc( ptr, size ) {
    auto h = ptr - 20;
    if ( h[2] && h[4] == h[2][4] && h[2][1] && h[0] + h[2][0] + 20 >= size ) {
        __defrag2( h, h[2] );
        __frag( h, size );
        return ptr;
    }
    else {
        auto p = malloc(size);
        memcpy( p, ptr, h[0] );
        free( ptr );
        return p;
    }
}

/* The C library strdup() */
strdup( str ) {
    auto l = strlen(str);
    auto str2 = malloc(l + 1);
    strcpy( str2, str );
    str2[l] = 0;
    return str2;
}
