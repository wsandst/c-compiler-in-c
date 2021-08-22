// <stdarg.h> GCC header simplified
// Variadic function related

#ifndef _STDARG_H
#ifndef _ANSI_STDARG_H_
#ifndef __need___va_list
#define _STDARG_H
#define _ANSI_STDARG_H_
#endif /* not __need___va_list */
#undef __need___va_list

/* Define __gnuc_va_list.  */

#ifndef __GNUC_VA_LIST
#define __GNUC_VA_LIST
//typedef __builtin_va_list __gnuc_va_list;
#endif

/* Define the standard macros for the user,
   if this invocation was from the user program.  */
#ifdef _STDARG_H

//#define va_start(v,l)	__builtin_va_start(v,l)
//#define va_end(v)	__builtin_va_end(v)
//#define va_arg(v,l)	__builtin_va_arg(v,l)
//#define va_copy(d,s)	__builtin_va_copy(d,s)

#endif
#endif

#endif /* not _STDARG_H */
