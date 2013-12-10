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

char* opt_arg(argv, argc, argnptr, argname)
    char **argv;
    int argc, *argnptr;
    char *argname;
{
    auto char *arg = argv[*argnptr];
    auto int arglen = strlen(argname);
    if ( strncmp( arg, argname, arglen ) == 0 ) {
        if ( rchar( arg, arglen ) == 0 ) {
            if ( ++*argnptr == argc )
                cli_error("The %s option takes an argument\n", argname);
            arg = argv[*argnptr];
            ++*argnptr;
            return arg;
        }
        /* Short arguments (e.g. -X) do not have an '=' before their values. */
        else if ( arglen == 2 ) {
            arg += arglen;
            ++*argnptr;
            return arg;
        }
        /* Long arguments (e.g. --foo) need an '=' before their values. */
        else if ( rchar( arg, arglen ) == '=' ) {
            arg += arglen + 1;
            ++*argnptr;
            return arg;
        }
    }
    return 0;
}


