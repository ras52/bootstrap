/* main.c  --  code to tokenising C input stream
 *
 * Copyright (C) 2013 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */

main() {
    auto c;
    while ( (c = next()) != -1 ) {
        auto t = token;
        if (t == 'id' || t == 'num' || t == 'str' || t == 'chr') 
            printf( "%c: %s\n", token, value );
        else
            printf( "%c\n", c );
    }
    return 0;
}
