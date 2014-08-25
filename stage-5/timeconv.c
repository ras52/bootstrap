/* cc.c  --  functions to convert times
 *  
 * Copyright (C) 2014 Richard Smith <richard@ex-parrot.com>
 * All rights reserved.
 */ 

/* The Makefile sticks --compatibility=4 on the command line.  Remove it. */
#pragma RBC compatibility 5 

#include <bits/struct_tm.h>

static 
struct tm buf;

static
leap_year(year) {
    if (year % 4) return 0;
    else if (year % 100) return 1;
    else if (year % 400) return 0;
    else return 1;
}

/* The stage-5 compiler doesn't support multi-dimensional arrays */
static int month_lens[26] = { 
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, 0,
    31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, 0
};

/* The C standard gmtime */
struct tm *
gmtime(timep) 
    int *timep;
{
    int t = *timep;
    int ly = 0, year = 1970, month = 0, *lp;

    buf.tm_sec  = t % 60;  t /= 60;
    buf.tm_min  = t % 60;  t /= 60;
    buf.tm_hour = t % 24;  t /= 24;

    /* The Unix epoch, 1 Jan 1970, was a Thurday, day 4. */
    buf.tm_wday = (t - 4) % 7;

    while ( t < 0 )
        --year, ly = leap_year(year), t += 365 + ly;
    while ( !ly && t == 365 || t > 365 ) 
        ++year, t -= 365 + ly, ly = leap_year(year);

    buf.tm_year = year - 1900;
    buf.tm_yday = t;

    for ( lp = month_lens + ly*13; *lp && t >= *lp; month++, t -= *lp, ++lp )
        ;
    buf.tm_mon = month;
    buf.tm_mday = t + 1;

    buf.tm_isdst = 0;

    return &buf;
}


