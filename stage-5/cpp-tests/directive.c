/* These are all null directives. */
#
 #
   #
	\
#
/*  Foo # include */ #

/* Example from C99 6.10/4.
 *
 * "the sequence of preprocessing tokens on the second line is not a
 * preprocessing directive, because it does not begin with a # at the
 * start of translation phase 4, even though it will do so after the
 * macro EMPTY has been replaced." */

#define EMPTY
EMPTY # include <missing.h>


