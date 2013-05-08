/* signal.c  --  signal handling functions
 *
 * Copyright (C) 2013 Richard Smith <richard@ex-parrot.com> 
 * All rights reserved.
 */

/* The C library raise() */
raise( sig ) {
    return kill( getpid(), sig );
}

/* The C library abort() */
abort() {
    raise(6);   /* SIGABRT == 6 */

    /* If we're still here, reinstate the default handler and retry. */
    signal(6, 0);   /* SIG_DFL == 0 */
    raise(6);

    /* This shouldn't be possible. */
    _exit(128 + 6);
}
