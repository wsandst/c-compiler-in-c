// <ctype.h> assert GCC header simplified
// As we don't have access to function macros,
// we need to implement the assert as a regular function.
// This is problematic, as definitions in a header file is very bad
// Because of this, we cannot link multiple C files which import assert

#ifndef _ASSERT_H

#define _ASSERT_H 1
#include <stdlib.h>
#include <stdio.h>

extern void __assert(const char* __assertion, const char* __file, int __line);

void assert(int expression) {
#ifndef NDEBUG
    if (!expression) {
        fprintf(
            stderr,
            "Assertion failed! This compiler has very primitive asserts and thus no further information can be provided.\n");
        exit(1);
    }
#endif
}
#endif
