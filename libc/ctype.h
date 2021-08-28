// <ctype.h> GCC header simplified

#ifndef _CTYPE_H
#define _CTYPE_H 1

// These are macro functions, which we don't support
/*extern int isalnum(int __c);
extern int isalpha(int __c);
extern int iscntrl(int __c);
extern int isdigit(int __c);
extern int islower(int __c);
extern int isgraph(int __c);
extern int isprint(int __c);
extern int ispunct(int __c);
extern int isspace(int __c);
extern int isupper(int __c);
extern int isxdigit(int __c);*/

extern int tolower(int __c);

extern int toupper(int __c);

#endif
