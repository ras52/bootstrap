/* This test checks that macros are not expanded if not followed by an
 * open parenthesis. */
#define f() 1234

(f)();
