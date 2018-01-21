#pragma GCC system_header
#ifndef UTIL_H_INCLUDED_
#define UTIL_H_INCLUDED_
#include <stdlib.h>
#include <stdio.h>

#ifdef __GNUC__
#define FATAL_ERROR(fmt, ...) {fprintf(stderr, "File: %s, line %d :" fmt \
                                       "\n", __FILE__, __LINE__, ##     \
                                       __VA_ARGS__);exit(-1);}
#define WARNING(fmt, ...) {fprintf(stderr, "File: %s, line %d :" fmt	\
				   "\n", __FILE__, __LINE__, ##		\
				   __VA_ARGS__);}
#else
#define FATAL_ERROR {fprintf(stderr, "File: %s, line %d : fatal error\n", \
                             __FILE__, __LINE__);exit(-1);}
#define WARNING {fprintf(stderr, "File: %s, line %d : fatal error\n",	\
			 __FILE__, __LINE__);}
#endif

#endif // #ifndef UTIL_H_INCLUDED_
