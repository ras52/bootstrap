/* This is a slightly non-trivial test of rescanning the result of macro
 * expansion.  For a long time this case was causing a memory leak. */
#define f(a)    (0+a)
#define g       f
#define t(a)    a

t(g);
