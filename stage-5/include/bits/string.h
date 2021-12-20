/* <bits/struct_tm.h>  --  string manipulation functions
 *
 * Copyright (C) 2005, 2021 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */

#ifndef __RBC_BITS_STRING_INCLUDED
#define __RBC_BITS_STRING_INCLUDED

#if 0
/* C90 7.11.2:  Copying functions */
void*  memcpy( void* dest, void const* src, size_t n );
void*  memmove( void* deset, void const* src, size_t n );
int    strcpy( char* dest, char const* str );
int    strncpy( char* dest, char const* str, size_t n );

/* C90 7.11.3:  Concatenation functions */
char*  strcat( char* dest, char const* src )
char*  strncat( char* dest, char const* src, size_t n );

/* C90 7.11.4:  Comparison functions */
/* int    memcmp( void const* s1, void const* s2, size_t n ); */
int    strcmp( char const* s1, char const* s2 );
/* int    strcoll( char const* s1, char const* s2 ); */
int    strncmp( char const* s1, char const* s2, size_t n );
/* int    strxfrm( char const* s1, char const* s2, size_t n ); */

/* C90 7.11.5:  Search functions */
/* void*  memchr( void const* s, int c, size_t n ); */
char*  strchr( char const* s, int c );
size_t strcspn( char const* str, char const* chrs );
/* char*  strpbrk( char const* str, char const* chrs ); */
/* char*  strrchr( char const* str, int c ); */
size_t strspn( char const* str, char const* chrs );
/* size_t strstr( char const* str, char const* substr ); */
/* size_t strtok( char const* str, char const* chrs ); */

/* C90 7.11.6:  Miscellaneous functions */
void*  memset( void* s, int c, size_t n );
/* char*  strerror( int errnum ); */
size_t strlen( char const* s );

/* POSIX.1-2001 extensions */
char *strdup( const char *str );
/* TODO: memccpy, strtok_r */

/* POSIX.1-2008 extensions */
/* TODO: stpcpy, stpncpy, strndup, strsignal */
size_t strlen( char const* s, size_t n )

/* Random BSD extension */
char*  strlcat( char* dest, char const* src, size_t n )

#else

/* Temporary versions while we don't support prototypes. */
void* memcpy();
void* memmove();
char* strcat();
char* strncat();
char* strchr();
void* memset();
char* strdup();
char* strlcat();

#endif

#endif 
