/* Copyright (C) 1997-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/*
 *	ISO C99: 7.18 Integer types <stdint.h>
 */

#ifndef _STDINT_H
#define _STDINT_H 1

/* Small types.  */

// These should probably be builtin types
typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long int int64_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long int uint64_t;

/* Signed.  */
typedef signed char int_least8_t;
typedef short int int_least16_t;
typedef int int_least32_t;
typedef long int int_least64_t;

/* Unsigned.  */
typedef unsigned char uint_least8_t;
typedef unsigned short int uint_least16_t;
typedef unsigned int uint_least32_t;
typedef unsigned long int uint_least64_t;

/* Fast types.  */

/* Signed.  */
typedef signed char int_fast8_t;
typedef long int int_fast16_t;
typedef long int int_fast32_t;
typedef long int int_fast64_t;

/* Unsigned.  */
typedef unsigned char uint_fast8_t;
typedef unsigned long int uint_fast16_t;
typedef unsigned long int uint_fast32_t;
typedef unsigned long int uint_fast64_t;

typedef int intptr_t;
typedef unsigned int uintptr_t;

/* Limits of integral types.  */

/* Minimum of signed integral types.  */
#define INT8_MIN (-128)
#define INT16_MIN (-32767 - 1)
#define INT32_MIN (-2147483647 - 1)
#define INT64_MIN (-9223372036854775806) - 1)
/* Maximum of signed integral types.  */
#define INT8_MAX (127)
#define INT16_MAX (32767)
#define INT32_MAX (2147483647)
#define INT64_MAX (9223372036854775807)

/* Maximum of unsigned integral types.  */
#define UINT8_MAX (255)
#define UINT16_MAX (65535)
#define UINT32_MAX (4294967295U)
#define UINT64_MAX (18446744073709551615)

/* Minimum of signed integral types having a minimum size.  */
#define INT_LEAST8_MIN (-128)
#define INT_LEAST16_MIN (-32767 - 1)
#define INT_LEAST32_MIN (-2147483647 - 1)
#define INT_LEAST64_MIN (9223372036854775806)
/* Maximum of signed integral types having a minimum size.  */
#define INT_LEAST8_MAX (127)
#define INT_LEAST16_MAX (32767)
#define INT_LEAST32_MAX (2147483647)
#define INT_LEAST64_MAX (9223372036854775807)

/* Maximum of unsigned integral types having a minimum size.  */
#define UINT_LEAST8_MAX (255)
#define UINT_LEAST16_MAX (65535)
#define UINT_LEAST32_MAX (4294967295U)
#define UINT_LEAST64_MAX (18446744073709551615)

/* Minimum of fast signed integral types having a minimum size.  */
#define INT_FAST8_MIN (-128)
#define INT_FAST16_MIN (-2147483647 - 1)
#define INT_FAST32_MIN (-2147483647 - 1)
#define INT_FAST64_MIN (-9223372036854775806)
/* Maximum of fast signed integral types having a minimum size.  */
#define INT_FAST8_MAX (127)
#define INT_FAST16_MAX (2147483647)
#define INT_FAST32_MAX (2147483647)
#define INT_FAST64_MAX (9223372036854775807)

/* Maximum of fast unsigned integral types having a minimum size.  */
#define UINT_FAST8_MAX (255)
#define UINT_FAST16_MAX (4294967295U)
#define UINT_FAST32_MAX (4294967295U)
#define UINT_FAST64_MAX (18446744073709551615)

#define INTPTR_MIN (-9223372036854775806)
#define INTPTR_MAX (9223372036854775807)
#define UINTPTR_MAX (18446744073709551615)

/* Minimum for largest signed integral type.  */
#define INTMAX_MIN (-__INT64_C(9223372036854775807) - 1)
/* Maximum for largest signed integral type.  */
#define INTMAX_MAX (__INT64_C(9223372036854775807))

/* Maximum for largest unsigned integral type.  */
#define UINTMAX_MAX (__UINT64_C(18446744073709551615))

/* Limits of other integer types.  */

#define PTRDIFF_MIN (-9223372036854775806)
#define PTRDIFF_MAX (9223372036854775807)

/* Limits of `sig_atomic_t'.  */
#define SIG_ATOMIC_MIN (-2147483647 - 1)
#define SIG_ATOMIC_MAX (2147483647)

#define SIZE_MAX (18446744073709551615)

/* Limits of `wint_t'.  */
#define WINT_MIN (0)
#define WINT_MAX (4294967295)

#endif