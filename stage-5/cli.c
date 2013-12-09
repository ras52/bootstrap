/* cli.c  --  command line interface utils
 *
 * Copyright (C) 2013 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */

cli_error(fmt) 
    char *fmt;
{
    extern stderr;
    vfprintf(stderr, fmt, &fmt);
    exit(1);
}


