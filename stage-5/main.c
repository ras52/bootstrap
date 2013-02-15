/* main.c  --  code to tokenising C input stream
 *
 * Copyright (C) 2013 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */

print_node(stream, node) {
    if (!node)
        fputs("null", stream);

    else if ( is_op(node[0]) ) {
        fprintf(stream, "(%Mc", node[0]);
                       fputc(' ', stream); print_node(stream, node[1]); 
        if (node[2]) { fputc(' ', stream); print_node(stream, node[2]); }
        if (node[3]) { fputc(' ', stream); print_node(stream, node[3]); }
        if (node[4]) { fputc(' ', stream); print_node(stream, node[4]); }
        fputc(')', stream);
    }

    else if ( is_stmt_op(node[0]) ) {
        fprintf(stream, "(%Mc\n", node[0]);
                       print_node(stream, node[1]); fputc('\n', stream);
        if (node[2]) { print_node(stream, node[2]); fputc('\n', stream); }
        if (node[3]) { print_node(stream, node[3]); fputc('\n', stream); }
        if (node[4]) { print_node(stream, node[4]); fputc('\n', stream); }
        fputs(")\n", stream);
    }

    else if ( node[0] == '()' ) {
        auto i = 0;
        fputs("(call ", stream);
        print_node(stream, node[1]);
        while ( i < node[2] ) {
            fputc(' ', stream);
            print_node( stream, node[ 3 + i++ ] );
        }
        fputc(')', stream);
    }

    else if ( node[0] == 'id' || node[0] == 'str' || node[0] == 'chr' )
        fprintf( stream, "(%Mc %s)", node[0], &node[1] );

    else if ( node[0] == 'num' )
        fprintf( stream, "(%Mc %d)", node[0], node[1] );
    
    else if ( node[0] == '{}' ) {
        auto i = 0;
        fputs("(block\n", stream);
        while ( i < node[2] ) {
            print_node( stream, node[ 3 + i++ ] );
            fputc( '\n', stream );
        }
        fputs(")\n", stream);
    }

    else
        fputs( "(unknown)", stream );
}

main() {
    auto node;
    init_symtab();
    next();
    
    while ( token && (node = block()) ) {
        /* print_node( stderr, node ); */
        codegen( stdout, node, 0 );
        free_node( node );
    }
    return 0;
}
