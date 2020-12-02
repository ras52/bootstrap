/* Example from C99 6.10.2/8 */
#define VERSION 2
#if VERSION == 1
	#define INCFILE "vers1.h"
#elif VERSION == 2
	#define INCFILE "vers2.h"
#else
	#define INCFILE "versN.h"
#endif
#include INCFILE
