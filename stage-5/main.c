/* main.c  --  code to tokenising C input stream
 *
 * Copyright (C) 2013 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */

main() {
    auto node;
    init_symtab();

    while ( node = next() ) {
        auto t = node[0];
        if (t == 'id' || t == 'str' || t == 'chr') 
            printf( "%Mc: %s\n", t, &node[1] );
        else if (t == 'num')
            printf( "%Mc: %d\n", t, node[1] );
        else
            printf( "%Mc\n", t );
        free(node);
    }
    return 0;
}
