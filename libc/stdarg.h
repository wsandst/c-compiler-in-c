// <stdarg.h> GCC header simplified
// Variadic function related

#ifndef _STDARG_H
#define _STDARG_H
#define _ANSI_STDARG_H_

struct va_list {
    unsigned int gp_offset;
    unsigned int fp_offset;
    void* overflow_arg_area;
    void* reg_save_area;
};

typedef struct va_list va_list;

//#define va_start(v,l)	__builtin_va_start(v,l)
//#define va_end(v)	__builtin_va_end(v)
//#define va_arg(v,l)	__builtin_va_arg(v,l)
//#define va_copy(d,s)	__builtin_va_copy(d,s)

#endif
