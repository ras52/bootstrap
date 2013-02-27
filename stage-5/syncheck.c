/* syncheck.c  --  syntax checking beyond the grammar
 *
 * Copyright (C) 2013 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */

req_lvalue(node) {
    auto type = node[0], dummy;

    if ( type == 'num' || type == 'chr' || type == 'str' )
        error("Literal used as an lvalue");

    /* This will need reworking as the C standard considers arrays to 
     * be lvalues -- i.e. our concept of an lvalue is at odd's with std C's */
    else if ( type == 'id' ) {
        if ( !lookup_sym( &node[2], &dummy ) )
            error("Non-lvalue identifier '%s' where lvalue is required",
                  &node[2]);
    }

    else if ( type != '*' && type != '[]')
        error("Non-lvalue expression used as lvalue");
}

