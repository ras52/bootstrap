/* cc.c  --  the C compiler driver
 *  
 * Copyright (C) 2013 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */ 

/* The Makefile sticks --compat on the command line.  Remove it. */
#pragma RBC compatibility 5 

#include "pvector.h"

static struct pvector *temps, *pp_args, *cc_args, *as_args, *ld_args;
static int last_stage = 4;  /* -E = 1, -S = -2, -c = -3 */
static char* o_name = 0;    /* The -o option, if any is given. */
static int nostdlib = 0;    /* --nostdlib */

static
usage() {
    cli_error("Usage: cc {-E | -S | -c} [-o output] [options] files...\n");
}

extern char* opt_arg();

parse_args(argc, argv)
    int argc;
    char **argv;
{
    int i = 0;

    while ( ++i < argc ) {
        char *arg = argv[i], *arg2;

        if ( arg2 = opt_arg( argv, argc, &i, "-I" ) ) {
            pvec_push( pp_args, "-I" );
            pvec_push( pp_args, arg2 );
        }

        else if ( arg2 = opt_arg( argv, argc, &i, "-D" ) ) {
            pvec_push( pp_args, "-D" );
            pvec_push( pp_args, arg2 );
        }

        else if ( strcmp( arg, "-E" ) == 0 ) {
            if ( last_stage != 4 )
                cli_error("At most one of -E, -S and -c may be used");
            last_stage = 1;
        }

        else if ( strcmp( arg, "-S" ) == 0 ) {
            if ( last_stage != 4 )
                cli_error("At most one of -E, -S and -c may be used");
            last_stage = 2;
        }

        else if ( strcmp( arg, "-c" ) == 0 ) {
            if ( last_stage != 4 )
                cli_error("At most one of -E, -S and -c may be used");
            last_stage = 3;
        }

        else if ( arg2 = opt_arg( argv, argc, &i, "-o" ) ) {
            if ( o_name ) cli_error(
                "Multiple output files specified: '%s' and '%s'\n",
                o_name, arg2 );
            o_name = arg2;
        }

        else if ( strcmp( arg, "--compat" ) == 0 )
            pvec_push( cc_args, arg );

        else if ( strcmp( arg, "--nostdlib" ) == 0 )
            nostdlib = 1;

        else if ( strcmp( arg, "--help" ) == 0 )
            usage();
   }
}

/* Invoke the command in ARGS.  Return 0 for success or 1 for failure. */
invoke(args) 
    struct pvector* args;
{
    int pid, status;
    pvec_push( args, 0 );
    if ( ( pid = fork() ) == 0 )
        execve( args->start[0], args->start, 0 );
    else if ( pid == -1 ) {
        extern stderr;
        fprintf(stderr, "cc: Unable to invoke %s", args->start[0]);
        exit(1);
    }
    pvec_pop( args );

    waitpid( pid, &status, 0 );
    
    /* WTERMSIG(status) || WEXITSTATUS(status) */
    return (status & 0xff7f) ? 1 : 0;
}

preprocess(argc, argv)
    int argc;
    char **argv;
{
    int i = 0, fail = 0;
    extern char* strdup();

    while ( ++i < argc && !fail ) {
        char *arg = argv[i];
        int l = strlen(arg);
        if ( arg[0] != '-' && l > 2 && arg[l-1] == 'c' && arg[l-2] == '.' ) {
            char *oname;
            if ( o_name && last_stage == 1 ) oname = strdup(o_name);
            else { oname = strdup(arg); oname[l-1] = 'i'; }

            pvec_push( pp_args, "-o" );
            pvec_push( pp_args, oname );
            pvec_push( pp_args, arg );
            fail = invoke( pp_args );
            pvec_pop( pp_args );
            pvec_pop( pp_args );
            pvec_pop( pp_args );

            if ( last_stage != 1 ) pvec_push( temps, oname );
            else free( oname );
        }
    } 

    return fail;
}

compile(argc, argv)
    int argc;
    char **argv;
{
    int i = 0, fail = 0;
    extern char* strdup();

    while ( ++i < argc && !fail ) {
        char *arg = argv[i];
        int l = strlen(arg);
        if ( arg[0] != '-' && l > 2 && (arg[l-1] == 'c' || arg[l-1] == 'i') 
               && arg[l-2] == '.' ) {
            char *iname = strdup(arg);
            iname[l-1] = 'i';

            if ( o_name && last_stage == 2 ) {
                pvec_push( cc_args, "-o" );
                pvec_push( cc_args, o_name );
            }

            pvec_push( cc_args, iname );
            fail = invoke( cc_args );
            pvec_pop( cc_args );

            if ( o_name && last_stage == 2 ) {
                pvec_pop( cc_args );
                pvec_pop( cc_args );
            }
            else if ( last_stage != 2 ) {
                char *oname = strdup(arg);
                oname[l-1] = 's';
                pvec_push( temps, oname );
            }

            free( iname );
        }
    } 

    return fail;
}

assemble(argc, argv)
    int argc;
    char **argv;
{
    int i = 0, fail = 0;
    extern char* strdup();

    while ( ++i < argc && !fail ) {
        char *arg = argv[i];
        int l = strlen(arg);
        if ( arg[0] != '-' && l > 2 && (arg[l-1] == 'c' || arg[l-1] == 'i' 
               || arg[l-1] == 's') && arg[l-2] == '.' ) {
            char *iname = strdup(arg);
            iname[l-1] = 's';

            if ( o_name && last_stage == 3 ) {
                /* The stage 3 assembler doesn't support -o, but we'll be 
                 * replacing that soon. */
                pvec_push( as_args, "-o" );
                pvec_push( as_args, o_name );
            }

            pvec_push( as_args, iname );
            fail = invoke( as_args );
            pvec_pop( as_args );

            if ( o_name && last_stage == 3 ) {
                pvec_pop( as_args );
                pvec_pop( as_args );
            }
            else if ( last_stage != 3 ) {
                char *oname = strdup(arg);
                oname[l-1] = 'o';
                pvec_push( temps, oname );
            }

            free( iname );
        }
    } 

    return fail;
}

link(argc, argv)
    int argc;
    char **argv;
{
    int i = 0, fail = 0;
    struct pvector* free_list = pvec_new();
    char** f;
    extern char* strdup();

    if ( o_name ) {
        pvec_push( ld_args, "-o" );
        pvec_push( ld_args, o_name );
    }

    if ( !nostdlib ) 
        pvec_push( ld_args, "crt0.o" );

    while ( ++i < argc ) {
        char *arg = argv[i];
        int l = strlen(arg);
        if ( arg[0] != '-' && l > 2 && (arg[l-1] == 'c' || arg[l-1] == 'i' 
               || arg[l-1] == 's' || arg[l-1]== 'o') && arg[l-2] == '.' ) {
            char *iname = strdup(arg);
            iname[l-1] = 'o';
            pvec_push( ld_args, iname );
            pvec_push( free_list, iname );
        }
    } 

    if ( !nostdlib ) 
        /* This should be -lc, but the stage 3 linker doesn't accept that. */
        pvec_push( ld_args, "libc.o" );

    fail = invoke( ld_args );
    for ( f = free_list->start; f != free_list->end; ++f ) free(*f);
    return fail;
}

main(argc, argv)
    int argc;
    char **argv;
{
    char** t;
    int res;

    temps = pvec_new();
    pp_args = pvec_new(); pvec_push( pp_args, "cpp" );
    cc_args = pvec_new(); pvec_push( cc_args, "ccx" ); 
    as_args = pvec_new(); pvec_push( as_args, "as" ); 
    ld_args = pvec_new(); pvec_push( ld_args, "ld" ); 

    parse_args( argc, argv );
    res = preprocess(argc, argv);
    if (!res && last_stage > 1) res = compile(argc, argv);
    if (!res && last_stage > 2) res = assemble(argc, argv);
    if (!res && last_stage > 3) res = link(argc, argv);

    pvec_delete(pp_args);
    pvec_delete(cc_args);
    pvec_delete(ld_args);
    pvec_delete(as_args);

    for ( t = temps->start; t != temps->end; ++t ) {
        unlink(*t);
        free(*t);        
    }
    pvec_delete(temps);

    return res;
}
