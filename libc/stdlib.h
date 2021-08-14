
// <stdlib.h> GCC libc header simplified 
// String conversions, rand, malloc

#ifndef	_STDLIB_H

#include <stddef.h>

#define	_STDLIB_H	1

/* The largest number rand will return (same as INT_MAX).  */
#define	RAND_MAX	2147483647


/* We define these the same for all machines.
   Changes from this to the outside world should be done in `_exit'.  */
#define	EXIT_FAILURE	1	/* Failing exit status.  */
#define	EXIT_SUCCESS	0	/* Successful exit status.  */

// String conversions

extern double atof (const char *__nptr);

extern int atoi (const char *__nptr);

extern long int atol (const char *__nptr);

extern long long int atoll (const char *__nptr);

extern double strtod (const char * __nptr,
		      char ** __endptr);

extern float strtof (const char * __nptr,
		     char ** __endptr);

extern long double strtold (const char * __nptr,
			    char ** __endptr);

extern long int strtol (const char * __nptr,
			char ** __endptr, int __base);

extern unsigned long int strtoul (const char * __nptr,
				  char ** __endptr, int __base);

extern long long int strtoll (const char * __nptr,
			      char ** __endptr, int __base);

extern unsigned long long int strtoull (const char * __nptr,
					char ** __endptr, int __base);

// Random numbers

extern int rand (void);

extern void srand (unsigned int __seed);


// Memory allocation

extern void *malloc (size_t __size);

extern void *calloc (size_t __nmemb, size_t __size);

extern void *realloc (void *__ptr, size_t __size);

extern void free (void *__ptr);

extern void exit (int __status);

extern int system (const char *__command);

extern char *getenv (const char *__name);

extern int abs (int __x);

extern long int labs (long int __x);

#endif