// RUN: %clang_cc1 -E -dM -x assembler-with-cpp < /dev/null | FileCheck -check-prefix ASM %s
//
// ASM:#define __ASSEMBLER__ 1
//
// 
// RUN: %clang_cc1 -fblocks -E -dM < /dev/null | FileCheck -check-prefix BLOCKS %s
//
// BLOCKS:#define __BLOCKS__ 1
// BLOCKS:#define __block __attribute__((__blocks__(byref)))
//
//
// RUN: %clang_cc1 -x c++ -std=c++1y -E -dM < /dev/null | FileCheck -check-prefix CXX1Y %s
//
// CXX1Y:#define __GNUG__
// CXX1Y:#define __GXX_EXPERIMENTAL_CXX0X__ 1
// CXX1Y:#define __GXX_RTTI 1
// CXX1Y:#define __GXX_WEAK__ 1
// CXX1Y:#define __cplusplus 201402L
// CXX1Y:#define __private_extern__ extern
//
//
// RUN: %clang_cc1 -x c++ -std=c++11 -E -dM < /dev/null | FileCheck -check-prefix CXX11 %s
//
// CXX11:#define __GNUG__
// CXX11:#define __GXX_EXPERIMENTAL_CXX0X__ 1
// CXX11:#define __GXX_RTTI 1
// CXX11:#define __GXX_WEAK__ 1
// CXX11:#define __cplusplus 201103L
// CXX11:#define __private_extern__ extern
//
// 
// RUN: %clang_cc1 -x c++ -std=c++98 -E -dM < /dev/null | FileCheck -check-prefix CXX98 %s
// 
// CXX98:#define __GNUG__
// CXX98:#define __GXX_RTTI 1
// CXX98:#define __GXX_WEAK__ 1
// CXX98:#define __cplusplus 199711L
// CXX98:#define __private_extern__ extern
//
// 
// RUN: %clang_cc1 -fdeprecated-macro -E -dM < /dev/null | FileCheck -check-prefix DEPRECATED %s
//
// DEPRECATED:#define __DEPRECATED 1
//
// 
// RUN: %clang_cc1 -std=c99 -E -dM < /dev/null | FileCheck -check-prefix C99 %s
//
// C99:#define __STDC_VERSION__ 199901L
// C99:#define __STRICT_ANSI__ 1
//
// 
// RUN: %clang_cc1 -std=c11 -E -dM < /dev/null | FileCheck -check-prefix C11 %s
//
// C11:#define __STDC_UTF_16__ 1
// C11:#define __STDC_UTF_32__ 1
// C11:#define __STDC_VERSION__ 201112L
// C11:#define __STRICT_ANSI__ 1
//
// 
// RUN: %clang_cc1 -E -dM < /dev/null | FileCheck -check-prefix COMMON %s
//
// COMMON:#define __CONSTANT_CFSTRINGS__ 1
// COMMON:#define __FINITE_MATH_ONLY__ 0
// COMMON:#define __GNUC_MINOR__
// COMMON:#define __GNUC_PATCHLEVEL__
// COMMON:#define __GNUC_STDC_INLINE__ 1
// COMMON:#define __GNUC__
// COMMON:#define __GXX_ABI_VERSION
// COMMON:#define __ORDER_BIG_ENDIAN__ 4321
// COMMON:#define __ORDER_LITTLE_ENDIAN__ 1234
// COMMON:#define __ORDER_PDP_ENDIAN__ 3412
// COMMON:#define __STDC_HOSTED__ 1
// COMMON:#define __STDC_VERSION__
// COMMON:#define __STDC__ 1
// COMMON:#define __VERSION__
// COMMON:#define __clang__ 1
// COMMON:#define __clang_major__ {{[0-9]+}}
// COMMON:#define __clang_minor__ {{[0-9]+}}
// COMMON:#define __clang_patchlevel__ {{[0-9]+}}
// COMMON:#define __clang_version__
// COMMON:#define __llvm__ 1
//
// 
// RUN: %clang_cc1 -ffreestanding -E -dM < /dev/null | FileCheck -check-prefix FREESTANDING %s
// FREESTANDING:#define __STDC_HOSTED__ 0
//
//
// RUN: %clang_cc1 -x c++ -std=gnu++1y -E -dM < /dev/null | FileCheck -check-prefix GXX1Y %s
//
// GXX1Y:#define __GNUG__
// GXX1Y:#define __GXX_WEAK__ 1
// GXX1Y:#define __cplusplus 201402L
// GXX1Y:#define __private_extern__ extern
//
//
// RUN: %clang_cc1 -x c++ -std=gnu++11 -E -dM < /dev/null | FileCheck -check-prefix GXX11 %s
//
// GXX11:#define __GNUG__
// GXX11:#define __GXX_WEAK__ 1
// GXX11:#define __cplusplus 201103L
// GXX11:#define __private_extern__ extern
//
//
// RUN: %clang_cc1 -x c++ -std=gnu++98 -E -dM < /dev/null | FileCheck -check-prefix GXX98 %s
//
// GXX98:#define __GNUG__
// GXX98:#define __GXX_WEAK__ 1
// GXX98:#define __cplusplus 199711L
// GXX98:#define __private_extern__ extern
//
// 
// RUN: %clang_cc1 -std=iso9899:199409 -E -dM < /dev/null | FileCheck -check-prefix C94 %s
//
// C94:#define __STDC_VERSION__ 199409L
//
// 
// RUN: %clang_cc1 -fms-extensions -triple i686-pc-win32 -E -dM < /dev/null | FileCheck -check-prefix MSEXT %s
//
// MSEXT-NOT:#define __STDC__
// MSEXT:#define _INTEGRAL_MAX_BITS 64
// MSEXT-NOT:#define _NATIVE_WCHAR_T_DEFINED 1
// MSEXT-NOT:#define _WCHAR_T_DEFINED 1
//
//
// RUN: %clang_cc1 -x c++ -fms-extensions -triple i686-pc-win32 -E -dM < /dev/null | FileCheck -check-prefix MSEXT-CXX %s
//
// MSEXT-CXX:#define _NATIVE_WCHAR_T_DEFINED 1
// MSEXT-CXX:#define _WCHAR_T_DEFINED 1
//
//
// RUN: %clang_cc1 -x c++ -fno-wchar -fms-extensions -triple i686-pc-win32 -E -dM < /dev/null | FileCheck -check-prefix MSEXT-CXX-NOWCHAR %s
//
// MSEXT-CXX-NOWCHAR-NOT:#define _NATIVE_WCHAR_T_DEFINED 1
// MSEXT-CXX-NOWCHAR-NOT:#define _WCHAR_T_DEFINED 1
//
// 
// RUN: %clang_cc1 -x objective-c -E -dM < /dev/null | FileCheck -check-prefix OBJC %s
//
// OBJC:#define OBJC_NEW_PROPERTIES 1
// OBJC:#define __NEXT_RUNTIME__ 1
// OBJC:#define __OBJC__ 1
//
//
// RUN: %clang_cc1 -x objective-c -fobjc-gc -E -dM < /dev/null | FileCheck -check-prefix OBJCGC %s
//
// OBJCGC:#define __OBJC_GC__ 1
//
// 
// RUN: %clang_cc1 -x objective-c -fobjc-exceptions -E -dM < /dev/null | FileCheck -check-prefix NONFRAGILE %s
//
// NONFRAGILE:#define OBJC_ZEROCOST_EXCEPTIONS 1
// NONFRAGILE:#define __OBJC2__ 1
//
//
// RUN: %clang_cc1 -E -dM < /dev/null | FileCheck -check-prefix O0 %s
//
// O0:#define __NO_INLINE__ 1
// O0-NOT:#define __OPTIMIZE_SIZE__
// O0-NOT:#define __OPTIMIZE__
//
//
// RUN: %clang_cc1 -fno-inline -O3 -E -dM < /dev/null | FileCheck -check-prefix NO_INLINE %s
//
// NO_INLINE:#define __NO_INLINE__ 1
// NO_INLINE-NOT:#define __OPTIMIZE_SIZE__
// NO_INLINE:#define __OPTIMIZE__
//
//
// RUN: %clang_cc1 -O1 -E -dM < /dev/null | FileCheck -check-prefix O1 %s
//
// O1-NOT:#define __OPTIMIZE_SIZE__
// O1:#define __OPTIMIZE__ 1
//
//
// RUN: %clang_cc1 -Os -E -dM < /dev/null | FileCheck -check-prefix Os %s
//
// Os:#define __OPTIMIZE_SIZE__ 1
// Os:#define __OPTIMIZE__ 1
//
//
// RUN: %clang_cc1 -Oz -E -dM < /dev/null | FileCheck -check-prefix Oz %s
//
// Oz:#define __OPTIMIZE_SIZE__ 1
// Oz:#define __OPTIMIZE__ 1
//
//
// RUN: %clang_cc1 -fpascal-strings -E -dM < /dev/null | FileCheck -check-prefix PASCAL %s
//
// PASCAL:#define __PASCAL_STRINGS__ 1
//
// 
// RUN: %clang_cc1 -E -dM < /dev/null | FileCheck -check-prefix SCHAR %s
// 
// SCHAR:#define __STDC__ 1
// SCHAR-NOT:#define __UNSIGNED_CHAR__
// SCHAR:#define __clang__ 1
//
// RUN: %clang_cc1 -E -dM -fshort-wchar < /dev/null | FileCheck -check-prefix SHORTWCHAR %s
// wchar_t is u16 for targeting Win32.
// FIXME: Implement and check x86_64-cygwin.
// RUN: %clang_cc1 -E -dM -fno-short-wchar -triple=x86_64-w64-mingw32 < /dev/null | FileCheck -check-prefix SHORTWCHAR %s
//
// SHORTWCHAR: #define __SIZEOF_WCHAR_T__ 2
// SHORTWCHAR: #define __WCHAR_MAX__ 65535U
// SHORTWCHAR: #define __WCHAR_TYPE__ unsigned short
// SHORTWCHAR: #define __WCHAR_WIDTH__ 16
//
// RUN: %clang_cc1 -E -dM -fno-short-wchar -triple=i686-unknown-unknown < /dev/null | FileCheck -check-prefix SHORTWCHAR2 %s
// RUN: %clang_cc1 -E -dM -fno-short-wchar -triple=x86_64-unknown-unknown < /dev/null | FileCheck -check-prefix SHORTWCHAR2 %s
//
// SHORTWCHAR2: #define __SIZEOF_WCHAR_T__ 4
// SHORTWCHAR2: #define __WCHAR_WIDTH__ 32
// Other definitions vary from platform to platform
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=aarch64-none-none < /dev/null | FileCheck -check-prefix AARCH64 %s
//
// AARCH64:#define _LP64 1
// AARCH64:#define __AARCH64EL__ 1
// AARCH64:#define __ARM_64BIT_STATE 1
// AARCH64:#define __ARM_ARCH 8
// AARCH64:#define __ARM_ARCH_ISA_A64 1
// AARCH64:#define __BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__
// AARCH64:#define __CHAR16_TYPE__ unsigned short
// AARCH64:#define __CHAR32_TYPE__ unsigned int
// AARCH64:#define __CHAR_BIT__ 8
// AARCH64:#define __DBL_DENORM_MIN__ 4.9406564584124654e-324
// AARCH64:#define __DBL_DIG__ 15
// AARCH64:#define __DBL_EPSILON__ 2.2204460492503131e-16
// AARCH64:#define __DBL_HAS_DENORM__ 1
// AARCH64:#define __DBL_HAS_INFINITY__ 1
// AARCH64:#define __DBL_HAS_QUIET_NAN__ 1
// AARCH64:#define __DBL_MANT_DIG__ 53
// AARCH64:#define __DBL_MAX_10_EXP__ 308
// AARCH64:#define __DBL_MAX_EXP__ 1024
// AARCH64:#define __DBL_MAX__ 1.7976931348623157e+308
// AARCH64:#define __DBL_MIN_10_EXP__ (-307)
// AARCH64:#define __DBL_MIN_EXP__ (-1021)
// AARCH64:#define __DBL_MIN__ 2.2250738585072014e-308
// AARCH64:#define __DECIMAL_DIG__ 36
// AARCH64:#define __FLT_DENORM_MIN__ 1.40129846e-45F
// AARCH64:#define __FLT_DIG__ 6
// AARCH64:#define __FLT_EPSILON__ 1.19209290e-7F
// AARCH64:#define __FLT_EVAL_METHOD__ 0
// AARCH64:#define __FLT_HAS_DENORM__ 1
// AARCH64:#define __FLT_HAS_INFINITY__ 1
// AARCH64:#define __FLT_HAS_QUIET_NAN__ 1
// AARCH64:#define __FLT_MANT_DIG__ 24
// AARCH64:#define __FLT_MAX_10_EXP__ 38
// AARCH64:#define __FLT_MAX_EXP__ 128
// AARCH64:#define __FLT_MAX__ 3.40282347e+38F
// AARCH64:#define __FLT_MIN_10_EXP__ (-37)
// AARCH64:#define __FLT_MIN_EXP__ (-125)
// AARCH64:#define __FLT_MIN__ 1.17549435e-38F
// AARCH64:#define __FLT_RADIX__ 2
// AARCH64:#define __INT16_TYPE__ short
// AARCH64:#define __INT32_TYPE__ int
// AARCH64:#define __INT64_C_SUFFIX__ L
// AARCH64:#define __INT64_TYPE__ long int
// AARCH64:#define __INT8_TYPE__ char
// AARCH64:#define __INTMAX_MAX__ 9223372036854775807L
// AARCH64:#define __INTMAX_TYPE__ long int
// AARCH64:#define __INTMAX_WIDTH__ 64
// AARCH64:#define __INTPTR_TYPE__ long int
// AARCH64:#define __INTPTR_WIDTH__ 64
// AARCH64:#define __INT_MAX__ 2147483647
// AARCH64:#define __LDBL_DENORM_MIN__ 6.47517511943802511092443895822764655e-4966L
// AARCH64:#define __LDBL_DIG__ 33
// AARCH64:#define __LDBL_EPSILON__ 1.92592994438723585305597794258492732e-34L
// AARCH64:#define __LDBL_HAS_DENORM__ 1
// AARCH64:#define __LDBL_HAS_INFINITY__ 1
// AARCH64:#define __LDBL_HAS_QUIET_NAN__ 1
// AARCH64:#define __LDBL_MANT_DIG__ 113
// AARCH64:#define __LDBL_MAX_10_EXP__ 4932
// AARCH64:#define __LDBL_MAX_EXP__ 16384
// AARCH64:#define __LDBL_MAX__ 1.18973149535723176508575932662800702e+4932L
// AARCH64:#define __LDBL_MIN_10_EXP__ (-4931)
// AARCH64:#define __LDBL_MIN_EXP__ (-16381)
// AARCH64:#define __LDBL_MIN__ 3.36210314311209350626267781732175260e-4932L
// AARCH64:#define __LONG_LONG_MAX__ 9223372036854775807LL
// AARCH64:#define __LONG_MAX__ 9223372036854775807L
// AARCH64:#define __LP64__ 1
// AARCH64:#define __POINTER_WIDTH__ 64
// AARCH64:#define __PTRDIFF_TYPE__ long int
// AARCH64:#define __PTRDIFF_WIDTH__ 64
// AARCH64:#define __SCHAR_MAX__ 127
// AARCH64:#define __SHRT_MAX__ 32767
// AARCH64:#define __SIG_ATOMIC_WIDTH__ 32
// AARCH64:#define __SIZEOF_DOUBLE__ 8
// AARCH64:#define __SIZEOF_FLOAT__ 4
// AARCH64:#define __SIZEOF_INT128__ 16
// AARCH64:#define __SIZEOF_INT__ 4
// AARCH64:#define __SIZEOF_LONG_DOUBLE__ 16
// AARCH64:#define __SIZEOF_LONG_LONG__ 8
// AARCH64:#define __SIZEOF_LONG__ 8
// AARCH64:#define __SIZEOF_POINTER__ 8
// AARCH64:#define __SIZEOF_PTRDIFF_T__ 8
// AARCH64:#define __SIZEOF_SHORT__ 2
// AARCH64:#define __SIZEOF_SIZE_T__ 8
// AARCH64:#define __SIZEOF_WCHAR_T__ 4
// AARCH64:#define __SIZEOF_WINT_T__ 4
// AARCH64:#define __SIZE_MAX__ 18446744073709551615UL
// AARCH64:#define __SIZE_TYPE__ long unsigned int
// AARCH64:#define __SIZE_WIDTH__ 64
// AARCH64:#define __UINTMAX_TYPE__ long unsigned int
// AARCH64:#define __USER_LABEL_PREFIX__ _
// AARCH64:#define __WCHAR_MAX__ 4294967295U
// AARCH64:#define __WCHAR_TYPE__ unsigned int
// AARCH64:#define __WCHAR_UNSIGNED__ 1
// AARCH64:#define __WCHAR_WIDTH__ 32
// AARCH64:#define __WINT_TYPE__ int
// AARCH64:#define __WINT_WIDTH__ 32
// AARCH64:#define __aarch64__ 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=aarch64-netbsd < /dev/null | FileCheck -check-prefix AARCH64-NETBSD %s
//
// AARCH64-NETBSD:#define _LP64 1
// AARCH64-NETBSD:#define __AARCH64EL__ 1
// AARCH64-NETBSD:#define __ARM_64BIT_STATE 1
// AARCH64-NETBSD:#define __ARM_ARCH 8
// AARCH64-NETBSD:#define __ARM_ARCH_ISA_A64 1
// AARCH64-NETBSD:#define __BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__
// AARCH64-NETBSD:#define __CHAR16_TYPE__ unsigned short
// AARCH64-NETBSD:#define __CHAR32_TYPE__ unsigned int
// AARCH64-NETBSD:#define __CHAR_BIT__ 8
// AARCH64-NETBSD:#define __DBL_DENORM_MIN__ 4.9406564584124654e-324
// AARCH64-NETBSD:#define __DBL_DIG__ 15
// AARCH64-NETBSD:#define __DBL_EPSILON__ 2.2204460492503131e-16
// AARCH64-NETBSD:#define __DBL_HAS_DENORM__ 1
// AARCH64-NETBSD:#define __DBL_HAS_INFINITY__ 1
// AARCH64-NETBSD:#define __DBL_HAS_QUIET_NAN__ 1
// AARCH64-NETBSD:#define __DBL_MANT_DIG__ 53
// AARCH64-NETBSD:#define __DBL_MAX_10_EXP__ 308
// AARCH64-NETBSD:#define __DBL_MAX_EXP__ 1024
// AARCH64-NETBSD:#define __DBL_MAX__ 1.7976931348623157e+308
// AARCH64-NETBSD:#define __DBL_MIN_10_EXP__ (-307)
// AARCH64-NETBSD:#define __DBL_MIN_EXP__ (-1021)
// AARCH64-NETBSD:#define __DBL_MIN__ 2.2250738585072014e-308
// AARCH64-NETBSD:#define __DECIMAL_DIG__ 36
// AARCH64-NETBSD:#define __ELF__ 1
// AARCH64-NETBSD:#define __FLT_DENORM_MIN__ 1.40129846e-45F
// AARCH64-NETBSD:#define __FLT_DIG__ 6
// AARCH64-NETBSD:#define __FLT_EPSILON__ 1.19209290e-7F
// AARCH64-NETBSD:#define __FLT_EVAL_METHOD__ 0
// AARCH64-NETBSD:#define __FLT_HAS_DENORM__ 1
// AARCH64-NETBSD:#define __FLT_HAS_INFINITY__ 1
// AARCH64-NETBSD:#define __FLT_HAS_QUIET_NAN__ 1
// AARCH64-NETBSD:#define __FLT_MANT_DIG__ 24
// AARCH64-NETBSD:#define __FLT_MAX_10_EXP__ 38
// AARCH64-NETBSD:#define __FLT_MAX_EXP__ 128
// AARCH64-NETBSD:#define __FLT_MAX__ 3.40282347e+38F
// AARCH64-NETBSD:#define __FLT_MIN_10_EXP__ (-37)
// AARCH64-NETBSD:#define __FLT_MIN_EXP__ (-125)
// AARCH64-NETBSD:#define __FLT_MIN__ 1.17549435e-38F
// AARCH64-NETBSD:#define __FLT_RADIX__ 2
// AARCH64-NETBSD:#define __INT16_TYPE__ short
// AARCH64-NETBSD:#define __INT32_TYPE__ int
// AARCH64-NETBSD:#define __INT64_C_SUFFIX__ L
// AARCH64-NETBSD:#define __INT64_TYPE__ long int
// AARCH64-NETBSD:#define __INT8_TYPE__ char
// AARCH64-NETBSD:#define __INTMAX_MAX__ 9223372036854775807L
// AARCH64-NETBSD:#define __INTMAX_TYPE__ long int
// AARCH64-NETBSD:#define __INTMAX_WIDTH__ 64
// AARCH64-NETBSD:#define __INTPTR_TYPE__ long int
// AARCH64-NETBSD:#define __INTPTR_WIDTH__ 64
// AARCH64-NETBSD:#define __INT_MAX__ 2147483647
// AARCH64-NETBSD:#define __LDBL_DENORM_MIN__ 6.47517511943802511092443895822764655e-4966L
// AARCH64-NETBSD:#define __LDBL_DIG__ 33
// AARCH64-NETBSD:#define __LDBL_EPSILON__ 1.92592994438723585305597794258492732e-34L
// AARCH64-NETBSD:#define __LDBL_HAS_DENORM__ 1
// AARCH64-NETBSD:#define __LDBL_HAS_INFINITY__ 1
// AARCH64-NETBSD:#define __LDBL_HAS_QUIET_NAN__ 1
// AARCH64-NETBSD:#define __LDBL_MANT_DIG__ 113
// AARCH64-NETBSD:#define __LDBL_MAX_10_EXP__ 4932
// AARCH64-NETBSD:#define __LDBL_MAX_EXP__ 16384
// AARCH64-NETBSD:#define __LDBL_MAX__ 1.18973149535723176508575932662800702e+4932L
// AARCH64-NETBSD:#define __LDBL_MIN_10_EXP__ (-4931)
// AARCH64-NETBSD:#define __LDBL_MIN_EXP__ (-16381)
// AARCH64-NETBSD:#define __LDBL_MIN__ 3.36210314311209350626267781732175260e-4932L
// AARCH64-NETBSD:#define __LITTLE_ENDIAN__ 1
// AARCH64-NETBSD:#define __LONG_LONG_MAX__ 9223372036854775807LL
// AARCH64-NETBSD:#define __LONG_MAX__ 9223372036854775807L
// AARCH64-NETBSD:#define __LP64__ 1
// AARCH64-NETBSD:#define __NetBSD__ 1
// AARCH64-NETBSD:#define __POINTER_WIDTH__ 64
// AARCH64-NETBSD:#define __PTRDIFF_TYPE__ long int
// AARCH64-NETBSD:#define __PTRDIFF_WIDTH__ 64
// AARCH64-NETBSD:#define __SCHAR_MAX__ 127
// AARCH64-NETBSD:#define __SHRT_MAX__ 32767
// AARCH64-NETBSD:#define __SIG_ATOMIC_WIDTH__ 32
// AARCH64-NETBSD:#define __SIZEOF_DOUBLE__ 8
// AARCH64-NETBSD:#define __SIZEOF_FLOAT__ 4
// AARCH64-NETBSD:#define __SIZEOF_INT__ 4
// AARCH64-NETBSD:#define __SIZEOF_LONG_DOUBLE__ 16
// AARCH64-NETBSD:#define __SIZEOF_LONG_LONG__ 8
// AARCH64-NETBSD:#define __SIZEOF_LONG__ 8
// AARCH64-NETBSD:#define __SIZEOF_POINTER__ 8
// AARCH64-NETBSD:#define __SIZEOF_PTRDIFF_T__ 8
// AARCH64-NETBSD:#define __SIZEOF_SHORT__ 2
// AARCH64-NETBSD:#define __SIZEOF_SIZE_T__ 8
// AARCH64-NETBSD:#define __SIZEOF_WCHAR_T__ 4
// AARCH64-NETBSD:#define __SIZEOF_WINT_T__ 4
// AARCH64-NETBSD:#define __SIZE_MAX__ 18446744073709551615UL
// AARCH64-NETBSD:#define __SIZE_TYPE__ long unsigned int
// AARCH64-NETBSD:#define __SIZE_WIDTH__ 64
// AARCH64-NETBSD:#define __UINTMAX_TYPE__ long unsigned int
// AARCH64-NETBSD:#define __USER_LABEL_PREFIX__
// AARCH64-NETBSD:#define __WCHAR_MAX__ 2147483647
// AARCH64-NETBSD:#define __WCHAR_TYPE__ int
// AARCH64-NETBSD:#define __WCHAR_WIDTH__ 32
// AARCH64-NETBSD:#define __WINT_TYPE__ int
// AARCH64-NETBSD:#define __WINT_WIDTH__ 32
// AARCH64-NETBSD:#define __aarch64__ 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=arm-none-none < /dev/null | FileCheck -check-prefix ARM %s
//
// ARM-NOT:#define _LP64
// ARM:#define __APCS_32__ 1
// ARM:#define __ARMEL__ 1
// ARM:#define __ARM_ARCH_6J__ 1
// ARM:#define __BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__
// ARM:#define __CHAR16_TYPE__ unsigned short
// ARM:#define __CHAR32_TYPE__ unsigned int
// ARM:#define __CHAR_BIT__ 8
// ARM:#define __DBL_DENORM_MIN__ 4.9406564584124654e-324
// ARM:#define __DBL_DIG__ 15
// ARM:#define __DBL_EPSILON__ 2.2204460492503131e-16
// ARM:#define __DBL_HAS_DENORM__ 1
// ARM:#define __DBL_HAS_INFINITY__ 1
// ARM:#define __DBL_HAS_QUIET_NAN__ 1
// ARM:#define __DBL_MANT_DIG__ 53
// ARM:#define __DBL_MAX_10_EXP__ 308
// ARM:#define __DBL_MAX_EXP__ 1024
// ARM:#define __DBL_MAX__ 1.7976931348623157e+308
// ARM:#define __DBL_MIN_10_EXP__ (-307)
// ARM:#define __DBL_MIN_EXP__ (-1021)
// ARM:#define __DBL_MIN__ 2.2250738585072014e-308
// ARM:#define __DECIMAL_DIG__ 17
// ARM:#define __FLT_DENORM_MIN__ 1.40129846e-45F
// ARM:#define __FLT_DIG__ 6
// ARM:#define __FLT_EPSILON__ 1.19209290e-7F
// ARM:#define __FLT_EVAL_METHOD__ 0
// ARM:#define __FLT_HAS_DENORM__ 1
// ARM:#define __FLT_HAS_INFINITY__ 1
// ARM:#define __FLT_HAS_QUIET_NAN__ 1
// ARM:#define __FLT_MANT_DIG__ 24
// ARM:#define __FLT_MAX_10_EXP__ 38
// ARM:#define __FLT_MAX_EXP__ 128
// ARM:#define __FLT_MAX__ 3.40282347e+38F
// ARM:#define __FLT_MIN_10_EXP__ (-37)
// ARM:#define __FLT_MIN_EXP__ (-125)
// ARM:#define __FLT_MIN__ 1.17549435e-38F
// ARM:#define __FLT_RADIX__ 2
// ARM:#define __INT16_TYPE__ short
// ARM:#define __INT32_TYPE__ int
// ARM:#define __INT64_C_SUFFIX__ LL
// ARM:#define __INT64_TYPE__ long long int
// ARM:#define __INT8_TYPE__ char
// ARM:#define __INTMAX_MAX__ 9223372036854775807LL
// ARM:#define __INTMAX_TYPE__ long long int
// ARM:#define __INTMAX_WIDTH__ 64
// ARM:#define __INTPTR_TYPE__ long int
// ARM:#define __INTPTR_WIDTH__ 32
// ARM:#define __INT_MAX__ 2147483647
// ARM:#define __LDBL_DENORM_MIN__ 4.9406564584124654e-324L
// ARM:#define __LDBL_DIG__ 15
// ARM:#define __LDBL_EPSILON__ 2.2204460492503131e-16L
// ARM:#define __LDBL_HAS_DENORM__ 1
// ARM:#define __LDBL_HAS_INFINITY__ 1
// ARM:#define __LDBL_HAS_QUIET_NAN__ 1
// ARM:#define __LDBL_MANT_DIG__ 53
// ARM:#define __LDBL_MAX_10_EXP__ 308
// ARM:#define __LDBL_MAX_EXP__ 1024
// ARM:#define __LDBL_MAX__ 1.7976931348623157e+308L
// ARM:#define __LDBL_MIN_10_EXP__ (-307)
// ARM:#define __LDBL_MIN_EXP__ (-1021)
// ARM:#define __LDBL_MIN__ 2.2250738585072014e-308L
// ARM:#define __LITTLE_ENDIAN__ 1
// ARM:#define __LONG_LONG_MAX__ 9223372036854775807LL
// ARM:#define __LONG_MAX__ 2147483647L
// ARM-NOT:#define __LP64__
// ARM:#define __POINTER_WIDTH__ 32
// ARM:#define __PTRDIFF_TYPE__ int
// ARM:#define __PTRDIFF_WIDTH__ 32
// ARM:#define __REGISTER_PREFIX__
// ARM:#define __SCHAR_MAX__ 127
// ARM:#define __SHRT_MAX__ 32767
// ARM:#define __SIG_ATOMIC_WIDTH__ 32
// ARM:#define __SIZEOF_DOUBLE__ 8
// ARM:#define __SIZEOF_FLOAT__ 4
// ARM:#define __SIZEOF_INT__ 4
// ARM:#define __SIZEOF_LONG_DOUBLE__ 8
// ARM:#define __SIZEOF_LONG_LONG__ 8
// ARM:#define __SIZEOF_LONG__ 4
// ARM:#define __SIZEOF_POINTER__ 4
// ARM:#define __SIZEOF_PTRDIFF_T__ 4
// ARM:#define __SIZEOF_SHORT__ 2
// ARM:#define __SIZEOF_SIZE_T__ 4
// ARM:#define __SIZEOF_WCHAR_T__ 4
// ARM:#define __SIZEOF_WINT_T__ 4
// ARM:#define __SIZE_MAX__ 4294967295U
// ARM:#define __SIZE_TYPE__ unsigned int
// ARM:#define __SIZE_WIDTH__ 32
// ARM:#define __THUMB_INTERWORK__ 1
// ARM:#define __UINTMAX_TYPE__ long long unsigned int
// ARM:#define __USER_LABEL_PREFIX__ _
// ARM:#define __WCHAR_MAX__ 4294967295U
// ARM:#define __WCHAR_TYPE__ unsigned int
// ARM:#define __WCHAR_WIDTH__ 32
// ARM:#define __WINT_TYPE__ int
// ARM:#define __WINT_WIDTH__ 32
// ARM:#define __arm 1
// ARM:#define __arm__ 1

// RUN: %clang_cc1 -E -dM -ffreestanding -triple=arm-none-linux-gnueabi -target-feature +soft-float -target-feature +soft-float-abi < /dev/null | FileCheck -check-prefix ARMEABISOFTFP %s
//
// ARM-NOT:#define _LP64
// ARMEABISOFTFP:#define __APCS_32__ 1
// ARMEABISOFTFP:#define __ARMEL__ 1
// ARMEABISOFTFP:#define __ARM_ARCH 6
// ARMEABISOFTFP:#define __ARM_ARCH_6J__ 1
// ARMEABISOFTFP:#define __ARM_EABI__ 1
// ARMEABISOFTFP:#define __ARM_PCS 1
// ARMEABISOFTFP-NOT:#define __ARM_PCS_VFP 1
// ARMEABISOFTFP:#define __BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__
// ARMEABISOFTFP:#define __CHAR16_TYPE__ unsigned short
// ARMEABISOFTFP:#define __CHAR32_TYPE__ unsigned int
// ARMEABISOFTFP:#define __CHAR_BIT__ 8
// ARMEABISOFTFP:#define __DBL_DENORM_MIN__ 4.9406564584124654e-324
// ARMEABISOFTFP:#define __DBL_DIG__ 15
// ARMEABISOFTFP:#define __DBL_EPSILON__ 2.2204460492503131e-16
// ARMEABISOFTFP:#define __DBL_HAS_DENORM__ 1
// ARMEABISOFTFP:#define __DBL_HAS_INFINITY__ 1
// ARMEABISOFTFP:#define __DBL_HAS_QUIET_NAN__ 1
// ARMEABISOFTFP:#define __DBL_MANT_DIG__ 53
// ARMEABISOFTFP:#define __DBL_MAX_10_EXP__ 308
// ARMEABISOFTFP:#define __DBL_MAX_EXP__ 1024
// ARMEABISOFTFP:#define __DBL_MAX__ 1.7976931348623157e+308
// ARMEABISOFTFP:#define __DBL_MIN_10_EXP__ (-307)
// ARMEABISOFTFP:#define __DBL_MIN_EXP__ (-1021)
// ARMEABISOFTFP:#define __DBL_MIN__ 2.2250738585072014e-308
// ARMEABISOFTFP:#define __DECIMAL_DIG__ 17
// ARMEABISOFTFP:#define __FLT_DENORM_MIN__ 1.40129846e-45F
// ARMEABISOFTFP:#define __FLT_DIG__ 6
// ARMEABISOFTFP:#define __FLT_EPSILON__ 1.19209290e-7F
// ARMEABISOFTFP:#define __FLT_EVAL_METHOD__ 0
// ARMEABISOFTFP:#define __FLT_HAS_DENORM__ 1
// ARMEABISOFTFP:#define __FLT_HAS_INFINITY__ 1
// ARMEABISOFTFP:#define __FLT_HAS_QUIET_NAN__ 1
// ARMEABISOFTFP:#define __FLT_MANT_DIG__ 24
// ARMEABISOFTFP:#define __FLT_MAX_10_EXP__ 38
// ARMEABISOFTFP:#define __FLT_MAX_EXP__ 128
// ARMEABISOFTFP:#define __FLT_MAX__ 3.40282347e+38F
// ARMEABISOFTFP:#define __FLT_MIN_10_EXP__ (-37)
// ARMEABISOFTFP:#define __FLT_MIN_EXP__ (-125)
// ARMEABISOFTFP:#define __FLT_MIN__ 1.17549435e-38F
// ARMEABISOFTFP:#define __FLT_RADIX__ 2
// ARMEABISOFTFP:#define __INT16_TYPE__ short
// ARMEABISOFTFP:#define __INT32_TYPE__ int
// ARMEABISOFTFP:#define __INT64_C_SUFFIX__ LL
// ARMEABISOFTFP:#define __INT64_TYPE__ long long int
// ARMEABISOFTFP:#define __INT8_TYPE__ char
// ARMEABISOFTFP:#define __INTMAX_MAX__ 9223372036854775807LL
// ARMEABISOFTFP:#define __INTMAX_TYPE__ long long int
// ARMEABISOFTFP:#define __INTMAX_WIDTH__ 64
// ARMEABISOFTFP:#define __INTPTR_TYPE__ long int
// ARMEABISOFTFP:#define __INTPTR_WIDTH__ 32
// ARMEABISOFTFP:#define __INT_MAX__ 2147483647
// ARMEABISOFTFP:#define __LDBL_DENORM_MIN__ 4.9406564584124654e-324L
// ARMEABISOFTFP:#define __LDBL_DIG__ 15
// ARMEABISOFTFP:#define __LDBL_EPSILON__ 2.2204460492503131e-16L
// ARMEABISOFTFP:#define __LDBL_HAS_DENORM__ 1
// ARMEABISOFTFP:#define __LDBL_HAS_INFINITY__ 1
// ARMEABISOFTFP:#define __LDBL_HAS_QUIET_NAN__ 1
// ARMEABISOFTFP:#define __LDBL_MANT_DIG__ 53
// ARMEABISOFTFP:#define __LDBL_MAX_10_EXP__ 308
// ARMEABISOFTFP:#define __LDBL_MAX_EXP__ 1024
// ARMEABISOFTFP:#define __LDBL_MAX__ 1.7976931348623157e+308L
// ARMEABISOFTFP:#define __LDBL_MIN_10_EXP__ (-307)
// ARMEABISOFTFP:#define __LDBL_MIN_EXP__ (-1021)
// ARMEABISOFTFP:#define __LDBL_MIN__ 2.2250738585072014e-308L
// ARMEABISOFTFP:#define __LITTLE_ENDIAN__ 1
// ARMEABISOFTFP:#define __LONG_LONG_MAX__ 9223372036854775807LL
// ARMEABISOFTFP:#define __LONG_MAX__ 2147483647L
// ARMEABISOFTFP-NOT:#define __LP64__
// ARMEABISOFTFP:#define __POINTER_WIDTH__ 32
// ARMEABISOFTFP:#define __PTRDIFF_TYPE__ int
// ARMEABISOFTFP:#define __PTRDIFF_WIDTH__ 32
// ARMEABISOFTFP:#define __REGISTER_PREFIX__
// ARMEABISOFTFP:#define __SCHAR_MAX__ 127
// ARMEABISOFTFP:#define __SHRT_MAX__ 32767
// ARMEABISOFTFP:#define __SIG_ATOMIC_WIDTH__ 32
// ARMEABISOFTFP:#define __SIZEOF_DOUBLE__ 8
// ARMEABISOFTFP:#define __SIZEOF_FLOAT__ 4
// ARMEABISOFTFP:#define __SIZEOF_INT__ 4
// ARMEABISOFTFP:#define __SIZEOF_LONG_DOUBLE__ 8
// ARMEABISOFTFP:#define __SIZEOF_LONG_LONG__ 8
// ARMEABISOFTFP:#define __SIZEOF_LONG__ 4
// ARMEABISOFTFP:#define __SIZEOF_POINTER__ 4
// ARMEABISOFTFP:#define __SIZEOF_PTRDIFF_T__ 4
// ARMEABISOFTFP:#define __SIZEOF_SHORT__ 2
// ARMEABISOFTFP:#define __SIZEOF_SIZE_T__ 4
// ARMEABISOFTFP:#define __SIZEOF_WCHAR_T__ 4
// ARMEABISOFTFP:#define __SIZEOF_WINT_T__ 4
// ARMEABISOFTFP:#define __SIZE_MAX__ 4294967295U
// ARMEABISOFTFP:#define __SIZE_TYPE__ unsigned int
// ARMEABISOFTFP:#define __SIZE_WIDTH__ 32
// ARMEABISOFTFP:#define __SOFTFP__ 1
// ARMEABISOFTFP:#define __THUMB_INTERWORK__ 1
// ARMEABISOFTFP:#define __UINTMAX_TYPE__ long long unsigned int
// ARMEABISOFTFP:#define __USER_LABEL_PREFIX__
// ARMEABISOFTFP:#define __WCHAR_MAX__ 4294967295U
// ARMEABISOFTFP:#define __WCHAR_TYPE__ unsigned int
// ARMEABISOFTFP:#define __WCHAR_WIDTH__ 32
// ARMEABISOFTFP:#define __WINT_TYPE__ unsigned int
// ARMEABISOFTFP:#define __WINT_WIDTH__ 32
// ARMEABISOFTFP:#define __arm 1
// ARMEABISOFTFP:#define __arm__ 1

// RUN: %clang_cc1 -E -dM -ffreestanding -triple=arm-none-linux-gnueabi < /dev/null | FileCheck -check-prefix ARMEABIHARDFP %s
//
// ARM-NOT:#define _LP64
// ARMEABIHARDFP:#define __APCS_32__ 1
// ARMEABIHARDFP:#define __ARMEL__ 1
// ARMEABIHARDFP:#define __ARM_ARCH 6
// ARMEABIHARDFP:#define __ARM_ARCH_6J__ 1
// ARMEABIHARDFP:#define __ARM_EABI__ 1
// ARMEABIHARDFP:#define __ARM_PCS 1
// ARMEABIHARDFP:#define __ARM_PCS_VFP 1
// ARMEABIHARDFP:#define __BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__
// ARMEABIHARDFP:#define __CHAR16_TYPE__ unsigned short
// ARMEABIHARDFP:#define __CHAR32_TYPE__ unsigned int
// ARMEABIHARDFP:#define __CHAR_BIT__ 8
// ARMEABIHARDFP:#define __DBL_DENORM_MIN__ 4.9406564584124654e-324
// ARMEABIHARDFP:#define __DBL_DIG__ 15
// ARMEABIHARDFP:#define __DBL_EPSILON__ 2.2204460492503131e-16
// ARMEABIHARDFP:#define __DBL_HAS_DENORM__ 1
// ARMEABIHARDFP:#define __DBL_HAS_INFINITY__ 1
// ARMEABIHARDFP:#define __DBL_HAS_QUIET_NAN__ 1
// ARMEABIHARDFP:#define __DBL_MANT_DIG__ 53
// ARMEABIHARDFP:#define __DBL_MAX_10_EXP__ 308
// ARMEABIHARDFP:#define __DBL_MAX_EXP__ 1024
// ARMEABIHARDFP:#define __DBL_MAX__ 1.7976931348623157e+308
// ARMEABIHARDFP:#define __DBL_MIN_10_EXP__ (-307)
// ARMEABIHARDFP:#define __DBL_MIN_EXP__ (-1021)
// ARMEABIHARDFP:#define __DBL_MIN__ 2.2250738585072014e-308
// ARMEABIHARDFP:#define __DECIMAL_DIG__ 17
// ARMEABIHARDFP:#define __FLT_DENORM_MIN__ 1.40129846e-45F
// ARMEABIHARDFP:#define __FLT_DIG__ 6
// ARMEABIHARDFP:#define __FLT_EPSILON__ 1.19209290e-7F
// ARMEABIHARDFP:#define __FLT_EVAL_METHOD__ 0
// ARMEABIHARDFP:#define __FLT_HAS_DENORM__ 1
// ARMEABIHARDFP:#define __FLT_HAS_INFINITY__ 1
// ARMEABIHARDFP:#define __FLT_HAS_QUIET_NAN__ 1
// ARMEABIHARDFP:#define __FLT_MANT_DIG__ 24
// ARMEABIHARDFP:#define __FLT_MAX_10_EXP__ 38
// ARMEABIHARDFP:#define __FLT_MAX_EXP__ 128
// ARMEABIHARDFP:#define __FLT_MAX__ 3.40282347e+38F
// ARMEABIHARDFP:#define __FLT_MIN_10_EXP__ (-37)
// ARMEABIHARDFP:#define __FLT_MIN_EXP__ (-125)
// ARMEABIHARDFP:#define __FLT_MIN__ 1.17549435e-38F
// ARMEABIHARDFP:#define __FLT_RADIX__ 2
// ARMEABIHARDFP:#define __INT16_TYPE__ short
// ARMEABIHARDFP:#define __INT32_TYPE__ int
// ARMEABIHARDFP:#define __INT64_C_SUFFIX__ LL
// ARMEABIHARDFP:#define __INT64_TYPE__ long long int
// ARMEABIHARDFP:#define __INT8_TYPE__ char
// ARMEABIHARDFP:#define __INTMAX_MAX__ 9223372036854775807LL
// ARMEABIHARDFP:#define __INTMAX_TYPE__ long long int
// ARMEABIHARDFP:#define __INTMAX_WIDTH__ 64
// ARMEABIHARDFP:#define __INTPTR_TYPE__ long int
// ARMEABIHARDFP:#define __INTPTR_WIDTH__ 32
// ARMEABIHARDFP:#define __INT_MAX__ 2147483647
// ARMEABIHARDFP:#define __LDBL_DENORM_MIN__ 4.9406564584124654e-324L
// ARMEABIHARDFP:#define __LDBL_DIG__ 15
// ARMEABIHARDFP:#define __LDBL_EPSILON__ 2.2204460492503131e-16L
// ARMEABIHARDFP:#define __LDBL_HAS_DENORM__ 1
// ARMEABIHARDFP:#define __LDBL_HAS_INFINITY__ 1
// ARMEABIHARDFP:#define __LDBL_HAS_QUIET_NAN__ 1
// ARMEABIHARDFP:#define __LDBL_MANT_DIG__ 53
// ARMEABIHARDFP:#define __LDBL_MAX_10_EXP__ 308
// ARMEABIHARDFP:#define __LDBL_MAX_EXP__ 1024
// ARMEABIHARDFP:#define __LDBL_MAX__ 1.7976931348623157e+308L
// ARMEABIHARDFP:#define __LDBL_MIN_10_EXP__ (-307)
// ARMEABIHARDFP:#define __LDBL_MIN_EXP__ (-1021)
// ARMEABIHARDFP:#define __LDBL_MIN__ 2.2250738585072014e-308L
// ARMEABIHARDFP:#define __LITTLE_ENDIAN__ 1
// ARMEABIHARDFP:#define __LONG_LONG_MAX__ 9223372036854775807LL
// ARMEABIHARDFP:#define __LONG_MAX__ 2147483647L
// ARMEABIHARDFP-NOT:#define __LP64__
// ARMEABIHARDFP:#define __POINTER_WIDTH__ 32
// ARMEABIHARDFP:#define __PTRDIFF_TYPE__ int
// ARMEABIHARDFP:#define __PTRDIFF_WIDTH__ 32
// ARMEABIHARDFP:#define __REGISTER_PREFIX__
// ARMEABIHARDFP:#define __SCHAR_MAX__ 127
// ARMEABIHARDFP:#define __SHRT_MAX__ 32767
// ARMEABIHARDFP:#define __SIG_ATOMIC_WIDTH__ 32
// ARMEABIHARDFP:#define __SIZEOF_DOUBLE__ 8
// ARMEABIHARDFP:#define __SIZEOF_FLOAT__ 4
// ARMEABIHARDFP:#define __SIZEOF_INT__ 4
// ARMEABIHARDFP:#define __SIZEOF_LONG_DOUBLE__ 8
// ARMEABIHARDFP:#define __SIZEOF_LONG_LONG__ 8
// ARMEABIHARDFP:#define __SIZEOF_LONG__ 4
// ARMEABIHARDFP:#define __SIZEOF_POINTER__ 4
// ARMEABIHARDFP:#define __SIZEOF_PTRDIFF_T__ 4
// ARMEABIHARDFP:#define __SIZEOF_SHORT__ 2
// ARMEABIHARDFP:#define __SIZEOF_SIZE_T__ 4
// ARMEABIHARDFP:#define __SIZEOF_WCHAR_T__ 4
// ARMEABIHARDFP:#define __SIZEOF_WINT_T__ 4
// ARMEABIHARDFP:#define __SIZE_MAX__ 4294967295U
// ARMEABIHARDFP:#define __SIZE_TYPE__ unsigned int
// ARMEABIHARDFP:#define __SIZE_WIDTH__ 32
// ARMEABIHARDFP-NOT:#define __SOFTFP__ 1
// ARMEABIHARDFP:#define __THUMB_INTERWORK__ 1
// ARMEABIHARDFP:#define __UINTMAX_TYPE__ long long unsigned int
// ARMEABIHARDFP:#define __USER_LABEL_PREFIX__
// ARMEABIHARDFP:#define __WCHAR_MAX__ 4294967295U
// ARMEABIHARDFP:#define __WCHAR_TYPE__ unsigned int
// ARMEABIHARDFP:#define __WCHAR_WIDTH__ 32
// ARMEABIHARDFP:#define __WINT_TYPE__ unsigned int
// ARMEABIHARDFP:#define __WINT_WIDTH__ 32
// ARMEABIHARDFP:#define __arm 1
// ARMEABIHARDFP:#define __arm__ 1

// RUN: %clang -target arm -arch armv7s -x c -E -dM %s -o - | FileCheck --check-prefix=ARM-DARWIN-NO-EABI %s
// RUN: %clang -target arm -arch armv6m -x c -E -dM %s -o - | FileCheck --check-prefix=ARM-DARWIN-NO-EABI %s
// RUN: %clang -target arm -arch armv7m -x c -E -dM %s -o - | FileCheck --check-prefix=ARM-DARWIN-NO-EABI %s
// RUN: %clang -target arm -arch armv7em -x c -E -dM %s -o - | FileCheck --check-prefix=ARM-DARWIN-NO-EABI %s
// RUN: %clang -target thumbv7-apple-darwin-eabi -arch armv7 -x c -E -dM %s -o - | FileCheck --check-prefix=ARM-DARWIN-NO-EABI %s
// ARM-DARWIN-NO-EABI-NOT: #define __ARM_EABI__ 1

// Check that -mhwdiv works properly for targets which don't have the hwdiv feature enabled by default.

// RUN: %clang -target arm -mhwdiv=arm -x c -E -dM %s -o - | FileCheck --check-prefix=ARMHWDIV-ARM %s
// ARMHWDIV-ARM:#define __ARM_ARCH_EXT_IDIV__ 1

// RUN: %clang -target arm -mthumb -mhwdiv=thumb -x c -E -dM %s -o - | FileCheck --check-prefix=THUMBHWDIV-THUMB %s
// THUMBHWDIV-THUMB:#define __ARM_ARCH_EXT_IDIV__ 1

// RUN: %clang -target arm -x c -E -dM %s -o - | FileCheck --check-prefix=ARM-FALSE %s
// ARM-FALSE-NOT:#define __ARM_ARCH_EXT_IDIV__

// RUN: %clang -target arm -mthumb -x c -E -dM %s -o - | FileCheck --check-prefix=THUMB-FALSE %s
// THUMB-FALSE-NOT:#define __ARM_ARCH_EXT_IDIV__

// RUN: %clang -target arm -mhwdiv=thumb -x c -E -dM %s -o - | FileCheck --check-prefix=THUMBHWDIV-ARM-FALSE %s
// THUMBHWDIV-ARM-FALSE-NOT:#define __ARM_ARCH_EXT_IDIV__

// RUN: %clang -target arm -mthumb -mhwdiv=arm -x c -E -dM %s -o - | FileCheck --check-prefix=ARMHWDIV-THUMB-FALSE %s
// ARMHWDIV-THUMB-FALSE-NOT:#define __ARM_ARCH_EXT_IDIV__

// RUN: %clang_cc1 -E -dM -ffreestanding -triple=armv8-none-none < /dev/null | FileCheck -check-prefix ARMv8 %s
// ARMv8: #define __THUMB_INTERWORK__ 1
// ARMv8-NOT: #define __thumb2__

// RUN: %clang_cc1 -E -dM -ffreestanding -triple=thumbv8 < /dev/null | FileCheck -check-prefix Thumbv8 %s
// Thumbv8: #define __THUMB_INTERWORK__ 1
// Thumbv8: #define __thumb2__

// RUN: %clang_cc1 -E -dM -ffreestanding -triple=thumbv5 < /dev/null | FileCheck -check-prefix Thumbv5 %s
// Thumbv5: #define __THUMB_INTERWORK__ 1
// Thumbv5-NOT: #define __thumb2__

// RUN: %clang_cc1 -E -dM -ffreestanding -triple=thumbv6t2 < /dev/null | FileCheck -check-prefix Thumbv6t2 %s
// Thumbv6t2: #define __THUMB_INTERWORK__ 1
// Thumbv6t2: #define __thumb2__

// RUN: %clang_cc1 -E -dM -ffreestanding -triple=thumbv7 < /dev/null | FileCheck -check-prefix Thumbv7 %s
// Thumbv7: #define __THUMB_INTERWORK__ 1
// Thumbv7: #define __thumb2__

//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=i386-none-none < /dev/null | FileCheck -check-prefix I386 %s
//
// I386-NOT:#define _LP64
// I386:#define __BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__
// I386:#define __CHAR16_TYPE__ unsigned short
// I386:#define __CHAR32_TYPE__ unsigned int
// I386:#define __CHAR_BIT__ 8
// I386:#define __DBL_DENORM_MIN__ 4.9406564584124654e-324
// I386:#define __DBL_DIG__ 15
// I386:#define __DBL_EPSILON__ 2.2204460492503131e-16
// I386:#define __DBL_HAS_DENORM__ 1
// I386:#define __DBL_HAS_INFINITY__ 1
// I386:#define __DBL_HAS_QUIET_NAN__ 1
// I386:#define __DBL_MANT_DIG__ 53
// I386:#define __DBL_MAX_10_EXP__ 308
// I386:#define __DBL_MAX_EXP__ 1024
// I386:#define __DBL_MAX__ 1.7976931348623157e+308
// I386:#define __DBL_MIN_10_EXP__ (-307)
// I386:#define __DBL_MIN_EXP__ (-1021)
// I386:#define __DBL_MIN__ 2.2250738585072014e-308
// I386:#define __DECIMAL_DIG__ 21
// I386:#define __FLT_DENORM_MIN__ 1.40129846e-45F
// I386:#define __FLT_DIG__ 6
// I386:#define __FLT_EPSILON__ 1.19209290e-7F
// I386:#define __FLT_EVAL_METHOD__ 2
// I386:#define __FLT_HAS_DENORM__ 1
// I386:#define __FLT_HAS_INFINITY__ 1
// I386:#define __FLT_HAS_QUIET_NAN__ 1
// I386:#define __FLT_MANT_DIG__ 24
// I386:#define __FLT_MAX_10_EXP__ 38
// I386:#define __FLT_MAX_EXP__ 128
// I386:#define __FLT_MAX__ 3.40282347e+38F
// I386:#define __FLT_MIN_10_EXP__ (-37)
// I386:#define __FLT_MIN_EXP__ (-125)
// I386:#define __FLT_MIN__ 1.17549435e-38F
// I386:#define __FLT_RADIX__ 2
// I386:#define __INT16_TYPE__ short
// I386:#define __INT32_TYPE__ int
// I386:#define __INT64_C_SUFFIX__ LL
// I386:#define __INT64_TYPE__ long long int
// I386:#define __INT8_TYPE__ char
// I386:#define __INTMAX_MAX__ 9223372036854775807LL
// I386:#define __INTMAX_TYPE__ long long int
// I386:#define __INTMAX_WIDTH__ 64
// I386:#define __INTPTR_TYPE__ int
// I386:#define __INTPTR_WIDTH__ 32
// I386:#define __INT_MAX__ 2147483647
// I386:#define __LDBL_DENORM_MIN__ 3.64519953188247460253e-4951L
// I386:#define __LDBL_DIG__ 18
// I386:#define __LDBL_EPSILON__ 1.08420217248550443401e-19L
// I386:#define __LDBL_HAS_DENORM__ 1
// I386:#define __LDBL_HAS_INFINITY__ 1
// I386:#define __LDBL_HAS_QUIET_NAN__ 1
// I386:#define __LDBL_MANT_DIG__ 64
// I386:#define __LDBL_MAX_10_EXP__ 4932
// I386:#define __LDBL_MAX_EXP__ 16384
// I386:#define __LDBL_MAX__ 1.18973149535723176502e+4932L
// I386:#define __LDBL_MIN_10_EXP__ (-4931)
// I386:#define __LDBL_MIN_EXP__ (-16381)
// I386:#define __LDBL_MIN__ 3.36210314311209350626e-4932L
// I386:#define __LITTLE_ENDIAN__ 1
// I386:#define __LONG_LONG_MAX__ 9223372036854775807LL
// I386:#define __LONG_MAX__ 2147483647L
// I386-NOT:#define __LP64__
// I386:#define __NO_MATH_INLINES 1
// I386:#define __POINTER_WIDTH__ 32
// I386:#define __PTRDIFF_TYPE__ int
// I386:#define __PTRDIFF_WIDTH__ 32
// I386:#define __REGISTER_PREFIX__ 
// I386:#define __SCHAR_MAX__ 127
// I386:#define __SHRT_MAX__ 32767
// I386:#define __SIG_ATOMIC_WIDTH__ 32
// I386:#define __SIZEOF_DOUBLE__ 8
// I386:#define __SIZEOF_FLOAT__ 4
// I386:#define __SIZEOF_INT__ 4
// I386:#define __SIZEOF_LONG_DOUBLE__ 12
// I386:#define __SIZEOF_LONG_LONG__ 8
// I386:#define __SIZEOF_LONG__ 4
// I386:#define __SIZEOF_POINTER__ 4
// I386:#define __SIZEOF_PTRDIFF_T__ 4
// I386:#define __SIZEOF_SHORT__ 2
// I386:#define __SIZEOF_SIZE_T__ 4
// I386:#define __SIZEOF_WCHAR_T__ 4
// I386:#define __SIZEOF_WINT_T__ 4
// I386:#define __SIZE_MAX__ 4294967295U
// I386:#define __SIZE_TYPE__ unsigned int
// I386:#define __SIZE_WIDTH__ 32
// I386:#define __UINTMAX_TYPE__ long long unsigned int
// I386:#define __USER_LABEL_PREFIX__ _
// I386:#define __WCHAR_MAX__ 2147483647
// I386:#define __WCHAR_TYPE__ int
// I386:#define __WCHAR_WIDTH__ 32
// I386:#define __WINT_TYPE__ int
// I386:#define __WINT_WIDTH__ 32
// I386:#define __i386 1
// I386:#define __i386__ 1
// I386:#define i386 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=i386-pc-linux-gnu -target-cpu pentium4 < /dev/null | FileCheck -check-prefix I386-LINUX %s
//
// I386-LINUX-NOT:#define _LP64
// I386-LINUX:#define __BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__
// I386-LINUX:#define __CHAR16_TYPE__ unsigned short
// I386-LINUX:#define __CHAR32_TYPE__ unsigned int
// I386-LINUX:#define __CHAR_BIT__ 8
// I386-LINUX:#define __DBL_DENORM_MIN__ 4.9406564584124654e-324
// I386-LINUX:#define __DBL_DIG__ 15
// I386-LINUX:#define __DBL_EPSILON__ 2.2204460492503131e-16
// I386-LINUX:#define __DBL_HAS_DENORM__ 1
// I386-LINUX:#define __DBL_HAS_INFINITY__ 1
// I386-LINUX:#define __DBL_HAS_QUIET_NAN__ 1
// I386-LINUX:#define __DBL_MANT_DIG__ 53
// I386-LINUX:#define __DBL_MAX_10_EXP__ 308
// I386-LINUX:#define __DBL_MAX_EXP__ 1024
// I386-LINUX:#define __DBL_MAX__ 1.7976931348623157e+308
// I386-LINUX:#define __DBL_MIN_10_EXP__ (-307)
// I386-LINUX:#define __DBL_MIN_EXP__ (-1021)
// I386-LINUX:#define __DBL_MIN__ 2.2250738585072014e-308
// I386-LINUX:#define __DECIMAL_DIG__ 21
// I386-LINUX:#define __FLT_DENORM_MIN__ 1.40129846e-45F
// I386-LINUX:#define __FLT_DIG__ 6
// I386-LINUX:#define __FLT_EPSILON__ 1.19209290e-7F
// I386-LINUX:#define __FLT_EVAL_METHOD__ 0
// I386-LINUX:#define __FLT_HAS_DENORM__ 1
// I386-LINUX:#define __FLT_HAS_INFINITY__ 1
// I386-LINUX:#define __FLT_HAS_QUIET_NAN__ 1
// I386-LINUX:#define __FLT_MANT_DIG__ 24
// I386-LINUX:#define __FLT_MAX_10_EXP__ 38
// I386-LINUX:#define __FLT_MAX_EXP__ 128
// I386-LINUX:#define __FLT_MAX__ 3.40282347e+38F
// I386-LINUX:#define __FLT_MIN_10_EXP__ (-37)
// I386-LINUX:#define __FLT_MIN_EXP__ (-125)
// I386-LINUX:#define __FLT_MIN__ 1.17549435e-38F
// I386-LINUX:#define __FLT_RADIX__ 2
// I386-LINUX:#define __INT16_TYPE__ short
// I386-LINUX:#define __INT32_TYPE__ int
// I386-LINUX:#define __INT64_C_SUFFIX__ LL
// I386-LINUX:#define __INT64_TYPE__ long long int
// I386-LINUX:#define __INT8_TYPE__ char
// I386-LINUX:#define __INTMAX_MAX__ 9223372036854775807LL
// I386-LINUX:#define __INTMAX_TYPE__ long long int
// I386-LINUX:#define __INTMAX_WIDTH__ 64
// I386-LINUX:#define __INTPTR_TYPE__ int
// I386-LINUX:#define __INTPTR_WIDTH__ 32
// I386-LINUX:#define __INT_MAX__ 2147483647
// I386-LINUX:#define __LDBL_DENORM_MIN__ 3.64519953188247460253e-4951L
// I386-LINUX:#define __LDBL_DIG__ 18
// I386-LINUX:#define __LDBL_EPSILON__ 1.08420217248550443401e-19L
// I386-LINUX:#define __LDBL_HAS_DENORM__ 1
// I386-LINUX:#define __LDBL_HAS_INFINITY__ 1
// I386-LINUX:#define __LDBL_HAS_QUIET_NAN__ 1
// I386-LINUX:#define __LDBL_MANT_DIG__ 64
// I386-LINUX:#define __LDBL_MAX_10_EXP__ 4932
// I386-LINUX:#define __LDBL_MAX_EXP__ 16384
// I386-LINUX:#define __LDBL_MAX__ 1.18973149535723176502e+4932L
// I386-LINUX:#define __LDBL_MIN_10_EXP__ (-4931)
// I386-LINUX:#define __LDBL_MIN_EXP__ (-16381)
// I386-LINUX:#define __LDBL_MIN__ 3.36210314311209350626e-4932L
// I386-LINUX:#define __LITTLE_ENDIAN__ 1
// I386-LINUX:#define __LONG_LONG_MAX__ 9223372036854775807LL
// I386-LINUX:#define __LONG_MAX__ 2147483647L
// I386-LINUX-NOT:#define __LP64__
// I386-LINUX:#define __NO_MATH_INLINES 1
// I386-LINUX:#define __POINTER_WIDTH__ 32
// I386-LINUX:#define __PTRDIFF_TYPE__ int
// I386-LINUX:#define __PTRDIFF_WIDTH__ 32
// I386-LINUX:#define __REGISTER_PREFIX__ 
// I386-LINUX:#define __SCHAR_MAX__ 127
// I386-LINUX:#define __SHRT_MAX__ 32767
// I386-LINUX:#define __SIG_ATOMIC_WIDTH__ 32
// I386-LINUX:#define __SIZEOF_DOUBLE__ 8
// I386-LINUX:#define __SIZEOF_FLOAT__ 4
// I386-LINUX:#define __SIZEOF_INT__ 4
// I386-LINUX:#define __SIZEOF_LONG_DOUBLE__ 12
// I386-LINUX:#define __SIZEOF_LONG_LONG__ 8
// I386-LINUX:#define __SIZEOF_LONG__ 4
// I386-LINUX:#define __SIZEOF_POINTER__ 4
// I386-LINUX:#define __SIZEOF_PTRDIFF_T__ 4
// I386-LINUX:#define __SIZEOF_SHORT__ 2
// I386-LINUX:#define __SIZEOF_SIZE_T__ 4
// I386-LINUX:#define __SIZEOF_WCHAR_T__ 4
// I386-LINUX:#define __SIZEOF_WINT_T__ 4
// I386-LINUX:#define __SIZE_MAX__ 4294967295U
// I386-LINUX:#define __SIZE_TYPE__ unsigned int
// I386-LINUX:#define __SIZE_WIDTH__ 32
// I386-LINUX:#define __UINTMAX_TYPE__ long long unsigned int
// I386-LINUX:#define __USER_LABEL_PREFIX__
// I386-LINUX:#define __WCHAR_MAX__ 2147483647
// I386-LINUX:#define __WCHAR_TYPE__ int
// I386-LINUX:#define __WCHAR_WIDTH__ 32
// I386-LINUX:#define __WINT_TYPE__ unsigned int
// I386-LINUX:#define __WINT_WIDTH__ 32
// I386-LINUX:#define __i386 1
// I386-LINUX:#define __i386__ 1
// I386-LINUX:#define i386 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=i386-netbsd < /dev/null | FileCheck -check-prefix I386-NETBSD %s
//
// I386-NETBSD-NOT:#define _LP64
// I386-NETBSD:#define __BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__
// I386-NETBSD:#define __CHAR16_TYPE__ unsigned short
// I386-NETBSD:#define __CHAR32_TYPE__ unsigned int
// I386-NETBSD:#define __CHAR_BIT__ 8
// I386-NETBSD:#define __DBL_DENORM_MIN__ 4.9406564584124654e-324
// I386-NETBSD:#define __DBL_DIG__ 15
// I386-NETBSD:#define __DBL_EPSILON__ 2.2204460492503131e-16
// I386-NETBSD:#define __DBL_HAS_DENORM__ 1
// I386-NETBSD:#define __DBL_HAS_INFINITY__ 1
// I386-NETBSD:#define __DBL_HAS_QUIET_NAN__ 1
// I386-NETBSD:#define __DBL_MANT_DIG__ 53
// I386-NETBSD:#define __DBL_MAX_10_EXP__ 308
// I386-NETBSD:#define __DBL_MAX_EXP__ 1024
// I386-NETBSD:#define __DBL_MAX__ 1.7976931348623157e+308
// I386-NETBSD:#define __DBL_MIN_10_EXP__ (-307)
// I386-NETBSD:#define __DBL_MIN_EXP__ (-1021)
// I386-NETBSD:#define __DBL_MIN__ 2.2250738585072014e-308
// I386-NETBSD:#define __DECIMAL_DIG__ 21
// I386-NETBSD:#define __FLT_DENORM_MIN__ 1.40129846e-45F
// I386-NETBSD:#define __FLT_DIG__ 6
// I386-NETBSD:#define __FLT_EPSILON__ 1.19209290e-7F
// I386-NETBSD:#define __FLT_EVAL_METHOD__ 2
// I386-NETBSD:#define __FLT_HAS_DENORM__ 1
// I386-NETBSD:#define __FLT_HAS_INFINITY__ 1
// I386-NETBSD:#define __FLT_HAS_QUIET_NAN__ 1
// I386-NETBSD:#define __FLT_MANT_DIG__ 24
// I386-NETBSD:#define __FLT_MAX_10_EXP__ 38
// I386-NETBSD:#define __FLT_MAX_EXP__ 128
// I386-NETBSD:#define __FLT_MAX__ 3.40282347e+38F
// I386-NETBSD:#define __FLT_MIN_10_EXP__ (-37)
// I386-NETBSD:#define __FLT_MIN_EXP__ (-125)
// I386-NETBSD:#define __FLT_MIN__ 1.17549435e-38F
// I386-NETBSD:#define __FLT_RADIX__ 2
// I386-NETBSD:#define __INT16_TYPE__ short
// I386-NETBSD:#define __INT32_TYPE__ int
// I386-NETBSD:#define __INT64_C_SUFFIX__ LL
// I386-NETBSD:#define __INT64_TYPE__ long long int
// I386-NETBSD:#define __INT8_TYPE__ char
// I386-NETBSD:#define __INTMAX_MAX__ 9223372036854775807LL
// I386-NETBSD:#define __INTMAX_TYPE__ long long int
// I386-NETBSD:#define __INTMAX_WIDTH__ 64
// I386-NETBSD:#define __INTPTR_TYPE__ int
// I386-NETBSD:#define __INTPTR_WIDTH__ 32
// I386-NETBSD:#define __INT_MAX__ 2147483647
// I386-NETBSD:#define __LDBL_DENORM_MIN__ 3.64519953188247460253e-4951L
// I386-NETBSD:#define __LDBL_DIG__ 18
// I386-NETBSD:#define __LDBL_EPSILON__ 1.08420217248550443401e-19L
// I386-NETBSD:#define __LDBL_HAS_DENORM__ 1
// I386-NETBSD:#define __LDBL_HAS_INFINITY__ 1
// I386-NETBSD:#define __LDBL_HAS_QUIET_NAN__ 1
// I386-NETBSD:#define __LDBL_MANT_DIG__ 64
// I386-NETBSD:#define __LDBL_MAX_10_EXP__ 4932
// I386-NETBSD:#define __LDBL_MAX_EXP__ 16384
// I386-NETBSD:#define __LDBL_MAX__ 1.18973149535723176502e+4932L
// I386-NETBSD:#define __LDBL_MIN_10_EXP__ (-4931)
// I386-NETBSD:#define __LDBL_MIN_EXP__ (-16381)
// I386-NETBSD:#define __LDBL_MIN__ 3.36210314311209350626e-4932L
// I386-NETBSD:#define __LITTLE_ENDIAN__ 1
// I386-NETBSD:#define __LONG_LONG_MAX__ 9223372036854775807LL
// I386-NETBSD:#define __LONG_MAX__ 2147483647L
// I386-NETBSD-NOT:#define __LP64__
// I386-NETBSD:#define __NO_MATH_INLINES 1
// I386-NETBSD:#define __POINTER_WIDTH__ 32
// I386-NETBSD:#define __PTRDIFF_TYPE__ int
// I386-NETBSD:#define __PTRDIFF_WIDTH__ 32
// I386-NETBSD:#define __REGISTER_PREFIX__ 
// I386-NETBSD:#define __SCHAR_MAX__ 127
// I386-NETBSD:#define __SHRT_MAX__ 32767
// I386-NETBSD:#define __SIG_ATOMIC_WIDTH__ 32
// I386-NETBSD:#define __SIZEOF_DOUBLE__ 8
// I386-NETBSD:#define __SIZEOF_FLOAT__ 4
// I386-NETBSD:#define __SIZEOF_INT__ 4
// I386-NETBSD:#define __SIZEOF_LONG_DOUBLE__ 12
// I386-NETBSD:#define __SIZEOF_LONG_LONG__ 8
// I386-NETBSD:#define __SIZEOF_LONG__ 4
// I386-NETBSD:#define __SIZEOF_POINTER__ 4
// I386-NETBSD:#define __SIZEOF_PTRDIFF_T__ 4
// I386-NETBSD:#define __SIZEOF_SHORT__ 2
// I386-NETBSD:#define __SIZEOF_SIZE_T__ 4
// I386-NETBSD:#define __SIZEOF_WCHAR_T__ 4
// I386-NETBSD:#define __SIZEOF_WINT_T__ 4
// I386-NETBSD:#define __SIZE_MAX__ 4294967295U
// I386-NETBSD:#define __SIZE_TYPE__ unsigned int
// I386-NETBSD:#define __SIZE_WIDTH__ 32
// I386-NETBSD:#define __UINTMAX_TYPE__ long long unsigned int
// I386-NETBSD:#define __USER_LABEL_PREFIX__
// I386-NETBSD:#define __WCHAR_MAX__ 2147483647
// I386-NETBSD:#define __WCHAR_TYPE__ int
// I386-NETBSD:#define __WCHAR_WIDTH__ 32
// I386-NETBSD:#define __WINT_TYPE__ int
// I386-NETBSD:#define __WINT_WIDTH__ 32
// I386-NETBSD:#define __i386 1
// I386-NETBSD:#define __i386__ 1
// I386-NETBSD:#define i386 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=i386-netbsd -target-feature +sse2 < /dev/null | FileCheck -check-prefix I386-NETBSD-SSE %s
// I386-NETBSD-SSE:#define __FLT_EVAL_METHOD__ 0
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=i386-netbsd6  < /dev/null | FileCheck -check-prefix I386-NETBSD6 %s
// I386-NETBSD6:#define __FLT_EVAL_METHOD__ 1
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=i386-netbsd6 -target-feature +sse2 < /dev/null | FileCheck -check-prefix I386-NETBSD6-SSE %s
// I386-NETBSD6-SSE:#define __FLT_EVAL_METHOD__ 1

//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=mips-none-none < /dev/null | FileCheck -check-prefix MIPS32BE %s
//
// MIPS32BE:#define MIPSEB 1
// MIPS32BE:#define _ABIO32 1
// MIPS32BE-NOT:#define _LP64
// MIPS32BE:#define _MIPSEB 1
// MIPS32BE:#define _MIPS_ARCH "mips32r2"
// MIPS32BE:#define _MIPS_ARCH_MIPS32R2 1
// MIPS32BE:#define _MIPS_FPSET 16
// MIPS32BE:#define _MIPS_SIM _ABIO32
// MIPS32BE:#define _MIPS_SZINT 32
// MIPS32BE:#define _MIPS_SZLONG 32
// MIPS32BE:#define _MIPS_SZPTR 32
// MIPS32BE:#define __BYTE_ORDER__ __ORDER_BIG_ENDIAN__
// MIPS32BE:#define __CHAR16_TYPE__ unsigned short
// MIPS32BE:#define __CHAR32_TYPE__ unsigned int
// MIPS32BE:#define __CHAR_BIT__ 8
// MIPS32BE:#define __CONSTANT_CFSTRINGS__ 1
// MIPS32BE:#define __DBL_DENORM_MIN__ 4.9406564584124654e-324
// MIPS32BE:#define __DBL_DIG__ 15
// MIPS32BE:#define __DBL_EPSILON__ 2.2204460492503131e-16
// MIPS32BE:#define __DBL_HAS_DENORM__ 1
// MIPS32BE:#define __DBL_HAS_INFINITY__ 1
// MIPS32BE:#define __DBL_HAS_QUIET_NAN__ 1
// MIPS32BE:#define __DBL_MANT_DIG__ 53
// MIPS32BE:#define __DBL_MAX_10_EXP__ 308
// MIPS32BE:#define __DBL_MAX_EXP__ 1024
// MIPS32BE:#define __DBL_MAX__ 1.7976931348623157e+308
// MIPS32BE:#define __DBL_MIN_10_EXP__ (-307)
// MIPS32BE:#define __DBL_MIN_EXP__ (-1021)
// MIPS32BE:#define __DBL_MIN__ 2.2250738585072014e-308
// MIPS32BE:#define __DECIMAL_DIG__ 17
// MIPS32BE:#define __FLT_DENORM_MIN__ 1.40129846e-45F
// MIPS32BE:#define __FLT_DIG__ 6
// MIPS32BE:#define __FLT_EPSILON__ 1.19209290e-7F
// MIPS32BE:#define __FLT_EVAL_METHOD__ 0
// MIPS32BE:#define __FLT_HAS_DENORM__ 1
// MIPS32BE:#define __FLT_HAS_INFINITY__ 1
// MIPS32BE:#define __FLT_HAS_QUIET_NAN__ 1
// MIPS32BE:#define __FLT_MANT_DIG__ 24
// MIPS32BE:#define __FLT_MAX_10_EXP__ 38
// MIPS32BE:#define __FLT_MAX_EXP__ 128
// MIPS32BE:#define __FLT_MAX__ 3.40282347e+38F
// MIPS32BE:#define __FLT_MIN_10_EXP__ (-37)
// MIPS32BE:#define __FLT_MIN_EXP__ (-125)
// MIPS32BE:#define __FLT_MIN__ 1.17549435e-38F
// MIPS32BE:#define __FLT_RADIX__ 2
// MIPS32BE:#define __INT16_TYPE__ short
// MIPS32BE:#define __INT32_TYPE__ int
// MIPS32BE:#define __INT64_C_SUFFIX__ LL
// MIPS32BE:#define __INT64_TYPE__ long long int
// MIPS32BE:#define __INT8_TYPE__ char
// MIPS32BE:#define __INTMAX_MAX__ 9223372036854775807LL
// MIPS32BE:#define __INTMAX_TYPE__ long long int
// MIPS32BE:#define __INTMAX_WIDTH__ 64
// MIPS32BE:#define __INTPTR_TYPE__ long int
// MIPS32BE:#define __INTPTR_WIDTH__ 32
// MIPS32BE:#define __INT_MAX__ 2147483647
// MIPS32BE:#define __LDBL_DENORM_MIN__ 4.9406564584124654e-324L
// MIPS32BE:#define __LDBL_DIG__ 15
// MIPS32BE:#define __LDBL_EPSILON__ 2.2204460492503131e-16L
// MIPS32BE:#define __LDBL_HAS_DENORM__ 1
// MIPS32BE:#define __LDBL_HAS_INFINITY__ 1
// MIPS32BE:#define __LDBL_HAS_QUIET_NAN__ 1
// MIPS32BE:#define __LDBL_MANT_DIG__ 53
// MIPS32BE:#define __LDBL_MAX_10_EXP__ 308
// MIPS32BE:#define __LDBL_MAX_EXP__ 1024
// MIPS32BE:#define __LDBL_MAX__ 1.7976931348623157e+308L
// MIPS32BE:#define __LDBL_MIN_10_EXP__ (-307)
// MIPS32BE:#define __LDBL_MIN_EXP__ (-1021)
// MIPS32BE:#define __LDBL_MIN__ 2.2250738585072014e-308L
// MIPS32BE:#define __LONG_LONG_MAX__ 9223372036854775807LL
// MIPS32BE:#define __LONG_MAX__ 2147483647L
// MIPS32BE-NOT:#define __LP64__
// MIPS32BE:#define __MIPSEB 1
// MIPS32BE:#define __MIPSEB__ 1
// MIPS32BE:#define __POINTER_WIDTH__ 32
// MIPS32BE:#define __PRAGMA_REDEFINE_EXTNAME 1
// MIPS32BE:#define __PTRDIFF_TYPE__ int
// MIPS32BE:#define __PTRDIFF_WIDTH__ 32
// MIPS32BE:#define __REGISTER_PREFIX__ 
// MIPS32BE:#define __SCHAR_MAX__ 127
// MIPS32BE:#define __SHRT_MAX__ 32767
// MIPS32BE:#define __SIG_ATOMIC_WIDTH__ 32
// MIPS32BE:#define __SIZEOF_DOUBLE__ 8
// MIPS32BE:#define __SIZEOF_FLOAT__ 4
// MIPS32BE:#define __SIZEOF_INT__ 4
// MIPS32BE:#define __SIZEOF_LONG_DOUBLE__ 8
// MIPS32BE:#define __SIZEOF_LONG_LONG__ 8
// MIPS32BE:#define __SIZEOF_LONG__ 4
// MIPS32BE:#define __SIZEOF_POINTER__ 4
// MIPS32BE:#define __SIZEOF_PTRDIFF_T__ 4
// MIPS32BE:#define __SIZEOF_SHORT__ 2
// MIPS32BE:#define __SIZEOF_SIZE_T__ 4
// MIPS32BE:#define __SIZEOF_WCHAR_T__ 4
// MIPS32BE:#define __SIZEOF_WINT_T__ 4
// MIPS32BE:#define __SIZE_MAX__ 4294967295U
// MIPS32BE:#define __SIZE_TYPE__ unsigned int
// MIPS32BE:#define __SIZE_WIDTH__ 32
// MIPS32BE:#define __STDC_HOSTED__ 0
// MIPS32BE:#define __STDC_VERSION__ 199901L
// MIPS32BE:#define __STDC__ 1
// MIPS32BE:#define __UINTMAX_TYPE__ long long unsigned int
// MIPS32BE:#define __USER_LABEL_PREFIX__ _
// MIPS32BE:#define __WCHAR_MAX__ 2147483647
// MIPS32BE:#define __WCHAR_TYPE__ int
// MIPS32BE:#define __WCHAR_WIDTH__ 32
// MIPS32BE:#define __WINT_TYPE__ int
// MIPS32BE:#define __WINT_WIDTH__ 32
// MIPS32BE:#define __clang__ 1
// MIPS32BE:#define __llvm__ 1
// MIPS32BE:#define __mips 32
// MIPS32BE:#define __mips__ 1
// MIPS32BE:#define __mips_fpr 32
// MIPS32BE:#define __mips_hard_float 1
// MIPS32BE:#define __mips_o32 1
// MIPS32BE:#define _mips 1
// MIPS32BE:#define mips 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=mipsel-none-none < /dev/null | FileCheck -check-prefix MIPS32EL %s
//
// MIPS32EL:#define MIPSEL 1
// MIPS32EL:#define _ABIO32 1
// MIPS32EL-NOT:#define _LP64
// MIPS32EL:#define _MIPSEL 1
// MIPS32EL:#define _MIPS_ARCH "mips32r2"
// MIPS32EL:#define _MIPS_ARCH_MIPS32R2 1
// MIPS32EL:#define _MIPS_FPSET 16
// MIPS32EL:#define _MIPS_SIM _ABIO32
// MIPS32EL:#define _MIPS_SZINT 32
// MIPS32EL:#define _MIPS_SZLONG 32
// MIPS32EL:#define _MIPS_SZPTR 32
// MIPS32EL:#define __BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__
// MIPS32EL:#define __CHAR16_TYPE__ unsigned short
// MIPS32EL:#define __CHAR32_TYPE__ unsigned int
// MIPS32EL:#define __CHAR_BIT__ 8
// MIPS32EL:#define __CONSTANT_CFSTRINGS__ 1
// MIPS32EL:#define __DBL_DENORM_MIN__ 4.9406564584124654e-324
// MIPS32EL:#define __DBL_DIG__ 15
// MIPS32EL:#define __DBL_EPSILON__ 2.2204460492503131e-16
// MIPS32EL:#define __DBL_HAS_DENORM__ 1
// MIPS32EL:#define __DBL_HAS_INFINITY__ 1
// MIPS32EL:#define __DBL_HAS_QUIET_NAN__ 1
// MIPS32EL:#define __DBL_MANT_DIG__ 53
// MIPS32EL:#define __DBL_MAX_10_EXP__ 308
// MIPS32EL:#define __DBL_MAX_EXP__ 1024
// MIPS32EL:#define __DBL_MAX__ 1.7976931348623157e+308
// MIPS32EL:#define __DBL_MIN_10_EXP__ (-307)
// MIPS32EL:#define __DBL_MIN_EXP__ (-1021)
// MIPS32EL:#define __DBL_MIN__ 2.2250738585072014e-308
// MIPS32EL:#define __DECIMAL_DIG__ 17
// MIPS32EL:#define __FLT_DENORM_MIN__ 1.40129846e-45F
// MIPS32EL:#define __FLT_DIG__ 6
// MIPS32EL:#define __FLT_EPSILON__ 1.19209290e-7F
// MIPS32EL:#define __FLT_EVAL_METHOD__ 0
// MIPS32EL:#define __FLT_HAS_DENORM__ 1
// MIPS32EL:#define __FLT_HAS_INFINITY__ 1
// MIPS32EL:#define __FLT_HAS_QUIET_NAN__ 1
// MIPS32EL:#define __FLT_MANT_DIG__ 24
// MIPS32EL:#define __FLT_MAX_10_EXP__ 38
// MIPS32EL:#define __FLT_MAX_EXP__ 128
// MIPS32EL:#define __FLT_MAX__ 3.40282347e+38F
// MIPS32EL:#define __FLT_MIN_10_EXP__ (-37)
// MIPS32EL:#define __FLT_MIN_EXP__ (-125)
// MIPS32EL:#define __FLT_MIN__ 1.17549435e-38F
// MIPS32EL:#define __FLT_RADIX__ 2
// MIPS32EL:#define __INT16_TYPE__ short
// MIPS32EL:#define __INT32_TYPE__ int
// MIPS32EL:#define __INT64_C_SUFFIX__ LL
// MIPS32EL:#define __INT64_TYPE__ long long int
// MIPS32EL:#define __INT8_TYPE__ char
// MIPS32EL:#define __INTMAX_MAX__ 9223372036854775807LL
// MIPS32EL:#define __INTMAX_TYPE__ long long int
// MIPS32EL:#define __INTMAX_WIDTH__ 64
// MIPS32EL:#define __INTPTR_TYPE__ long int
// MIPS32EL:#define __INTPTR_WIDTH__ 32
// MIPS32EL:#define __INT_MAX__ 2147483647
// MIPS32EL:#define __LDBL_DENORM_MIN__ 4.9406564584124654e-324L
// MIPS32EL:#define __LDBL_DIG__ 15
// MIPS32EL:#define __LDBL_EPSILON__ 2.2204460492503131e-16L
// MIPS32EL:#define __LDBL_HAS_DENORM__ 1
// MIPS32EL:#define __LDBL_HAS_INFINITY__ 1
// MIPS32EL:#define __LDBL_HAS_QUIET_NAN__ 1
// MIPS32EL:#define __LDBL_MANT_DIG__ 53
// MIPS32EL:#define __LDBL_MAX_10_EXP__ 308
// MIPS32EL:#define __LDBL_MAX_EXP__ 1024
// MIPS32EL:#define __LDBL_MAX__ 1.7976931348623157e+308L
// MIPS32EL:#define __LDBL_MIN_10_EXP__ (-307)
// MIPS32EL:#define __LDBL_MIN_EXP__ (-1021)
// MIPS32EL:#define __LDBL_MIN__ 2.2250738585072014e-308L
// MIPS32EL:#define __LONG_LONG_MAX__ 9223372036854775807LL
// MIPS32EL:#define __LONG_MAX__ 2147483647L
// MIPS32EL-NOT:#define __LP64__
// MIPS32EL:#define __MIPSEL 1
// MIPS32EL:#define __MIPSEL__ 1
// MIPS32EL:#define __POINTER_WIDTH__ 32
// MIPS32EL:#define __PRAGMA_REDEFINE_EXTNAME 1
// MIPS32EL:#define __PTRDIFF_TYPE__ int
// MIPS32EL:#define __PTRDIFF_WIDTH__ 32
// MIPS32EL:#define __REGISTER_PREFIX__ 
// MIPS32EL:#define __SCHAR_MAX__ 127
// MIPS32EL:#define __SHRT_MAX__ 32767
// MIPS32EL:#define __SIG_ATOMIC_WIDTH__ 32
// MIPS32EL:#define __SIZEOF_DOUBLE__ 8
// MIPS32EL:#define __SIZEOF_FLOAT__ 4
// MIPS32EL:#define __SIZEOF_INT__ 4
// MIPS32EL:#define __SIZEOF_LONG_DOUBLE__ 8
// MIPS32EL:#define __SIZEOF_LONG_LONG__ 8
// MIPS32EL:#define __SIZEOF_LONG__ 4
// MIPS32EL:#define __SIZEOF_POINTER__ 4
// MIPS32EL:#define __SIZEOF_PTRDIFF_T__ 4
// MIPS32EL:#define __SIZEOF_SHORT__ 2
// MIPS32EL:#define __SIZEOF_SIZE_T__ 4
// MIPS32EL:#define __SIZEOF_WCHAR_T__ 4
// MIPS32EL:#define __SIZEOF_WINT_T__ 4
// MIPS32EL:#define __SIZE_MAX__ 4294967295U
// MIPS32EL:#define __SIZE_TYPE__ unsigned int
// MIPS32EL:#define __SIZE_WIDTH__ 32
// MIPS32EL:#define __UINTMAX_TYPE__ long long unsigned int
// MIPS32EL:#define __USER_LABEL_PREFIX__ _
// MIPS32EL:#define __WCHAR_MAX__ 2147483647
// MIPS32EL:#define __WCHAR_TYPE__ int
// MIPS32EL:#define __WCHAR_WIDTH__ 32
// MIPS32EL:#define __WINT_TYPE__ int
// MIPS32EL:#define __WINT_WIDTH__ 32
// MIPS32EL:#define __clang__ 1
// MIPS32EL:#define __llvm__ 1
// MIPS32EL:#define __mips 32
// MIPS32EL:#define __mips__ 1
// MIPS32EL:#define __mips_fpr 32
// MIPS32EL:#define __mips_hard_float 1
// MIPS32EL:#define __mips_o32 1
// MIPS32EL:#define _mips 1
// MIPS32EL:#define mips 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=mips64-none-none < /dev/null | FileCheck -check-prefix MIPS64BE %s
//
// MIPS64BE:#define MIPSEB 1
// MIPS64BE:#define _ABI64 3
// MIPS64BE:#define _LP64 1
// MIPS64BE:#define _MIPSEB 1
// MIPS64BE:#define _MIPS_ARCH "mips64r2"
// MIPS64BE:#define _MIPS_ARCH_MIPS64R2 1
// MIPS64BE:#define _MIPS_FPSET 32
// MIPS64BE:#define _MIPS_SIM _ABI64
// MIPS64BE:#define _MIPS_SZINT 32
// MIPS64BE:#define _MIPS_SZLONG 64
// MIPS64BE:#define _MIPS_SZPTR 64
// MIPS64BE:#define __BYTE_ORDER__ __ORDER_BIG_ENDIAN__
// MIPS64BE:#define __CHAR16_TYPE__ unsigned short
// MIPS64BE:#define __CHAR32_TYPE__ unsigned int
// MIPS64BE:#define __CHAR_BIT__ 8
// MIPS64BE:#define __CONSTANT_CFSTRINGS__ 1
// MIPS64BE:#define __DBL_DENORM_MIN__ 4.9406564584124654e-324
// MIPS64BE:#define __DBL_DIG__ 15
// MIPS64BE:#define __DBL_EPSILON__ 2.2204460492503131e-16
// MIPS64BE:#define __DBL_HAS_DENORM__ 1
// MIPS64BE:#define __DBL_HAS_INFINITY__ 1
// MIPS64BE:#define __DBL_HAS_QUIET_NAN__ 1
// MIPS64BE:#define __DBL_MANT_DIG__ 53
// MIPS64BE:#define __DBL_MAX_10_EXP__ 308
// MIPS64BE:#define __DBL_MAX_EXP__ 1024
// MIPS64BE:#define __DBL_MAX__ 1.7976931348623157e+308
// MIPS64BE:#define __DBL_MIN_10_EXP__ (-307)
// MIPS64BE:#define __DBL_MIN_EXP__ (-1021)
// MIPS64BE:#define __DBL_MIN__ 2.2250738585072014e-308
// MIPS64BE:#define __DECIMAL_DIG__ 36
// MIPS64BE:#define __FLT_DENORM_MIN__ 1.40129846e-45F
// MIPS64BE:#define __FLT_DIG__ 6
// MIPS64BE:#define __FLT_EPSILON__ 1.19209290e-7F
// MIPS64BE:#define __FLT_EVAL_METHOD__ 0
// MIPS64BE:#define __FLT_HAS_DENORM__ 1
// MIPS64BE:#define __FLT_HAS_INFINITY__ 1
// MIPS64BE:#define __FLT_HAS_QUIET_NAN__ 1
// MIPS64BE:#define __FLT_MANT_DIG__ 24
// MIPS64BE:#define __FLT_MAX_10_EXP__ 38
// MIPS64BE:#define __FLT_MAX_EXP__ 128
// MIPS64BE:#define __FLT_MAX__ 3.40282347e+38F
// MIPS64BE:#define __FLT_MIN_10_EXP__ (-37)
// MIPS64BE:#define __FLT_MIN_EXP__ (-125)
// MIPS64BE:#define __FLT_MIN__ 1.17549435e-38F
// MIPS64BE:#define __FLT_RADIX__ 2
// MIPS64BE:#define __INT16_TYPE__ short
// MIPS64BE:#define __INT32_TYPE__ int
// MIPS64BE:#define __INT64_C_SUFFIX__ LL
// MIPS64BE:#define __INT64_TYPE__ long long int
// MIPS64BE:#define __INT8_TYPE__ char
// MIPS64BE:#define __INTMAX_MAX__ 9223372036854775807LL
// MIPS64BE:#define __INTMAX_TYPE__ long long int
// MIPS64BE:#define __INTMAX_WIDTH__ 64
// MIPS64BE:#define __INTPTR_TYPE__ long int
// MIPS64BE:#define __INTPTR_WIDTH__ 64
// MIPS64BE:#define __INT_MAX__ 2147483647
// MIPS64BE:#define __LDBL_DENORM_MIN__ 6.47517511943802511092443895822764655e-4966L
// MIPS64BE:#define __LDBL_DIG__ 33
// MIPS64BE:#define __LDBL_EPSILON__ 1.92592994438723585305597794258492732e-34L
// MIPS64BE:#define __LDBL_HAS_DENORM__ 1
// MIPS64BE:#define __LDBL_HAS_INFINITY__ 1
// MIPS64BE:#define __LDBL_HAS_QUIET_NAN__ 1
// MIPS64BE:#define __LDBL_MANT_DIG__ 113
// MIPS64BE:#define __LDBL_MAX_10_EXP__ 4932
// MIPS64BE:#define __LDBL_MAX_EXP__ 16384
// MIPS64BE:#define __LDBL_MAX__ 1.18973149535723176508575932662800702e+4932L
// MIPS64BE:#define __LDBL_MIN_10_EXP__ (-4931)
// MIPS64BE:#define __LDBL_MIN_EXP__ (-16381)
// MIPS64BE:#define __LDBL_MIN__ 3.36210314311209350626267781732175260e-4932L
// MIPS64BE:#define __LONG_LONG_MAX__ 9223372036854775807LL
// MIPS64BE:#define __LONG_MAX__ 9223372036854775807L
// MIPS64BE:#define __LP64__ 1
// MIPS64BE:#define __MIPSEB 1
// MIPS64BE:#define __MIPSEB__ 1
// MIPS64BE:#define __POINTER_WIDTH__ 64
// MIPS64BE:#define __PRAGMA_REDEFINE_EXTNAME 1
// MIPS64BE:#define __PTRDIFF_TYPE__ long int
// MIPS64BE:#define __PTRDIFF_WIDTH__ 64
// MIPS64BE:#define __REGISTER_PREFIX__ 
// MIPS64BE:#define __SCHAR_MAX__ 127
// MIPS64BE:#define __SHRT_MAX__ 32767
// MIPS64BE:#define __SIG_ATOMIC_WIDTH__ 32
// MIPS64BE:#define __SIZEOF_DOUBLE__ 8
// MIPS64BE:#define __SIZEOF_FLOAT__ 4
// MIPS64BE:#define __SIZEOF_INT__ 4
// MIPS64BE:#define __SIZEOF_LONG_DOUBLE__ 16
// MIPS64BE:#define __SIZEOF_LONG_LONG__ 8
// MIPS64BE:#define __SIZEOF_LONG__ 8
// MIPS64BE:#define __SIZEOF_POINTER__ 8
// MIPS64BE:#define __SIZEOF_PTRDIFF_T__ 8
// MIPS64BE:#define __SIZEOF_SHORT__ 2
// MIPS64BE:#define __SIZEOF_SIZE_T__ 8
// MIPS64BE:#define __SIZEOF_WCHAR_T__ 4
// MIPS64BE:#define __SIZEOF_WINT_T__ 4
// MIPS64BE:#define __SIZE_MAX__ 18446744073709551615UL
// MIPS64BE:#define __SIZE_TYPE__ long unsigned int
// MIPS64BE:#define __SIZE_WIDTH__ 64
// MIPS64BE:#define __UINTMAX_TYPE__ long long unsigned int
// MIPS64BE:#define __USER_LABEL_PREFIX__ _
// MIPS64BE:#define __WCHAR_MAX__ 2147483647
// MIPS64BE:#define __WCHAR_TYPE__ int
// MIPS64BE:#define __WCHAR_WIDTH__ 32
// MIPS64BE:#define __WINT_TYPE__ int
// MIPS64BE:#define __WINT_WIDTH__ 32
// MIPS64BE:#define __clang__ 1
// MIPS64BE:#define __llvm__ 1
// MIPS64BE:#define __mips 64
// MIPS64BE:#define __mips64 1
// MIPS64BE:#define __mips64__ 1
// MIPS64BE:#define __mips__ 1
// MIPS64BE:#define __mips_fpr 64
// MIPS64BE:#define __mips_hard_float 1
// MIPS64BE:#define __mips_n64 1
// MIPS64BE:#define _mips 1
// MIPS64BE:#define mips 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=mips64el-none-none < /dev/null | FileCheck -check-prefix MIPS64EL %s
//
// MIPS64EL:#define MIPSEL 1
// MIPS64EL:#define _ABI64 3
// MIPS64EL:#define _LP64 1
// MIPS64EL:#define _MIPSEL 1
// MIPS64EL:#define _MIPS_ARCH "mips64r2"
// MIPS64EL:#define _MIPS_ARCH_MIPS64R2 1
// MIPS64EL:#define _MIPS_FPSET 32
// MIPS64EL:#define _MIPS_SIM _ABI64
// MIPS64EL:#define _MIPS_SZINT 32
// MIPS64EL:#define _MIPS_SZLONG 64
// MIPS64EL:#define _MIPS_SZPTR 64
// MIPS64EL:#define __BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__
// MIPS64EL:#define __CHAR16_TYPE__ unsigned short
// MIPS64EL:#define __CHAR32_TYPE__ unsigned int
// MIPS64EL:#define __CHAR_BIT__ 8
// MIPS64EL:#define __CONSTANT_CFSTRINGS__ 1
// MIPS64EL:#define __DBL_DENORM_MIN__ 4.9406564584124654e-324
// MIPS64EL:#define __DBL_DIG__ 15
// MIPS64EL:#define __DBL_EPSILON__ 2.2204460492503131e-16
// MIPS64EL:#define __DBL_HAS_DENORM__ 1
// MIPS64EL:#define __DBL_HAS_INFINITY__ 1
// MIPS64EL:#define __DBL_HAS_QUIET_NAN__ 1
// MIPS64EL:#define __DBL_MANT_DIG__ 53
// MIPS64EL:#define __DBL_MAX_10_EXP__ 308
// MIPS64EL:#define __DBL_MAX_EXP__ 1024
// MIPS64EL:#define __DBL_MAX__ 1.7976931348623157e+308
// MIPS64EL:#define __DBL_MIN_10_EXP__ (-307)
// MIPS64EL:#define __DBL_MIN_EXP__ (-1021)
// MIPS64EL:#define __DBL_MIN__ 2.2250738585072014e-308
// MIPS64EL:#define __DECIMAL_DIG__ 36
// MIPS64EL:#define __FLT_DENORM_MIN__ 1.40129846e-45F
// MIPS64EL:#define __FLT_DIG__ 6
// MIPS64EL:#define __FLT_EPSILON__ 1.19209290e-7F
// MIPS64EL:#define __FLT_EVAL_METHOD__ 0
// MIPS64EL:#define __FLT_HAS_DENORM__ 1
// MIPS64EL:#define __FLT_HAS_INFINITY__ 1
// MIPS64EL:#define __FLT_HAS_QUIET_NAN__ 1
// MIPS64EL:#define __FLT_MANT_DIG__ 24
// MIPS64EL:#define __FLT_MAX_10_EXP__ 38
// MIPS64EL:#define __FLT_MAX_EXP__ 128
// MIPS64EL:#define __FLT_MAX__ 3.40282347e+38F
// MIPS64EL:#define __FLT_MIN_10_EXP__ (-37)
// MIPS64EL:#define __FLT_MIN_EXP__ (-125)
// MIPS64EL:#define __FLT_MIN__ 1.17549435e-38F
// MIPS64EL:#define __FLT_RADIX__ 2
// MIPS64EL:#define __INT16_TYPE__ short
// MIPS64EL:#define __INT32_TYPE__ int
// MIPS64EL:#define __INT64_C_SUFFIX__ LL
// MIPS64EL:#define __INT64_TYPE__ long long int
// MIPS64EL:#define __INT8_TYPE__ char
// MIPS64EL:#define __INTMAX_MAX__ 9223372036854775807LL
// MIPS64EL:#define __INTMAX_TYPE__ long long int
// MIPS64EL:#define __INTMAX_WIDTH__ 64
// MIPS64EL:#define __INTPTR_TYPE__ long int
// MIPS64EL:#define __INTPTR_WIDTH__ 64
// MIPS64EL:#define __INT_MAX__ 2147483647
// MIPS64EL:#define __LDBL_DENORM_MIN__ 6.47517511943802511092443895822764655e-4966L
// MIPS64EL:#define __LDBL_DIG__ 33
// MIPS64EL:#define __LDBL_EPSILON__ 1.92592994438723585305597794258492732e-34L
// MIPS64EL:#define __LDBL_HAS_DENORM__ 1
// MIPS64EL:#define __LDBL_HAS_INFINITY__ 1
// MIPS64EL:#define __LDBL_HAS_QUIET_NAN__ 1
// MIPS64EL:#define __LDBL_MANT_DIG__ 113
// MIPS64EL:#define __LDBL_MAX_10_EXP__ 4932
// MIPS64EL:#define __LDBL_MAX_EXP__ 16384
// MIPS64EL:#define __LDBL_MAX__ 1.18973149535723176508575932662800702e+4932L
// MIPS64EL:#define __LDBL_MIN_10_EXP__ (-4931)
// MIPS64EL:#define __LDBL_MIN_EXP__ (-16381)
// MIPS64EL:#define __LDBL_MIN__ 3.36210314311209350626267781732175260e-4932L
// MIPS64EL:#define __LONG_LONG_MAX__ 9223372036854775807LL
// MIPS64EL:#define __LONG_MAX__ 9223372036854775807L
// MIPS64EL:#define __LP64__ 1
// MIPS64EL:#define __MIPSEL 1
// MIPS64EL:#define __MIPSEL__ 1
// MIPS64EL:#define __POINTER_WIDTH__ 64
// MIPS64EL:#define __PRAGMA_REDEFINE_EXTNAME 1
// MIPS64EL:#define __PTRDIFF_TYPE__ long int
// MIPS64EL:#define __PTRDIFF_WIDTH__ 64
// MIPS64EL:#define __REGISTER_PREFIX__ 
// MIPS64EL:#define __SCHAR_MAX__ 127
// MIPS64EL:#define __SHRT_MAX__ 32767
// MIPS64EL:#define __SIG_ATOMIC_WIDTH__ 32
// MIPS64EL:#define __SIZEOF_DOUBLE__ 8
// MIPS64EL:#define __SIZEOF_FLOAT__ 4
// MIPS64EL:#define __SIZEOF_INT__ 4
// MIPS64EL:#define __SIZEOF_LONG_DOUBLE__ 16
// MIPS64EL:#define __SIZEOF_LONG_LONG__ 8
// MIPS64EL:#define __SIZEOF_LONG__ 8
// MIPS64EL:#define __SIZEOF_POINTER__ 8
// MIPS64EL:#define __SIZEOF_PTRDIFF_T__ 8
// MIPS64EL:#define __SIZEOF_SHORT__ 2
// MIPS64EL:#define __SIZEOF_SIZE_T__ 8
// MIPS64EL:#define __SIZEOF_WCHAR_T__ 4
// MIPS64EL:#define __SIZEOF_WINT_T__ 4
// MIPS64EL:#define __SIZE_MAX__ 18446744073709551615UL
// MIPS64EL:#define __SIZE_TYPE__ long unsigned int
// MIPS64EL:#define __SIZE_WIDTH__ 64
// MIPS64EL:#define __UINTMAX_TYPE__ long long unsigned int
// MIPS64EL:#define __USER_LABEL_PREFIX__ _
// MIPS64EL:#define __WCHAR_MAX__ 2147483647
// MIPS64EL:#define __WCHAR_TYPE__ int
// MIPS64EL:#define __WCHAR_WIDTH__ 32
// MIPS64EL:#define __WINT_TYPE__ int
// MIPS64EL:#define __WINT_WIDTH__ 32
// MIPS64EL:#define __clang__ 1
// MIPS64EL:#define __llvm__ 1
// MIPS64EL:#define __mips 64
// MIPS64EL:#define __mips64 1
// MIPS64EL:#define __mips64__ 1
// MIPS64EL:#define __mips__ 1
// MIPS64EL:#define __mips_fpr 64
// MIPS64EL:#define __mips_hard_float 1
// MIPS64EL:#define __mips_n64 1
// MIPS64EL:#define _mips 1
// MIPS64EL:#define mips 1
//
// Check MIPS arch macros
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=mips-none-none \
// RUN:            < /dev/null \
// RUN:   | FileCheck -check-prefix MIPS-ARCH-DEF32 %s
//
// MIPS-ARCH-DEF32:#define _MIPS_ARCH "mips32r2"
// MIPS-ARCH-DEF32:#define _MIPS_ARCH_MIPS32R2 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=mips-none-nones \
// RUN:            -target-cpu mips32 < /dev/null \
// RUN:   | FileCheck -check-prefix MIPS-ARCH-32 %s
//
// MIPS-ARCH-32:#define _MIPS_ARCH "mips32"
// MIPS-ARCH-32:#define _MIPS_ARCH_MIPS32 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=mips-none-none \
// RUN:            -target-cpu mips32r2 < /dev/null \
// RUN:   | FileCheck -check-prefix MIPS-ARCH-32R2 %s
//
// MIPS-ARCH-32R2:#define _MIPS_ARCH "mips32r2"
// MIPS-ARCH-32R2:#define _MIPS_ARCH_MIPS32R2 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=mips64-none-none \
// RUN:            < /dev/null \
// RUN:   | FileCheck -check-prefix MIPS-ARCH-DEF64 %s
//
// MIPS-ARCH-DEF64:#define _MIPS_ARCH "mips64r2"
// MIPS-ARCH-DEF64:#define _MIPS_ARCH_MIPS64R2 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=mips64-none-none \
// RUN:            -target-cpu mips64 < /dev/null \
// RUN:   | FileCheck -check-prefix MIPS-ARCH-64 %s
//
// MIPS-ARCH-64:#define _MIPS_ARCH "mips64"
// MIPS-ARCH-64:#define _MIPS_ARCH_MIPS64 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=mips64-none-none \
// RUN:            -target-cpu mips64r2 < /dev/null \
// RUN:   | FileCheck -check-prefix MIPS-ARCH-64R2 %s
//
// MIPS-ARCH-64R2:#define _MIPS_ARCH "mips64r2"
// MIPS-ARCH-64R2:#define _MIPS_ARCH_MIPS64R2 1
//
// Check MIPS float ABI macros
//
// RUN: %clang_cc1 -E -dM -ffreestanding \
// RUN:   -triple=mips-none-none < /dev/null \
// RUN:   | FileCheck -check-prefix MIPS-FABI-HARD %s
// MIPS-FABI-HARD:#define __mips_hard_float 1
//
// RUN: %clang_cc1 -target-feature +soft-float -E -dM -ffreestanding \
// RUN:   -triple=mips-none-none < /dev/null \
// RUN:   | FileCheck -check-prefix MIPS-FABI-SOFT %s
// MIPS-FABI-SOFT:#define __mips_soft_float 1
//
// RUN: %clang_cc1 -target-feature +single-float -E -dM -ffreestanding \
// RUN:   -triple=mips-none-none < /dev/null \
// RUN:   | FileCheck -check-prefix MIPS-FABI-SINGLE %s
// MIPS-FABI-SINGLE:#define __mips_hard_float 1
// MIPS-FABI-SINGLE:#define __mips_single_float 1
//
// RUN: %clang_cc1 -target-feature +soft-float -target-feature +single-float \
// RUN:   -E -dM -ffreestanding -triple=mips-none-none < /dev/null \
// RUN:   | FileCheck -check-prefix MIPS-FABI-SINGLE-SOFT %s
// MIPS-FABI-SINGLE-SOFT:#define __mips_single_float 1
// MIPS-FABI-SINGLE-SOFT:#define __mips_soft_float 1
//
// Check MIPS features macros
//
// RUN: %clang_cc1 -target-feature +mips16 \
// RUN:   -E -dM -triple=mips-none-none < /dev/null \
// RUN:   | FileCheck -check-prefix MIPS16 %s
// MIPS16:#define __mips16 1
//
// RUN: %clang_cc1 -target-feature -mips16 \
// RUN:   -E -dM -triple=mips-none-none < /dev/null \
// RUN:   | FileCheck -check-prefix NOMIPS16 %s
// NOMIPS16-NOT:#define __mips16 1
//
// RUN: %clang_cc1 -target-feature +micromips \
// RUN:   -E -dM -triple=mips-none-none < /dev/null \
// RUN:   | FileCheck -check-prefix MICROMIPS %s
// MICROMIPS:#define __mips_micromips 1
//
// RUN: %clang_cc1 -target-feature -micromips \
// RUN:   -E -dM -triple=mips-none-none < /dev/null \
// RUN:   | FileCheck -check-prefix NOMICROMIPS %s
// NOMICROMIPS-NOT:#define __mips_micromips 1
//
// RUN: %clang_cc1 -target-feature +dsp \
// RUN:   -E -dM -triple=mips-none-none < /dev/null \
// RUN:   | FileCheck -check-prefix MIPS-DSP %s
// MIPS-DSP:#define __mips_dsp 1
// MIPS-DSP:#define __mips_dsp_rev 1
// MIPS-DSP-NOT:#define __mips_dspr2 1
//
// RUN: %clang_cc1 -target-feature +dspr2 \
// RUN:   -E -dM -triple=mips-none-none < /dev/null \
// RUN:   | FileCheck -check-prefix MIPS-DSPR2 %s
// MIPS-DSPR2:#define __mips_dsp 1
// MIPS-DSPR2:#define __mips_dsp_rev 2
// MIPS-DSPR2:#define __mips_dspr2 1
//
// RUN: %clang_cc1 -target-feature +msa \
// RUN:   -E -dM -triple=mips-none-none < /dev/null \
// RUN:   | FileCheck -check-prefix MIPS-MSA %s
// MIPS-MSA:#define __mips_msa 1
//
// RUN: %clang_cc1 -target-feature +nan2008 \
// RUN:   -E -dM -triple=mips-none-none < /dev/null \
// RUN:   | FileCheck -check-prefix MIPS-NAN2008 %s
// MIPS-NAN2008:#define __mips_nan2008 1
//
// RUN: %clang_cc1 -target-feature -fp64 \
// RUN:   -E -dM -triple=mips-none-none < /dev/null \
// RUN:   | FileCheck -check-prefix MIPS32-MFP32 %s
// MIPS32-MFP32:#define _MIPS_FPSET 16
// MIPS32-MFP32:#define __mips_fpr 32
//
// RUN: %clang_cc1 -target-feature +fp64 \
// RUN:   -E -dM -triple=mips-none-none < /dev/null \
// RUN:   | FileCheck -check-prefix MIPS32-MFP64 %s
// MIPS32-MFP64:#define _MIPS_FPSET 32
// MIPS32-MFP64:#define __mips_fpr 64
//
// RUN: %clang_cc1 -target-feature +single-float \
// RUN:   -E -dM -triple=mips-none-none < /dev/null \
// RUN:   | FileCheck -check-prefix MIPS32-MFP32SF %s
// MIPS32-MFP32SF:#define _MIPS_FPSET 32
// MIPS32-MFP32SF:#define __mips_fpr 32
//
// RUN: %clang_cc1 -target-feature +fp64 \
// RUN:   -E -dM -triple=mips64-none-none < /dev/null \
// RUN:   | FileCheck -check-prefix MIPS64-MFP64 %s
// MIPS64-MFP64:#define _MIPS_FPSET 32
// MIPS64-MFP64:#define __mips_fpr 64
//
// RUN: %clang_cc1 -target-feature -fp64 -target-feature +single-float \
// RUN:   -E -dM -triple=mips64-none-none < /dev/null \
// RUN:   | FileCheck -check-prefix MIPS64-NOMFP64 %s
// MIPS64-NOMFP64:#define _MIPS_FPSET 32
// MIPS64-NOMFP64:#define __mips_fpr 32
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=msp430-none-none < /dev/null | FileCheck -check-prefix MSP430 %s
//
// MSP430:#define MSP430 1
// MSP430-NOT:#define _LP64
// MSP430:#define __BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__
// MSP430:#define __CHAR16_TYPE__ unsigned short
// MSP430:#define __CHAR32_TYPE__ unsigned int
// MSP430:#define __CHAR_BIT__ 8
// MSP430:#define __DBL_DENORM_MIN__ 4.9406564584124654e-324
// MSP430:#define __DBL_DIG__ 15
// MSP430:#define __DBL_EPSILON__ 2.2204460492503131e-16
// MSP430:#define __DBL_HAS_DENORM__ 1
// MSP430:#define __DBL_HAS_INFINITY__ 1
// MSP430:#define __DBL_HAS_QUIET_NAN__ 1
// MSP430:#define __DBL_MANT_DIG__ 53
// MSP430:#define __DBL_MAX_10_EXP__ 308
// MSP430:#define __DBL_MAX_EXP__ 1024
// MSP430:#define __DBL_MAX__ 1.7976931348623157e+308
// MSP430:#define __DBL_MIN_10_EXP__ (-307)
// MSP430:#define __DBL_MIN_EXP__ (-1021)
// MSP430:#define __DBL_MIN__ 2.2250738585072014e-308
// MSP430:#define __DECIMAL_DIG__ 17
// MSP430:#define __FLT_DENORM_MIN__ 1.40129846e-45F
// MSP430:#define __FLT_DIG__ 6
// MSP430:#define __FLT_EPSILON__ 1.19209290e-7F
// MSP430:#define __FLT_EVAL_METHOD__ 0
// MSP430:#define __FLT_HAS_DENORM__ 1
// MSP430:#define __FLT_HAS_INFINITY__ 1
// MSP430:#define __FLT_HAS_QUIET_NAN__ 1
// MSP430:#define __FLT_MANT_DIG__ 24
// MSP430:#define __FLT_MAX_10_EXP__ 38
// MSP430:#define __FLT_MAX_EXP__ 128
// MSP430:#define __FLT_MAX__ 3.40282347e+38F
// MSP430:#define __FLT_MIN_10_EXP__ (-37)
// MSP430:#define __FLT_MIN_EXP__ (-125)
// MSP430:#define __FLT_MIN__ 1.17549435e-38F
// MSP430:#define __FLT_RADIX__ 2
// MSP430:#define __INT16_TYPE__ short
// MSP430:#define __INT32_C_SUFFIX__ L
// MSP430:#define __INT32_TYPE__ long int
// MSP430:#define __INT8_TYPE__ char
// MSP430:#define __INTMAX_MAX__ 9223372036854775807LL
// MSP430:#define __INTMAX_TYPE__ long long int
// MSP430:#define __INTMAX_WIDTH__ 64
// MSP430:#define __INTPTR_TYPE__ int
// MSP430:#define __INTPTR_WIDTH__ 16
// MSP430:#define __INT_MAX__ 32767
// MSP430:#define __LDBL_DENORM_MIN__ 4.9406564584124654e-324L
// MSP430:#define __LDBL_DIG__ 15
// MSP430:#define __LDBL_EPSILON__ 2.2204460492503131e-16L
// MSP430:#define __LDBL_HAS_DENORM__ 1
// MSP430:#define __LDBL_HAS_INFINITY__ 1
// MSP430:#define __LDBL_HAS_QUIET_NAN__ 1
// MSP430:#define __LDBL_MANT_DIG__ 53
// MSP430:#define __LDBL_MAX_10_EXP__ 308
// MSP430:#define __LDBL_MAX_EXP__ 1024
// MSP430:#define __LDBL_MAX__ 1.7976931348623157e+308L
// MSP430:#define __LDBL_MIN_10_EXP__ (-307)
// MSP430:#define __LDBL_MIN_EXP__ (-1021)
// MSP430:#define __LDBL_MIN__ 2.2250738585072014e-308L
// MSP430:#define __LONG_LONG_MAX__ 9223372036854775807LL
// MSP430:#define __LONG_MAX__ 2147483647L
// MSP430-NOT:#define __LP64__
// MSP430:#define __MSP430__ 1
// MSP430:#define __POINTER_WIDTH__ 16
// MSP430:#define __PTRDIFF_TYPE__ int
// MSP430:#define __PTRDIFF_WIDTH__ 16 
// MSP430:#define __SCHAR_MAX__ 127
// MSP430:#define __SHRT_MAX__ 32767
// MSP430:#define __SIG_ATOMIC_WIDTH__ 32
// MSP430:#define __SIZEOF_DOUBLE__ 8
// MSP430:#define __SIZEOF_FLOAT__ 4
// MSP430:#define __SIZEOF_INT__ 2
// MSP430:#define __SIZEOF_LONG_DOUBLE__ 8
// MSP430:#define __SIZEOF_LONG_LONG__ 8
// MSP430:#define __SIZEOF_LONG__ 4
// MSP430:#define __SIZEOF_POINTER__ 2
// MSP430:#define __SIZEOF_PTRDIFF_T__ 2
// MSP430:#define __SIZEOF_SHORT__ 2
// MSP430:#define __SIZEOF_SIZE_T__ 2
// MSP430:#define __SIZEOF_WCHAR_T__ 2
// MSP430:#define __SIZEOF_WINT_T__ 2
// MSP430:#define __SIZE_MAX__ 65535U
// MSP430:#define __SIZE_TYPE__ unsigned int
// MSP430:#define __SIZE_WIDTH__ 16
// MSP430:#define __UINTMAX_TYPE__ long long unsigned int
// MSP430:#define __USER_LABEL_PREFIX__ _
// MSP430:#define __WCHAR_MAX__ 32767
// MSP430:#define __WCHAR_TYPE__ int
// MSP430:#define __WCHAR_WIDTH__ 16
// MSP430:#define __WINT_TYPE__ int
// MSP430:#define __WINT_WIDTH__ 16
// MSP430:#define __clang__ 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=nvptx-none-none < /dev/null | FileCheck -check-prefix NVPTX32 %s
//
// NVPTX32-NOT:#define _LP64
// NVPTX32:#define __BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__
// NVPTX32:#define __CHAR16_TYPE__ unsigned short
// NVPTX32:#define __CHAR32_TYPE__ unsigned int
// NVPTX32:#define __CHAR_BIT__ 8
// NVPTX32:#define __CONSTANT_CFSTRINGS__ 1
// NVPTX32:#define __DBL_DENORM_MIN__ 4.9406564584124654e-324
// NVPTX32:#define __DBL_DIG__ 15
// NVPTX32:#define __DBL_EPSILON__ 2.2204460492503131e-16
// NVPTX32:#define __DBL_HAS_DENORM__ 1
// NVPTX32:#define __DBL_HAS_INFINITY__ 1
// NVPTX32:#define __DBL_HAS_QUIET_NAN__ 1
// NVPTX32:#define __DBL_MANT_DIG__ 53
// NVPTX32:#define __DBL_MAX_10_EXP__ 308
// NVPTX32:#define __DBL_MAX_EXP__ 1024
// NVPTX32:#define __DBL_MAX__ 1.7976931348623157e+308
// NVPTX32:#define __DBL_MIN_10_EXP__ (-307)
// NVPTX32:#define __DBL_MIN_EXP__ (-1021)
// NVPTX32:#define __DBL_MIN__ 2.2250738585072014e-308
// NVPTX32:#define __DECIMAL_DIG__ 17
// NVPTX32:#define __FINITE_MATH_ONLY__ 0
// NVPTX32:#define __FLT_DENORM_MIN__ 1.40129846e-45F
// NVPTX32:#define __FLT_DIG__ 6
// NVPTX32:#define __FLT_EPSILON__ 1.19209290e-7F
// NVPTX32:#define __FLT_EVAL_METHOD__ 0
// NVPTX32:#define __FLT_HAS_DENORM__ 1
// NVPTX32:#define __FLT_HAS_INFINITY__ 1
// NVPTX32:#define __FLT_HAS_QUIET_NAN__ 1
// NVPTX32:#define __FLT_MANT_DIG__ 24
// NVPTX32:#define __FLT_MAX_10_EXP__ 38
// NVPTX32:#define __FLT_MAX_EXP__ 128
// NVPTX32:#define __FLT_MAX__ 3.40282347e+38F
// NVPTX32:#define __FLT_MIN_10_EXP__ (-37)
// NVPTX32:#define __FLT_MIN_EXP__ (-125)
// NVPTX32:#define __FLT_MIN__ 1.17549435e-38F
// NVPTX32:#define __FLT_RADIX__ 2
// NVPTX32:#define __INT16_TYPE__ short
// NVPTX32:#define __INT32_TYPE__ int
// NVPTX32:#define __INT64_C_SUFFIX__ LL
// NVPTX32:#define __INT64_TYPE__ long long int
// NVPTX32:#define __INT8_TYPE__ char
// NVPTX32:#define __INTMAX_MAX__ 9223372036854775807LL
// NVPTX32:#define __INTMAX_TYPE__ long long int
// NVPTX32:#define __INTMAX_WIDTH__ 64
// NVPTX32:#define __INTPTR_TYPE__ unsigned int
// NVPTX32:#define __INTPTR_WIDTH__ 32
// NVPTX32:#define __INT_MAX__ 2147483647
// NVPTX32:#define __LDBL_DENORM_MIN__ 4.9406564584124654e-324L
// NVPTX32:#define __LDBL_DIG__ 15
// NVPTX32:#define __LDBL_EPSILON__ 2.2204460492503131e-16L
// NVPTX32:#define __LDBL_HAS_DENORM__ 1
// NVPTX32:#define __LDBL_HAS_INFINITY__ 1
// NVPTX32:#define __LDBL_HAS_QUIET_NAN__ 1
// NVPTX32:#define __LDBL_MANT_DIG__ 53
// NVPTX32:#define __LDBL_MAX_10_EXP__ 308
// NVPTX32:#define __LDBL_MAX_EXP__ 1024
// NVPTX32:#define __LDBL_MAX__ 1.7976931348623157e+308L
// NVPTX32:#define __LDBL_MIN_10_EXP__ (-307)
// NVPTX32:#define __LDBL_MIN_EXP__ (-1021)
// NVPTX32:#define __LDBL_MIN__ 2.2250738585072014e-308L
// NVPTX32:#define __LONG_LONG_MAX__ 9223372036854775807LL
// NVPTX32:#define __LONG_MAX__ 9223372036854775807L
// NVPTX32-NOT:#define __LP64__
// NVPTX32:#define __NVPTX__ 1
// NVPTX32:#define __POINTER_WIDTH__ 32
// NVPTX32:#define __PRAGMA_REDEFINE_EXTNAME 1
// NVPTX32:#define __PTRDIFF_TYPE__ unsigned int
// NVPTX32:#define __PTRDIFF_WIDTH__ 32
// NVPTX32:#define __PTX__ 1
// NVPTX32:#define __SCHAR_MAX__ 127
// NVPTX32:#define __SHRT_MAX__ 32767
// NVPTX32:#define __SIG_ATOMIC_WIDTH__ 32
// NVPTX32:#define __SIZEOF_DOUBLE__ 8
// NVPTX32:#define __SIZEOF_FLOAT__ 4
// NVPTX32:#define __SIZEOF_INT__ 4
// NVPTX32:#define __SIZEOF_LONG_DOUBLE__ 8
// NVPTX32:#define __SIZEOF_LONG_LONG__ 8
// NVPTX32:#define __SIZEOF_LONG__ 8
// NVPTX32:#define __SIZEOF_POINTER__ 4
// NVPTX32:#define __SIZEOF_PTRDIFF_T__ 4
// NVPTX32:#define __SIZEOF_SHORT__ 2
// NVPTX32:#define __SIZEOF_SIZE_T__ 4
// NVPTX32:#define __SIZEOF_WCHAR_T__ 4
// NVPTX32:#define __SIZEOF_WINT_T__ 4
// NVPTX32:#define __SIZE_MAX__ 4294967295U
// NVPTX32:#define __SIZE_TYPE__ unsigned int
// NVPTX32:#define __SIZE_WIDTH__ 32
// NVPTX32:#define __UINTMAX_TYPE__ long long unsigned int
// NVPTX32:#define __USER_LABEL_PREFIX__ _
// NVPTX32:#define __WCHAR_MAX__ 2147483647
// NVPTX32:#define __WCHAR_TYPE__ int
// NVPTX32:#define __WCHAR_WIDTH__ 32
// NVPTX32:#define __WINT_TYPE__ int
// NVPTX32:#define __WINT_WIDTH__ 32
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=nvptx64-none-none < /dev/null | FileCheck -check-prefix NVPTX64 %s
//
// NVPTX64:#define _LP64 1
// NVPTX64:#define __BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__
// NVPTX64:#define __CHAR16_TYPE__ unsigned short
// NVPTX64:#define __CHAR32_TYPE__ unsigned int
// NVPTX64:#define __CHAR_BIT__ 8
// NVPTX64:#define __CONSTANT_CFSTRINGS__ 1
// NVPTX64:#define __DBL_DENORM_MIN__ 4.9406564584124654e-324
// NVPTX64:#define __DBL_DIG__ 15
// NVPTX64:#define __DBL_EPSILON__ 2.2204460492503131e-16
// NVPTX64:#define __DBL_HAS_DENORM__ 1
// NVPTX64:#define __DBL_HAS_INFINITY__ 1
// NVPTX64:#define __DBL_HAS_QUIET_NAN__ 1
// NVPTX64:#define __DBL_MANT_DIG__ 53
// NVPTX64:#define __DBL_MAX_10_EXP__ 308
// NVPTX64:#define __DBL_MAX_EXP__ 1024
// NVPTX64:#define __DBL_MAX__ 1.7976931348623157e+308
// NVPTX64:#define __DBL_MIN_10_EXP__ (-307)
// NVPTX64:#define __DBL_MIN_EXP__ (-1021)
// NVPTX64:#define __DBL_MIN__ 2.2250738585072014e-308
// NVPTX64:#define __DECIMAL_DIG__ 17
// NVPTX64:#define __FINITE_MATH_ONLY__ 0
// NVPTX64:#define __FLT_DENORM_MIN__ 1.40129846e-45F
// NVPTX64:#define __FLT_DIG__ 6
// NVPTX64:#define __FLT_EPSILON__ 1.19209290e-7F
// NVPTX64:#define __FLT_EVAL_METHOD__ 0
// NVPTX64:#define __FLT_HAS_DENORM__ 1
// NVPTX64:#define __FLT_HAS_INFINITY__ 1
// NVPTX64:#define __FLT_HAS_QUIET_NAN__ 1
// NVPTX64:#define __FLT_MANT_DIG__ 24
// NVPTX64:#define __FLT_MAX_10_EXP__ 38
// NVPTX64:#define __FLT_MAX_EXP__ 128
// NVPTX64:#define __FLT_MAX__ 3.40282347e+38F
// NVPTX64:#define __FLT_MIN_10_EXP__ (-37)
// NVPTX64:#define __FLT_MIN_EXP__ (-125)
// NVPTX64:#define __FLT_MIN__ 1.17549435e-38F
// NVPTX64:#define __FLT_RADIX__ 2
// NVPTX64:#define __INT16_TYPE__ short
// NVPTX64:#define __INT32_TYPE__ int
// NVPTX64:#define __INT64_C_SUFFIX__ LL
// NVPTX64:#define __INT64_TYPE__ long long int
// NVPTX64:#define __INT8_TYPE__ char
// NVPTX64:#define __INTMAX_MAX__ 9223372036854775807LL
// NVPTX64:#define __INTMAX_TYPE__ long long int
// NVPTX64:#define __INTMAX_WIDTH__ 64
// NVPTX64:#define __INTPTR_TYPE__ long long unsigned int
// NVPTX64:#define __INTPTR_WIDTH__ 64
// NVPTX64:#define __INT_MAX__ 2147483647
// NVPTX64:#define __LDBL_DENORM_MIN__ 4.9406564584124654e-324L
// NVPTX64:#define __LDBL_DIG__ 15
// NVPTX64:#define __LDBL_EPSILON__ 2.2204460492503131e-16L
// NVPTX64:#define __LDBL_HAS_DENORM__ 1
// NVPTX64:#define __LDBL_HAS_INFINITY__ 1
// NVPTX64:#define __LDBL_HAS_QUIET_NAN__ 1
// NVPTX64:#define __LDBL_MANT_DIG__ 53
// NVPTX64:#define __LDBL_MAX_10_EXP__ 308
// NVPTX64:#define __LDBL_MAX_EXP__ 1024
// NVPTX64:#define __LDBL_MAX__ 1.7976931348623157e+308L
// NVPTX64:#define __LDBL_MIN_10_EXP__ (-307)
// NVPTX64:#define __LDBL_MIN_EXP__ (-1021)
// NVPTX64:#define __LDBL_MIN__ 2.2250738585072014e-308L
// NVPTX64:#define __LONG_LONG_MAX__ 9223372036854775807LL
// NVPTX64:#define __LONG_MAX__ 9223372036854775807L
// NVPTX64:#define __LP64__ 1
// NVPTX64:#define __NVPTX__ 1
// NVPTX64:#define __POINTER_WIDTH__ 64
// NVPTX64:#define __PRAGMA_REDEFINE_EXTNAME 1
// NVPTX64:#define __PTRDIFF_TYPE__ long long unsigned int
// NVPTX64:#define __PTRDIFF_WIDTH__ 64
// NVPTX64:#define __PTX__ 1
// NVPTX64:#define __SCHAR_MAX__ 127
// NVPTX64:#define __SHRT_MAX__ 32767
// NVPTX64:#define __SIG_ATOMIC_WIDTH__ 32
// NVPTX64:#define __SIZEOF_DOUBLE__ 8
// NVPTX64:#define __SIZEOF_FLOAT__ 4
// NVPTX64:#define __SIZEOF_INT__ 4
// NVPTX64:#define __SIZEOF_LONG_DOUBLE__ 8
// NVPTX64:#define __SIZEOF_LONG_LONG__ 8
// NVPTX64:#define __SIZEOF_LONG__ 8
// NVPTX64:#define __SIZEOF_POINTER__ 8
// NVPTX64:#define __SIZEOF_PTRDIFF_T__ 8
// NVPTX64:#define __SIZEOF_SHORT__ 2
// NVPTX64:#define __SIZEOF_SIZE_T__ 8
// NVPTX64:#define __SIZEOF_WCHAR_T__ 4
// NVPTX64:#define __SIZEOF_WINT_T__ 4
// NVPTX64:#define __SIZE_MAX__ 18446744073709551615UL
// NVPTX64:#define __SIZE_TYPE__ long long unsigned int
// NVPTX64:#define __SIZE_WIDTH__ 64
// NVPTX64:#define __UINTMAX_TYPE__ long long unsigned int
// NVPTX64:#define __USER_LABEL_PREFIX__ _
// NVPTX64:#define __WCHAR_MAX__ 2147483647
// NVPTX64:#define __WCHAR_TYPE__ int
// NVPTX64:#define __WCHAR_WIDTH__ 32
// NVPTX64:#define __WINT_TYPE__ int
// NVPTX64:#define __WINT_WIDTH__ 32
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=powerpc-none-none -target-cpu 603e < /dev/null | FileCheck -check-prefix PPC603E %s
//
// PPC603E:#define _ARCH_603 1
// PPC603E:#define _ARCH_603E 1
// PPC603E:#define _ARCH_PPC 1
// PPC603E:#define _ARCH_PPCGR 1
// PPC603E:#define _BIG_ENDIAN 1
// PPC603E-NOT:#define _LP64
// PPC603E:#define __BIG_ENDIAN__ 1
// PPC603E:#define __BYTE_ORDER__ __ORDER_BIG_ENDIAN__
// PPC603E:#define __CHAR16_TYPE__ unsigned short
// PPC603E:#define __CHAR32_TYPE__ unsigned int
// PPC603E:#define __CHAR_BIT__ 8
// PPC603E:#define __DBL_DENORM_MIN__ 4.9406564584124654e-324
// PPC603E:#define __DBL_DIG__ 15
// PPC603E:#define __DBL_EPSILON__ 2.2204460492503131e-16
// PPC603E:#define __DBL_HAS_DENORM__ 1
// PPC603E:#define __DBL_HAS_INFINITY__ 1
// PPC603E:#define __DBL_HAS_QUIET_NAN__ 1
// PPC603E:#define __DBL_MANT_DIG__ 53
// PPC603E:#define __DBL_MAX_10_EXP__ 308
// PPC603E:#define __DBL_MAX_EXP__ 1024
// PPC603E:#define __DBL_MAX__ 1.7976931348623157e+308
// PPC603E:#define __DBL_MIN_10_EXP__ (-307)
// PPC603E:#define __DBL_MIN_EXP__ (-1021)
// PPC603E:#define __DBL_MIN__ 2.2250738585072014e-308
// PPC603E:#define __DECIMAL_DIG__ 33
// PPC603E:#define __FLT_DENORM_MIN__ 1.40129846e-45F
// PPC603E:#define __FLT_DIG__ 6
// PPC603E:#define __FLT_EPSILON__ 1.19209290e-7F
// PPC603E:#define __FLT_EVAL_METHOD__ 0
// PPC603E:#define __FLT_HAS_DENORM__ 1
// PPC603E:#define __FLT_HAS_INFINITY__ 1
// PPC603E:#define __FLT_HAS_QUIET_NAN__ 1
// PPC603E:#define __FLT_MANT_DIG__ 24
// PPC603E:#define __FLT_MAX_10_EXP__ 38
// PPC603E:#define __FLT_MAX_EXP__ 128
// PPC603E:#define __FLT_MAX__ 3.40282347e+38F
// PPC603E:#define __FLT_MIN_10_EXP__ (-37)
// PPC603E:#define __FLT_MIN_EXP__ (-125)
// PPC603E:#define __FLT_MIN__ 1.17549435e-38F
// PPC603E:#define __FLT_RADIX__ 2
// PPC603E:#define __INT16_TYPE__ short
// PPC603E:#define __INT32_TYPE__ int
// PPC603E:#define __INT64_C_SUFFIX__ LL
// PPC603E:#define __INT64_TYPE__ long long int
// PPC603E:#define __INT8_TYPE__ char
// PPC603E:#define __INTMAX_MAX__ 9223372036854775807LL
// PPC603E:#define __INTMAX_TYPE__ long long int
// PPC603E:#define __INTMAX_WIDTH__ 64
// PPC603E:#define __INTPTR_TYPE__ long int
// PPC603E:#define __INTPTR_WIDTH__ 32
// PPC603E:#define __INT_MAX__ 2147483647
// PPC603E:#define __LDBL_DENORM_MIN__ 4.94065645841246544176568792868221e-324L
// PPC603E:#define __LDBL_DIG__ 31
// PPC603E:#define __LDBL_EPSILON__ 4.94065645841246544176568792868221e-324L
// PPC603E:#define __LDBL_HAS_DENORM__ 1
// PPC603E:#define __LDBL_HAS_INFINITY__ 1
// PPC603E:#define __LDBL_HAS_QUIET_NAN__ 1
// PPC603E:#define __LDBL_MANT_DIG__ 106
// PPC603E:#define __LDBL_MAX_10_EXP__ 308
// PPC603E:#define __LDBL_MAX_EXP__ 1024
// PPC603E:#define __LDBL_MAX__ 1.79769313486231580793728971405301e+308L
// PPC603E:#define __LDBL_MIN_10_EXP__ (-291)
// PPC603E:#define __LDBL_MIN_EXP__ (-968)
// PPC603E:#define __LDBL_MIN__ 2.00416836000897277799610805135016e-292L
// PPC603E:#define __LONG_DOUBLE_128__ 1
// PPC603E:#define __LONG_LONG_MAX__ 9223372036854775807LL
// PPC603E:#define __LONG_MAX__ 2147483647L
// PPC603E-NOT:#define __LP64__
// PPC603E:#define __NATURAL_ALIGNMENT__ 1
// PPC603E:#define __POINTER_WIDTH__ 32
// PPC603E:#define __POWERPC__ 1
// PPC603E:#define __PPC__ 1
// PPC603E:#define __PTRDIFF_TYPE__ long int
// PPC603E:#define __PTRDIFF_WIDTH__ 32
// PPC603E:#define __REGISTER_PREFIX__
// PPC603E:#define __SCHAR_MAX__ 127
// PPC603E:#define __SHRT_MAX__ 32767
// PPC603E:#define __SIG_ATOMIC_WIDTH__ 32
// PPC603E:#define __SIZEOF_DOUBLE__ 8
// PPC603E:#define __SIZEOF_FLOAT__ 4
// PPC603E:#define __SIZEOF_INT__ 4
// PPC603E:#define __SIZEOF_LONG_DOUBLE__ 16
// PPC603E:#define __SIZEOF_LONG_LONG__ 8
// PPC603E:#define __SIZEOF_LONG__ 4
// PPC603E:#define __SIZEOF_POINTER__ 4
// PPC603E:#define __SIZEOF_PTRDIFF_T__ 4
// PPC603E:#define __SIZEOF_SHORT__ 2
// PPC603E:#define __SIZEOF_SIZE_T__ 4
// PPC603E:#define __SIZEOF_WCHAR_T__ 4
// PPC603E:#define __SIZEOF_WINT_T__ 4
// PPC603E:#define __SIZE_MAX__ 4294967295U
// PPC603E:#define __SIZE_TYPE__ long unsigned int
// PPC603E:#define __SIZE_WIDTH__ 32
// PPC603E:#define __UINTMAX_TYPE__ long long unsigned int
// PPC603E:#define __USER_LABEL_PREFIX__ _
// PPC603E:#define __WCHAR_MAX__ 2147483647
// PPC603E:#define __WCHAR_TYPE__ int
// PPC603E:#define __WCHAR_WIDTH__ 32
// PPC603E:#define __WINT_TYPE__ int
// PPC603E:#define __WINT_WIDTH__ 32
// PPC603E:#define __powerpc__ 1
// PPC603E:#define __ppc__ 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=powerpc64-none-none -target-cpu pwr7 -fno-signed-char < /dev/null | FileCheck -check-prefix PPC64 %s
//
// PPC64:#define _ARCH_PPC 1
// PPC64:#define _ARCH_PPC64 1
// PPC64:#define _ARCH_PPCGR 1
// PPC64:#define _ARCH_PPCSQ 1
// PPC64:#define _ARCH_PWR4 1
// PPC64:#define _ARCH_PWR5 1
// PPC64:#define _ARCH_PWR6 1
// PPC64:#define _ARCH_PWR7 1
// PPC64:#define _BIG_ENDIAN 1
// PPC64:#define _LP64 1
// PPC64:#define __BIG_ENDIAN__ 1
// PPC64:#define __BYTE_ORDER__ __ORDER_BIG_ENDIAN__
// PPC64:#define __CHAR16_TYPE__ unsigned short
// PPC64:#define __CHAR32_TYPE__ unsigned int
// PPC64:#define __CHAR_BIT__ 8
// PPC64:#define __CHAR_UNSIGNED__ 1
// PPC64:#define __DBL_DENORM_MIN__ 4.9406564584124654e-324
// PPC64:#define __DBL_DIG__ 15
// PPC64:#define __DBL_EPSILON__ 2.2204460492503131e-16
// PPC64:#define __DBL_HAS_DENORM__ 1
// PPC64:#define __DBL_HAS_INFINITY__ 1
// PPC64:#define __DBL_HAS_QUIET_NAN__ 1
// PPC64:#define __DBL_MANT_DIG__ 53
// PPC64:#define __DBL_MAX_10_EXP__ 308
// PPC64:#define __DBL_MAX_EXP__ 1024
// PPC64:#define __DBL_MAX__ 1.7976931348623157e+308
// PPC64:#define __DBL_MIN_10_EXP__ (-307)
// PPC64:#define __DBL_MIN_EXP__ (-1021)
// PPC64:#define __DBL_MIN__ 2.2250738585072014e-308
// PPC64:#define __DECIMAL_DIG__ 33
// PPC64:#define __FLT_DENORM_MIN__ 1.40129846e-45F
// PPC64:#define __FLT_DIG__ 6
// PPC64:#define __FLT_EPSILON__ 1.19209290e-7F
// PPC64:#define __FLT_EVAL_METHOD__ 0
// PPC64:#define __FLT_HAS_DENORM__ 1
// PPC64:#define __FLT_HAS_INFINITY__ 1
// PPC64:#define __FLT_HAS_QUIET_NAN__ 1
// PPC64:#define __FLT_MANT_DIG__ 24
// PPC64:#define __FLT_MAX_10_EXP__ 38
// PPC64:#define __FLT_MAX_EXP__ 128
// PPC64:#define __FLT_MAX__ 3.40282347e+38F
// PPC64:#define __FLT_MIN_10_EXP__ (-37)
// PPC64:#define __FLT_MIN_EXP__ (-125)
// PPC64:#define __FLT_MIN__ 1.17549435e-38F
// PPC64:#define __FLT_RADIX__ 2
// PPC64:#define __INT16_TYPE__ short
// PPC64:#define __INT32_TYPE__ int
// PPC64:#define __INT64_C_SUFFIX__ L
// PPC64:#define __INT64_TYPE__ long int
// PPC64:#define __INT8_TYPE__ char
// PPC64:#define __INTMAX_MAX__ 9223372036854775807L
// PPC64:#define __INTMAX_TYPE__ long int
// PPC64:#define __INTMAX_WIDTH__ 64
// PPC64:#define __INTPTR_TYPE__ long int
// PPC64:#define __INTPTR_WIDTH__ 64
// PPC64:#define __INT_MAX__ 2147483647
// PPC64:#define __LDBL_DENORM_MIN__ 4.94065645841246544176568792868221e-324L
// PPC64:#define __LDBL_DIG__ 31
// PPC64:#define __LDBL_EPSILON__ 4.94065645841246544176568792868221e-324L
// PPC64:#define __LDBL_HAS_DENORM__ 1
// PPC64:#define __LDBL_HAS_INFINITY__ 1
// PPC64:#define __LDBL_HAS_QUIET_NAN__ 1
// PPC64:#define __LDBL_MANT_DIG__ 106
// PPC64:#define __LDBL_MAX_10_EXP__ 308
// PPC64:#define __LDBL_MAX_EXP__ 1024
// PPC64:#define __LDBL_MAX__ 1.79769313486231580793728971405301e+308L
// PPC64:#define __LDBL_MIN_10_EXP__ (-291)
// PPC64:#define __LDBL_MIN_EXP__ (-968)
// PPC64:#define __LDBL_MIN__ 2.00416836000897277799610805135016e-292L
// PPC64:#define __LONG_DOUBLE_128__ 1
// PPC64:#define __LONG_LONG_MAX__ 9223372036854775807LL
// PPC64:#define __LONG_MAX__ 9223372036854775807L
// PPC64:#define __LP64__ 1
// PPC64:#define __NATURAL_ALIGNMENT__ 1
// PPC64:#define __POINTER_WIDTH__ 64
// PPC64:#define __POWERPC__ 1
// PPC64:#define __PPC64__ 1
// PPC64:#define __PPC__ 1
// PPC64:#define __PTRDIFF_TYPE__ long int
// PPC64:#define __PTRDIFF_WIDTH__ 64
// PPC64:#define __REGISTER_PREFIX__ 
// PPC64:#define __SCHAR_MAX__ 127
// PPC64:#define __SHRT_MAX__ 32767
// PPC64:#define __SIG_ATOMIC_WIDTH__ 32
// PPC64:#define __SIZEOF_DOUBLE__ 8
// PPC64:#define __SIZEOF_FLOAT__ 4
// PPC64:#define __SIZEOF_INT__ 4
// PPC64:#define __SIZEOF_LONG_DOUBLE__ 16
// PPC64:#define __SIZEOF_LONG_LONG__ 8
// PPC64:#define __SIZEOF_LONG__ 8
// PPC64:#define __SIZEOF_POINTER__ 8
// PPC64:#define __SIZEOF_PTRDIFF_T__ 8
// PPC64:#define __SIZEOF_SHORT__ 2
// PPC64:#define __SIZEOF_SIZE_T__ 8
// PPC64:#define __SIZEOF_WCHAR_T__ 4
// PPC64:#define __SIZEOF_WINT_T__ 4
// PPC64:#define __SIZE_MAX__ 18446744073709551615UL
// PPC64:#define __SIZE_TYPE__ long unsigned int
// PPC64:#define __SIZE_WIDTH__ 64
// PPC64:#define __UINTMAX_TYPE__ long unsigned int
// PPC64:#define __USER_LABEL_PREFIX__ _
// PPC64:#define __WCHAR_MAX__ 2147483647
// PPC64:#define __WCHAR_TYPE__ int
// PPC64:#define __WCHAR_WIDTH__ 32
// PPC64:#define __WINT_TYPE__ int
// PPC64:#define __WINT_WIDTH__ 32
// PPC64:#define __ppc64__ 1
// PPC64:#define __ppc__ 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=powerpc64le-none-none -target-cpu pwr7 -fno-signed-char < /dev/null | FileCheck -check-prefix PPC64LE %s
//
// PPC64LE:#define _ARCH_PPC 1
// PPC64LE:#define _ARCH_PPC64 1
// PPC64LE:#define _ARCH_PPCGR 1
// PPC64LE:#define _ARCH_PPCSQ 1
// PPC64LE:#define _ARCH_PWR4 1
// PPC64LE:#define _ARCH_PWR5 1
// PPC64LE:#define _ARCH_PWR5X 1
// PPC64LE:#define _ARCH_PWR6 1
// PPC64LE:#define _ARCH_PWR6X 1
// PPC64LE:#define _ARCH_PWR7 1
// PPC64LE:#define _LITTLE_ENDIAN 1
// PPC64LE:#define _LP64 1
// PPC64LE:#define __BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__
// PPC64LE:#define __CHAR16_TYPE__ unsigned short
// PPC64LE:#define __CHAR32_TYPE__ unsigned int
// PPC64LE:#define __CHAR_BIT__ 8
// PPC64LE:#define __CHAR_UNSIGNED__ 1
// PPC64LE:#define __DBL_DENORM_MIN__ 4.9406564584124654e-324
// PPC64LE:#define __DBL_DIG__ 15
// PPC64LE:#define __DBL_EPSILON__ 2.2204460492503131e-16
// PPC64LE:#define __DBL_HAS_DENORM__ 1
// PPC64LE:#define __DBL_HAS_INFINITY__ 1
// PPC64LE:#define __DBL_HAS_QUIET_NAN__ 1
// PPC64LE:#define __DBL_MANT_DIG__ 53
// PPC64LE:#define __DBL_MAX_10_EXP__ 308
// PPC64LE:#define __DBL_MAX_EXP__ 1024
// PPC64LE:#define __DBL_MAX__ 1.7976931348623157e+308
// PPC64LE:#define __DBL_MIN_10_EXP__ (-307)
// PPC64LE:#define __DBL_MIN_EXP__ (-1021)
// PPC64LE:#define __DBL_MIN__ 2.2250738585072014e-308
// PPC64LE:#define __DECIMAL_DIG__ 33
// PPC64LE:#define __FLT_DENORM_MIN__ 1.40129846e-45F
// PPC64LE:#define __FLT_DIG__ 6
// PPC64LE:#define __FLT_EPSILON__ 1.19209290e-7F
// PPC64LE:#define __FLT_EVAL_METHOD__ 0
// PPC64LE:#define __FLT_HAS_DENORM__ 1
// PPC64LE:#define __FLT_HAS_INFINITY__ 1
// PPC64LE:#define __FLT_HAS_QUIET_NAN__ 1
// PPC64LE:#define __FLT_MANT_DIG__ 24
// PPC64LE:#define __FLT_MAX_10_EXP__ 38
// PPC64LE:#define __FLT_MAX_EXP__ 128
// PPC64LE:#define __FLT_MAX__ 3.40282347e+38F
// PPC64LE:#define __FLT_MIN_10_EXP__ (-37)
// PPC64LE:#define __FLT_MIN_EXP__ (-125)
// PPC64LE:#define __FLT_MIN__ 1.17549435e-38F
// PPC64LE:#define __FLT_RADIX__ 2
// PPC64LE:#define __INT16_TYPE__ short
// PPC64LE:#define __INT32_TYPE__ int
// PPC64LE:#define __INT64_C_SUFFIX__ L
// PPC64LE:#define __INT64_TYPE__ long int
// PPC64LE:#define __INT8_TYPE__ char
// PPC64LE:#define __INTMAX_MAX__ 9223372036854775807L
// PPC64LE:#define __INTMAX_TYPE__ long int
// PPC64LE:#define __INTMAX_WIDTH__ 64
// PPC64LE:#define __INTPTR_TYPE__ long int
// PPC64LE:#define __INTPTR_WIDTH__ 64
// PPC64LE:#define __INT_MAX__ 2147483647
// PPC64LE:#define __LDBL_DENORM_MIN__ 4.94065645841246544176568792868221e-324L
// PPC64LE:#define __LDBL_DIG__ 31
// PPC64LE:#define __LDBL_EPSILON__ 4.94065645841246544176568792868221e-324L
// PPC64LE:#define __LDBL_HAS_DENORM__ 1
// PPC64LE:#define __LDBL_HAS_INFINITY__ 1
// PPC64LE:#define __LDBL_HAS_QUIET_NAN__ 1
// PPC64LE:#define __LDBL_MANT_DIG__ 106
// PPC64LE:#define __LDBL_MAX_10_EXP__ 308
// PPC64LE:#define __LDBL_MAX_EXP__ 1024
// PPC64LE:#define __LDBL_MAX__ 1.79769313486231580793728971405301e+308L
// PPC64LE:#define __LDBL_MIN_10_EXP__ (-291)
// PPC64LE:#define __LDBL_MIN_EXP__ (-968)
// PPC64LE:#define __LDBL_MIN__ 2.00416836000897277799610805135016e-292L
// PPC64LE:#define __LITTLE_ENDIAN__ 1
// PPC64LE:#define __LONG_DOUBLE_128__ 1
// PPC64LE:#define __LONG_LONG_MAX__ 9223372036854775807LL
// PPC64LE:#define __LONG_MAX__ 9223372036854775807L
// PPC64LE:#define __LP64__ 1
// PPC64LE:#define __NATURAL_ALIGNMENT__ 1
// PPC64LE:#define __POINTER_WIDTH__ 64
// PPC64LE:#define __POWERPC__ 1
// PPC64LE:#define __PPC64__ 1
// PPC64LE:#define __PPC__ 1
// PPC64LE:#define __PTRDIFF_TYPE__ long int
// PPC64LE:#define __PTRDIFF_WIDTH__ 64
// PPC64LE:#define __REGISTER_PREFIX__ 
// PPC64LE:#define __SCHAR_MAX__ 127
// PPC64LE:#define __SHRT_MAX__ 32767
// PPC64LE:#define __SIG_ATOMIC_WIDTH__ 32
// PPC64LE:#define __SIZEOF_DOUBLE__ 8
// PPC64LE:#define __SIZEOF_FLOAT__ 4
// PPC64LE:#define __SIZEOF_INT__ 4
// PPC64LE:#define __SIZEOF_LONG_DOUBLE__ 16
// PPC64LE:#define __SIZEOF_LONG_LONG__ 8
// PPC64LE:#define __SIZEOF_LONG__ 8
// PPC64LE:#define __SIZEOF_POINTER__ 8
// PPC64LE:#define __SIZEOF_PTRDIFF_T__ 8
// PPC64LE:#define __SIZEOF_SHORT__ 2
// PPC64LE:#define __SIZEOF_SIZE_T__ 8
// PPC64LE:#define __SIZEOF_WCHAR_T__ 4
// PPC64LE:#define __SIZEOF_WINT_T__ 4
// PPC64LE:#define __SIZE_MAX__ 18446744073709551615UL
// PPC64LE:#define __SIZE_TYPE__ long unsigned int
// PPC64LE:#define __SIZE_WIDTH__ 64
// PPC64LE:#define __UINTMAX_TYPE__ long unsigned int
// PPC64LE:#define __USER_LABEL_PREFIX__ _
// PPC64LE:#define __WCHAR_MAX__ 2147483647
// PPC64LE:#define __WCHAR_TYPE__ int
// PPC64LE:#define __WCHAR_WIDTH__ 32
// PPC64LE:#define __WINT_TYPE__ int
// PPC64LE:#define __WINT_WIDTH__ 32
// PPC64LE:#define __ppc64__ 1
// PPC64LE:#define __ppc__ 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=powerpc64-none-none -target-cpu a2q -fno-signed-char < /dev/null | FileCheck -check-prefix PPCA2Q %s
//
// PPCA2Q:#define _ARCH_A2 1
// PPCA2Q:#define _ARCH_A2Q 1
// PPCA2Q:#define _ARCH_PPC 1
// PPCA2Q:#define _ARCH_PPC64 1
// PPCA2Q:#define _ARCH_QP 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=powerpc64-bgq-linux -fno-signed-char < /dev/null | FileCheck -check-prefix PPCBGQ %s
//
// PPCBGQ:#define __THW_BLUEGENE__ 1
// PPCBGQ:#define __TOS_BGQ__ 1
// PPCBGQ:#define __bg__ 1
// PPCBGQ:#define __bgq__ 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=powerpc64-none-none -target-cpu 630 -fno-signed-char < /dev/null | FileCheck -check-prefix PPC630 %s
//
// PPC630:#define _ARCH_630 1
// PPC630:#define _ARCH_PPC 1
// PPC630:#define _ARCH_PPC64 1
// PPC630:#define _ARCH_PPCGR 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=powerpc64-none-none -target-cpu pwr3 -fno-signed-char < /dev/null | FileCheck -check-prefix PPCPWR3 %s
//
// PPCPWR3:#define _ARCH_PPC 1
// PPCPWR3:#define _ARCH_PPC64 1
// PPCPWR3:#define _ARCH_PPCGR 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=powerpc64-none-none -target-cpu power3 -fno-signed-char < /dev/null | FileCheck -check-prefix PPCPOWER3 %s
//
// PPCPOWER3:#define _ARCH_PPC 1
// PPCPOWER3:#define _ARCH_PPC64 1
// PPCPOWER3:#define _ARCH_PPCGR 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=powerpc64-none-none -target-cpu pwr4 -fno-signed-char < /dev/null | FileCheck -check-prefix PPCPWR4 %s
//
// PPCPWR4:#define _ARCH_PPC 1
// PPCPWR4:#define _ARCH_PPC64 1
// PPCPWR4:#define _ARCH_PPCGR 1
// PPCPWR4:#define _ARCH_PPCSQ 1
// PPCPWR4:#define _ARCH_PWR4 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=powerpc64-none-none -target-cpu power4 -fno-signed-char < /dev/null | FileCheck -check-prefix PPCPOWER4 %s
//
// PPCPOWER4:#define _ARCH_PPC 1
// PPCPOWER4:#define _ARCH_PPC64 1
// PPCPOWER4:#define _ARCH_PPCGR 1
// PPCPOWER4:#define _ARCH_PPCSQ 1
// PPCPOWER4:#define _ARCH_PWR4 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=powerpc64-none-none -target-cpu pwr5 -fno-signed-char < /dev/null | FileCheck -check-prefix PPCPWR5 %s
//
// PPCPWR5:#define _ARCH_PPC 1
// PPCPWR5:#define _ARCH_PPC64 1
// PPCPWR5:#define _ARCH_PPCGR 1
// PPCPWR5:#define _ARCH_PPCSQ 1
// PPCPWR5:#define _ARCH_PWR4 1
// PPCPWR5:#define _ARCH_PWR5 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=powerpc64-none-none -target-cpu power5 -fno-signed-char < /dev/null | FileCheck -check-prefix PPCPOWER5 %s
//
// PPCPOWER5:#define _ARCH_PPC 1
// PPCPOWER5:#define _ARCH_PPC64 1
// PPCPOWER5:#define _ARCH_PPCGR 1
// PPCPOWER5:#define _ARCH_PPCSQ 1
// PPCPOWER5:#define _ARCH_PWR4 1
// PPCPOWER5:#define _ARCH_PWR5 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=powerpc64-none-none -target-cpu pwr5x -fno-signed-char < /dev/null | FileCheck -check-prefix PPCPWR5X %s
//
// PPCPWR5X:#define _ARCH_PPC 1
// PPCPWR5X:#define _ARCH_PPC64 1
// PPCPWR5X:#define _ARCH_PPCGR 1
// PPCPWR5X:#define _ARCH_PPCSQ 1
// PPCPWR5X:#define _ARCH_PWR4 1
// PPCPWR5X:#define _ARCH_PWR5 1
// PPCPWR5X:#define _ARCH_PWR5X 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=powerpc64-none-none -target-cpu power5x -fno-signed-char < /dev/null | FileCheck -check-prefix PPCPOWER5X %s
//
// PPCPOWER5X:#define _ARCH_PPC 1
// PPCPOWER5X:#define _ARCH_PPC64 1
// PPCPOWER5X:#define _ARCH_PPCGR 1
// PPCPOWER5X:#define _ARCH_PPCSQ 1
// PPCPOWER5X:#define _ARCH_PWR4 1
// PPCPOWER5X:#define _ARCH_PWR5 1
// PPCPOWER5X:#define _ARCH_PWR5X 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=powerpc64-none-none -target-cpu pwr6 -fno-signed-char < /dev/null | FileCheck -check-prefix PPCPWR6 %s
//
// PPCPWR6:#define _ARCH_PPC 1
// PPCPWR6:#define _ARCH_PPC64 1
// PPCPWR6:#define _ARCH_PPCGR 1
// PPCPWR6:#define _ARCH_PPCSQ 1
// PPCPWR6:#define _ARCH_PWR4 1
// PPCPWR6:#define _ARCH_PWR5 1
// PPCPWR6:#define _ARCH_PWR5X 1
// PPCPWR6:#define _ARCH_PWR6 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=powerpc64-none-none -target-cpu power6 -fno-signed-char < /dev/null | FileCheck -check-prefix PPCPOWER6 %s
//
// PPCPOWER6:#define _ARCH_PPC 1
// PPCPOWER6:#define _ARCH_PPC64 1
// PPCPOWER6:#define _ARCH_PPCGR 1
// PPCPOWER6:#define _ARCH_PPCSQ 1
// PPCPOWER6:#define _ARCH_PWR4 1
// PPCPOWER6:#define _ARCH_PWR5 1
// PPCPOWER6:#define _ARCH_PWR5X 1
// PPCPOWER6:#define _ARCH_PWR6 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=powerpc64-none-none -target-cpu pwr6x -fno-signed-char < /dev/null | FileCheck -check-prefix PPCPWR6X %s
//
// PPCPWR6X:#define _ARCH_PPC 1
// PPCPWR6X:#define _ARCH_PPC64 1
// PPCPWR6X:#define _ARCH_PPCGR 1
// PPCPWR6X:#define _ARCH_PPCSQ 1
// PPCPWR6X:#define _ARCH_PWR4 1
// PPCPWR6X:#define _ARCH_PWR5 1
// PPCPWR6X:#define _ARCH_PWR5X 1
// PPCPWR6X:#define _ARCH_PWR6 1
// PPCPWR6X:#define _ARCH_PWR6X 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=powerpc64-none-none -target-cpu power6x -fno-signed-char < /dev/null | FileCheck -check-prefix PPCPOWER6X %s
//
// PPCPOWER6X:#define _ARCH_PPC 1
// PPCPOWER6X:#define _ARCH_PPC64 1
// PPCPOWER6X:#define _ARCH_PPCGR 1
// PPCPOWER6X:#define _ARCH_PPCSQ 1
// PPCPOWER6X:#define _ARCH_PWR4 1
// PPCPOWER6X:#define _ARCH_PWR5 1
// PPCPOWER6X:#define _ARCH_PWR5X 1
// PPCPOWER6X:#define _ARCH_PWR6 1
// PPCPOWER6X:#define _ARCH_PWR6X 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=powerpc64-none-none -target-cpu pwr7 -fno-signed-char < /dev/null | FileCheck -check-prefix PPCPWR7 %s
//
// PPCPWR7:#define _ARCH_PPC 1
// PPCPWR7:#define _ARCH_PPC64 1
// PPCPWR7:#define _ARCH_PPCGR 1
// PPCPWR7:#define _ARCH_PPCSQ 1
// PPCPWR7:#define _ARCH_PWR4 1
// PPCPWR7:#define _ARCH_PWR5 1
// PPCPWR7:#define _ARCH_PWR5X 1
// PPCPWR7:#define _ARCH_PWR6 1
// PPCPWR7:#define _ARCH_PWR6X 1
// PPCPWR7:#define _ARCH_PWR7 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=powerpc64-none-none -target-cpu power7 -fno-signed-char < /dev/null | FileCheck -check-prefix PPCPOWER7 %s
//
// PPCPOWER7:#define _ARCH_PPC 1
// PPCPOWER7:#define _ARCH_PPC64 1
// PPCPOWER7:#define _ARCH_PPCGR 1
// PPCPOWER7:#define _ARCH_PPCSQ 1
// PPCPOWER7:#define _ARCH_PWR4 1
// PPCPOWER7:#define _ARCH_PWR5 1
// PPCPOWER7:#define _ARCH_PWR5X 1
// PPCPOWER7:#define _ARCH_PWR6 1
// PPCPOWER7:#define _ARCH_PWR6X 1
// PPCPOWER7:#define _ARCH_PWR7 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=powerpc64-unknown-linux-gnu -fno-signed-char < /dev/null | FileCheck -check-prefix PPC64-LINUX %s
//
// PPC64-LINUX:#define _ARCH_PPC 1
// PPC64-LINUX:#define _ARCH_PPC64 1
// PPC64-LINUX:#define _BIG_ENDIAN 1
// PPC64-LINUX:#define _LP64 1
// PPC64-LINUX:#define __BIG_ENDIAN__ 1
// PPC64-LINUX:#define __BYTE_ORDER__ __ORDER_BIG_ENDIAN__
// PPC64-LINUX:#define __CHAR16_TYPE__ unsigned short
// PPC64-LINUX:#define __CHAR32_TYPE__ unsigned int
// PPC64-LINUX:#define __CHAR_BIT__ 8
// PPC64-LINUX:#define __CHAR_UNSIGNED__ 1
// PPC64-LINUX:#define __DBL_DENORM_MIN__ 4.9406564584124654e-324
// PPC64-LINUX:#define __DBL_DIG__ 15
// PPC64-LINUX:#define __DBL_EPSILON__ 2.2204460492503131e-16
// PPC64-LINUX:#define __DBL_HAS_DENORM__ 1
// PPC64-LINUX:#define __DBL_HAS_INFINITY__ 1
// PPC64-LINUX:#define __DBL_HAS_QUIET_NAN__ 1
// PPC64-LINUX:#define __DBL_MANT_DIG__ 53
// PPC64-LINUX:#define __DBL_MAX_10_EXP__ 308
// PPC64-LINUX:#define __DBL_MAX_EXP__ 1024
// PPC64-LINUX:#define __DBL_MAX__ 1.7976931348623157e+308
// PPC64-LINUX:#define __DBL_MIN_10_EXP__ (-307)
// PPC64-LINUX:#define __DBL_MIN_EXP__ (-1021)
// PPC64-LINUX:#define __DBL_MIN__ 2.2250738585072014e-308
// PPC64-LINUX:#define __DECIMAL_DIG__ 33
// PPC64-LINUX:#define __FLT_DENORM_MIN__ 1.40129846e-45F
// PPC64-LINUX:#define __FLT_DIG__ 6
// PPC64-LINUX:#define __FLT_EPSILON__ 1.19209290e-7F
// PPC64-LINUX:#define __FLT_EVAL_METHOD__ 0
// PPC64-LINUX:#define __FLT_HAS_DENORM__ 1
// PPC64-LINUX:#define __FLT_HAS_INFINITY__ 1
// PPC64-LINUX:#define __FLT_HAS_QUIET_NAN__ 1
// PPC64-LINUX:#define __FLT_MANT_DIG__ 24
// PPC64-LINUX:#define __FLT_MAX_10_EXP__ 38
// PPC64-LINUX:#define __FLT_MAX_EXP__ 128
// PPC64-LINUX:#define __FLT_MAX__ 3.40282347e+38F
// PPC64-LINUX:#define __FLT_MIN_10_EXP__ (-37)
// PPC64-LINUX:#define __FLT_MIN_EXP__ (-125)
// PPC64-LINUX:#define __FLT_MIN__ 1.17549435e-38F
// PPC64-LINUX:#define __FLT_RADIX__ 2
// PPC64-LINUX:#define __INT16_TYPE__ short
// PPC64-LINUX:#define __INT32_TYPE__ int
// PPC64-LINUX:#define __INT64_C_SUFFIX__ L
// PPC64-LINUX:#define __INT64_TYPE__ long int
// PPC64-LINUX:#define __INT8_TYPE__ char
// PPC64-LINUX:#define __INTMAX_MAX__ 9223372036854775807L
// PPC64-LINUX:#define __INTMAX_TYPE__ long int
// PPC64-LINUX:#define __INTMAX_WIDTH__ 64
// PPC64-LINUX:#define __INTPTR_TYPE__ long int
// PPC64-LINUX:#define __INTPTR_WIDTH__ 64
// PPC64-LINUX:#define __INT_MAX__ 2147483647
// PPC64-LINUX:#define __LDBL_DENORM_MIN__ 4.94065645841246544176568792868221e-324L
// PPC64-LINUX:#define __LDBL_DIG__ 31
// PPC64-LINUX:#define __LDBL_EPSILON__ 4.94065645841246544176568792868221e-324L
// PPC64-LINUX:#define __LDBL_HAS_DENORM__ 1
// PPC64-LINUX:#define __LDBL_HAS_INFINITY__ 1
// PPC64-LINUX:#define __LDBL_HAS_QUIET_NAN__ 1
// PPC64-LINUX:#define __LDBL_MANT_DIG__ 106
// PPC64-LINUX:#define __LDBL_MAX_10_EXP__ 308
// PPC64-LINUX:#define __LDBL_MAX_EXP__ 1024
// PPC64-LINUX:#define __LDBL_MAX__ 1.79769313486231580793728971405301e+308L
// PPC64-LINUX:#define __LDBL_MIN_10_EXP__ (-291)
// PPC64-LINUX:#define __LDBL_MIN_EXP__ (-968)
// PPC64-LINUX:#define __LDBL_MIN__ 2.00416836000897277799610805135016e-292L
// PPC64-LINUX:#define __LONG_DOUBLE_128__ 1
// PPC64-LINUX:#define __LONG_LONG_MAX__ 9223372036854775807LL
// PPC64-LINUX:#define __LONG_MAX__ 9223372036854775807L
// PPC64-LINUX:#define __LP64__ 1
// PPC64-LINUX:#define __NATURAL_ALIGNMENT__ 1
// PPC64-LINUX:#define __POINTER_WIDTH__ 64
// PPC64-LINUX:#define __POWERPC__ 1
// PPC64-LINUX:#define __PPC64__ 1
// PPC64-LINUX:#define __PPC__ 1
// PPC64-LINUX:#define __PTRDIFF_TYPE__ long int
// PPC64-LINUX:#define __PTRDIFF_WIDTH__ 64
// PPC64-LINUX:#define __REGISTER_PREFIX__
// PPC64-LINUX:#define __SCHAR_MAX__ 127
// PPC64-LINUX:#define __SHRT_MAX__ 32767
// PPC64-LINUX:#define __SIG_ATOMIC_WIDTH__ 32
// PPC64-LINUX:#define __SIZEOF_DOUBLE__ 8
// PPC64-LINUX:#define __SIZEOF_FLOAT__ 4
// PPC64-LINUX:#define __SIZEOF_INT__ 4
// PPC64-LINUX:#define __SIZEOF_LONG_DOUBLE__ 16
// PPC64-LINUX:#define __SIZEOF_LONG_LONG__ 8
// PPC64-LINUX:#define __SIZEOF_LONG__ 8
// PPC64-LINUX:#define __SIZEOF_POINTER__ 8
// PPC64-LINUX:#define __SIZEOF_PTRDIFF_T__ 8
// PPC64-LINUX:#define __SIZEOF_SHORT__ 2
// PPC64-LINUX:#define __SIZEOF_SIZE_T__ 8
// PPC64-LINUX:#define __SIZEOF_WCHAR_T__ 4
// PPC64-LINUX:#define __SIZEOF_WINT_T__ 4
// PPC64-LINUX:#define __SIZE_MAX__ 18446744073709551615UL
// PPC64-LINUX:#define __SIZE_TYPE__ long unsigned int
// PPC64-LINUX:#define __SIZE_WIDTH__ 64
// PPC64-LINUX:#define __UINTMAX_TYPE__ long unsigned int
// PPC64-LINUX:#define __USER_LABEL_PREFIX__
// PPC64-LINUX:#define __WCHAR_MAX__ 2147483647
// PPC64-LINUX:#define __WCHAR_TYPE__ int
// PPC64-LINUX:#define __WCHAR_WIDTH__ 32
// PPC64-LINUX:#define __WINT_TYPE__ unsigned int
// PPC64-LINUX:#define __WINT_UNSIGNED__ 1
// PPC64-LINUX:#define __WINT_WIDTH__ 32
// PPC64-LINUX:#define __powerpc64__ 1
// PPC64-LINUX:#define __powerpc__ 1
// PPC64-LINUX:#define __ppc64__ 1
// PPC64-LINUX:#define __ppc__ 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=powerpc-none-none -fno-signed-char < /dev/null | FileCheck -check-prefix PPC %s
//
// PPC:#define _ARCH_PPC 1
// PPC:#define _BIG_ENDIAN 1
// PPC-NOT:#define _LP64
// PPC:#define __BIG_ENDIAN__ 1
// PPC:#define __BYTE_ORDER__ __ORDER_BIG_ENDIAN__
// PPC:#define __CHAR16_TYPE__ unsigned short
// PPC:#define __CHAR32_TYPE__ unsigned int
// PPC:#define __CHAR_BIT__ 8
// PPC:#define __CHAR_UNSIGNED__ 1
// PPC:#define __DBL_DENORM_MIN__ 4.9406564584124654e-324
// PPC:#define __DBL_DIG__ 15
// PPC:#define __DBL_EPSILON__ 2.2204460492503131e-16
// PPC:#define __DBL_HAS_DENORM__ 1
// PPC:#define __DBL_HAS_INFINITY__ 1
// PPC:#define __DBL_HAS_QUIET_NAN__ 1
// PPC:#define __DBL_MANT_DIG__ 53
// PPC:#define __DBL_MAX_10_EXP__ 308
// PPC:#define __DBL_MAX_EXP__ 1024
// PPC:#define __DBL_MAX__ 1.7976931348623157e+308
// PPC:#define __DBL_MIN_10_EXP__ (-307)
// PPC:#define __DBL_MIN_EXP__ (-1021)
// PPC:#define __DBL_MIN__ 2.2250738585072014e-308
// PPC:#define __DECIMAL_DIG__ 33
// PPC:#define __FLT_DENORM_MIN__ 1.40129846e-45F
// PPC:#define __FLT_DIG__ 6
// PPC:#define __FLT_EPSILON__ 1.19209290e-7F
// PPC:#define __FLT_EVAL_METHOD__ 0
// PPC:#define __FLT_HAS_DENORM__ 1
// PPC:#define __FLT_HAS_INFINITY__ 1
// PPC:#define __FLT_HAS_QUIET_NAN__ 1
// PPC:#define __FLT_MANT_DIG__ 24
// PPC:#define __FLT_MAX_10_EXP__ 38
// PPC:#define __FLT_MAX_EXP__ 128
// PPC:#define __FLT_MAX__ 3.40282347e+38F
// PPC:#define __FLT_MIN_10_EXP__ (-37)
// PPC:#define __FLT_MIN_EXP__ (-125)
// PPC:#define __FLT_MIN__ 1.17549435e-38F
// PPC:#define __FLT_RADIX__ 2
// PPC:#define __INT16_TYPE__ short
// PPC:#define __INT32_TYPE__ int
// PPC:#define __INT64_C_SUFFIX__ LL
// PPC:#define __INT64_TYPE__ long long int
// PPC:#define __INT8_TYPE__ char
// PPC:#define __INTMAX_MAX__ 9223372036854775807LL
// PPC:#define __INTMAX_TYPE__ long long int
// PPC:#define __INTMAX_WIDTH__ 64
// PPC:#define __INTPTR_TYPE__ long int
// PPC:#define __INTPTR_WIDTH__ 32
// PPC:#define __INT_MAX__ 2147483647
// PPC:#define __LDBL_DENORM_MIN__ 4.94065645841246544176568792868221e-324L
// PPC:#define __LDBL_DIG__ 31
// PPC:#define __LDBL_EPSILON__ 4.94065645841246544176568792868221e-324L
// PPC:#define __LDBL_HAS_DENORM__ 1
// PPC:#define __LDBL_HAS_INFINITY__ 1
// PPC:#define __LDBL_HAS_QUIET_NAN__ 1
// PPC:#define __LDBL_MANT_DIG__ 106
// PPC:#define __LDBL_MAX_10_EXP__ 308
// PPC:#define __LDBL_MAX_EXP__ 1024
// PPC:#define __LDBL_MAX__ 1.79769313486231580793728971405301e+308L
// PPC:#define __LDBL_MIN_10_EXP__ (-291)
// PPC:#define __LDBL_MIN_EXP__ (-968)
// PPC:#define __LDBL_MIN__ 2.00416836000897277799610805135016e-292L
// PPC:#define __LONG_DOUBLE_128__ 1
// PPC:#define __LONG_LONG_MAX__ 9223372036854775807LL
// PPC:#define __LONG_MAX__ 2147483647L
// PPC-NOT:#define __LP64__
// PPC:#define __NATURAL_ALIGNMENT__ 1
// PPC:#define __POINTER_WIDTH__ 32
// PPC:#define __POWERPC__ 1
// PPC:#define __PPC__ 1
// PPC:#define __PTRDIFF_TYPE__ long int
// PPC:#define __PTRDIFF_WIDTH__ 32
// PPC:#define __REGISTER_PREFIX__ 
// PPC:#define __SCHAR_MAX__ 127
// PPC:#define __SHRT_MAX__ 32767
// PPC:#define __SIG_ATOMIC_WIDTH__ 32
// PPC:#define __SIZEOF_DOUBLE__ 8
// PPC:#define __SIZEOF_FLOAT__ 4
// PPC:#define __SIZEOF_INT__ 4
// PPC:#define __SIZEOF_LONG_DOUBLE__ 16
// PPC:#define __SIZEOF_LONG_LONG__ 8
// PPC:#define __SIZEOF_LONG__ 4
// PPC:#define __SIZEOF_POINTER__ 4
// PPC:#define __SIZEOF_PTRDIFF_T__ 4
// PPC:#define __SIZEOF_SHORT__ 2
// PPC:#define __SIZEOF_SIZE_T__ 4
// PPC:#define __SIZEOF_WCHAR_T__ 4
// PPC:#define __SIZEOF_WINT_T__ 4
// PPC:#define __SIZE_MAX__ 4294967295U
// PPC:#define __SIZE_TYPE__ long unsigned int
// PPC:#define __SIZE_WIDTH__ 32
// PPC:#define __UINTMAX_TYPE__ long long unsigned int
// PPC:#define __USER_LABEL_PREFIX__ _
// PPC:#define __WCHAR_MAX__ 2147483647
// PPC:#define __WCHAR_TYPE__ int
// PPC:#define __WCHAR_WIDTH__ 32
// PPC:#define __WINT_TYPE__ int
// PPC:#define __WINT_WIDTH__ 32
// PPC:#define __ppc__ 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=powerpc-unknown-linux-gnu -fno-signed-char < /dev/null | FileCheck -check-prefix PPC-LINUX %s
//
// PPC-LINUX:#define _ARCH_PPC 1
// PPC-LINUX:#define _BIG_ENDIAN 1
// PPC-LINUX-NOT:#define _LP64
// PPC-LINUX:#define __BIG_ENDIAN__ 1
// PPC-LINUX:#define __BYTE_ORDER__ __ORDER_BIG_ENDIAN__
// PPC-LINUX:#define __CHAR16_TYPE__ unsigned short
// PPC-LINUX:#define __CHAR32_TYPE__ unsigned int
// PPC-LINUX:#define __CHAR_BIT__ 8
// PPC-LINUX:#define __CHAR_UNSIGNED__ 1
// PPC-LINUX:#define __DBL_DENORM_MIN__ 4.9406564584124654e-324
// PPC-LINUX:#define __DBL_DIG__ 15
// PPC-LINUX:#define __DBL_EPSILON__ 2.2204460492503131e-16
// PPC-LINUX:#define __DBL_HAS_DENORM__ 1
// PPC-LINUX:#define __DBL_HAS_INFINITY__ 1
// PPC-LINUX:#define __DBL_HAS_QUIET_NAN__ 1
// PPC-LINUX:#define __DBL_MANT_DIG__ 53
// PPC-LINUX:#define __DBL_MAX_10_EXP__ 308
// PPC-LINUX:#define __DBL_MAX_EXP__ 1024
// PPC-LINUX:#define __DBL_MAX__ 1.7976931348623157e+308
// PPC-LINUX:#define __DBL_MIN_10_EXP__ (-307)
// PPC-LINUX:#define __DBL_MIN_EXP__ (-1021)
// PPC-LINUX:#define __DBL_MIN__ 2.2250738585072014e-308
// PPC-LINUX:#define __DECIMAL_DIG__ 33
// PPC-LINUX:#define __FLT_DENORM_MIN__ 1.40129846e-45F
// PPC-LINUX:#define __FLT_DIG__ 6
// PPC-LINUX:#define __FLT_EPSILON__ 1.19209290e-7F
// PPC-LINUX:#define __FLT_EVAL_METHOD__ 0
// PPC-LINUX:#define __FLT_HAS_DENORM__ 1
// PPC-LINUX:#define __FLT_HAS_INFINITY__ 1
// PPC-LINUX:#define __FLT_HAS_QUIET_NAN__ 1
// PPC-LINUX:#define __FLT_MANT_DIG__ 24
// PPC-LINUX:#define __FLT_MAX_10_EXP__ 38
// PPC-LINUX:#define __FLT_MAX_EXP__ 128
// PPC-LINUX:#define __FLT_MAX__ 3.40282347e+38F
// PPC-LINUX:#define __FLT_MIN_10_EXP__ (-37)
// PPC-LINUX:#define __FLT_MIN_EXP__ (-125)
// PPC-LINUX:#define __FLT_MIN__ 1.17549435e-38F
// PPC-LINUX:#define __FLT_RADIX__ 2
// PPC-LINUX:#define __INT16_TYPE__ short
// PPC-LINUX:#define __INT32_TYPE__ int
// PPC-LINUX:#define __INT64_C_SUFFIX__ LL
// PPC-LINUX:#define __INT64_TYPE__ long long int
// PPC-LINUX:#define __INT8_TYPE__ char
// PPC-LINUX:#define __INTMAX_MAX__ 9223372036854775807LL
// PPC-LINUX:#define __INTMAX_TYPE__ long long int
// PPC-LINUX:#define __INTMAX_WIDTH__ 64
// PPC-LINUX:#define __INTPTR_TYPE__ int
// PPC-LINUX:#define __INTPTR_WIDTH__ 32
// PPC-LINUX:#define __INT_MAX__ 2147483647
// PPC-LINUX:#define __LDBL_DENORM_MIN__ 4.94065645841246544176568792868221e-324L
// PPC-LINUX:#define __LDBL_DIG__ 31
// PPC-LINUX:#define __LDBL_EPSILON__ 4.94065645841246544176568792868221e-324L
// PPC-LINUX:#define __LDBL_HAS_DENORM__ 1
// PPC-LINUX:#define __LDBL_HAS_INFINITY__ 1
// PPC-LINUX:#define __LDBL_HAS_QUIET_NAN__ 1
// PPC-LINUX:#define __LDBL_MANT_DIG__ 106
// PPC-LINUX:#define __LDBL_MAX_10_EXP__ 308
// PPC-LINUX:#define __LDBL_MAX_EXP__ 1024
// PPC-LINUX:#define __LDBL_MAX__ 1.79769313486231580793728971405301e+308L
// PPC-LINUX:#define __LDBL_MIN_10_EXP__ (-291)
// PPC-LINUX:#define __LDBL_MIN_EXP__ (-968)
// PPC-LINUX:#define __LDBL_MIN__ 2.00416836000897277799610805135016e-292L
// PPC-LINUX:#define __LONG_DOUBLE_128__ 1
// PPC-LINUX:#define __LONG_LONG_MAX__ 9223372036854775807LL
// PPC-LINUX:#define __LONG_MAX__ 2147483647L
// PPC-LINUX-NOT:#define __LP64__
// PPC-LINUX:#define __NATURAL_ALIGNMENT__ 1
// PPC-LINUX:#define __POINTER_WIDTH__ 32
// PPC-LINUX:#define __POWERPC__ 1
// PPC-LINUX:#define __PPC__ 1
// PPC-LINUX:#define __PTRDIFF_TYPE__ int
// PPC-LINUX:#define __PTRDIFF_WIDTH__ 32
// PPC-LINUX:#define __REGISTER_PREFIX__
// PPC-LINUX:#define __SCHAR_MAX__ 127
// PPC-LINUX:#define __SHRT_MAX__ 32767
// PPC-LINUX:#define __SIG_ATOMIC_WIDTH__ 32
// PPC-LINUX:#define __SIZEOF_DOUBLE__ 8
// PPC-LINUX:#define __SIZEOF_FLOAT__ 4
// PPC-LINUX:#define __SIZEOF_INT__ 4
// PPC-LINUX:#define __SIZEOF_LONG_DOUBLE__ 16
// PPC-LINUX:#define __SIZEOF_LONG_LONG__ 8
// PPC-LINUX:#define __SIZEOF_LONG__ 4
// PPC-LINUX:#define __SIZEOF_POINTER__ 4
// PPC-LINUX:#define __SIZEOF_PTRDIFF_T__ 4
// PPC-LINUX:#define __SIZEOF_SHORT__ 2
// PPC-LINUX:#define __SIZEOF_SIZE_T__ 4
// PPC-LINUX:#define __SIZEOF_WCHAR_T__ 4
// PPC-LINUX:#define __SIZEOF_WINT_T__ 4
// PPC-LINUX:#define __SIZE_MAX__ 4294967295U
// PPC-LINUX:#define __SIZE_TYPE__ unsigned int
// PPC-LINUX:#define __SIZE_WIDTH__ 32
// PPC-LINUX:#define __UINTMAX_TYPE__ long long unsigned int
// PPC-LINUX:#define __USER_LABEL_PREFIX__
// PPC-LINUX:#define __WCHAR_MAX__ 2147483647
// PPC-LINUX:#define __WCHAR_TYPE__ int
// PPC-LINUX:#define __WCHAR_WIDTH__ 32
// PPC-LINUX:#define __WINT_TYPE__ unsigned int
// PPC-LINUX:#define __WINT_UNSIGNED__ 1
// PPC-LINUX:#define __WINT_WIDTH__ 32
// PPC-LINUX:#define __powerpc__ 1
// PPC-LINUX:#define __ppc__ 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=powerpc-apple-darwin8 < /dev/null | FileCheck -check-prefix PPC-DARWIN %s
//
// PPC-DARWIN:#define _ARCH_PPC 1
// PPC-DARWIN:#define _BIG_ENDIAN 1
// PPC-DARWIN:#define __BIG_ENDIAN__ 1
// PPC-DARWIN:#define __BYTE_ORDER__ __ORDER_BIG_ENDIAN__
// PPC-DARWIN:#define __CHAR16_TYPE__ unsigned short
// PPC-DARWIN:#define __CHAR32_TYPE__ unsigned int
// PPC-DARWIN:#define __CHAR_BIT__ 8
// PPC-DARWIN:#define __DBL_DENORM_MIN__ 4.9406564584124654e-324
// PPC-DARWIN:#define __DBL_DIG__ 15
// PPC-DARWIN:#define __DBL_EPSILON__ 2.2204460492503131e-16
// PPC-DARWIN:#define __DBL_HAS_DENORM__ 1
// PPC-DARWIN:#define __DBL_HAS_INFINITY__ 1
// PPC-DARWIN:#define __DBL_HAS_QUIET_NAN__ 1
// PPC-DARWIN:#define __DBL_MANT_DIG__ 53
// PPC-DARWIN:#define __DBL_MAX_10_EXP__ 308
// PPC-DARWIN:#define __DBL_MAX_EXP__ 1024
// PPC-DARWIN:#define __DBL_MAX__ 1.7976931348623157e+308
// PPC-DARWIN:#define __DBL_MIN_10_EXP__ (-307)
// PPC-DARWIN:#define __DBL_MIN_EXP__ (-1021)
// PPC-DARWIN:#define __DBL_MIN__ 2.2250738585072014e-308
// PPC-DARWIN:#define __DECIMAL_DIG__ 33
// PPC-DARWIN:#define __FLT_DENORM_MIN__ 1.40129846e-45F
// PPC-DARWIN:#define __FLT_DIG__ 6
// PPC-DARWIN:#define __FLT_EPSILON__ 1.19209290e-7F
// PPC-DARWIN:#define __FLT_EVAL_METHOD__ 0
// PPC-DARWIN:#define __FLT_HAS_DENORM__ 1
// PPC-DARWIN:#define __FLT_HAS_INFINITY__ 1
// PPC-DARWIN:#define __FLT_HAS_QUIET_NAN__ 1
// PPC-DARWIN:#define __FLT_MANT_DIG__ 24
// PPC-DARWIN:#define __FLT_MAX_10_EXP__ 38
// PPC-DARWIN:#define __FLT_MAX_EXP__ 128
// PPC-DARWIN:#define __FLT_MAX__ 3.40282347e+38F
// PPC-DARWIN:#define __FLT_MIN_10_EXP__ (-37)
// PPC-DARWIN:#define __FLT_MIN_EXP__ (-125)
// PPC-DARWIN:#define __FLT_MIN__ 1.17549435e-38F
// PPC-DARWIN:#define __FLT_RADIX__ 2
// PPC-DARWIN:#define __INT16_TYPE__ short
// PPC-DARWIN:#define __INT32_TYPE__ int
// PPC-DARWIN:#define __INT64_C_SUFFIX__ LL
// PPC-DARWIN:#define __INT64_TYPE__ long long int
// PPC-DARWIN:#define __INT8_TYPE__ char
// PPC-DARWIN:#define __INTMAX_MAX__ 9223372036854775807LL
// PPC-DARWIN:#define __INTMAX_TYPE__ long long int
// PPC-DARWIN:#define __INTMAX_WIDTH__ 64
// PPC-DARWIN:#define __INTPTR_TYPE__ long int
// PPC-DARWIN:#define __INTPTR_WIDTH__ 32
// PPC-DARWIN:#define __INT_MAX__ 2147483647
// PPC-DARWIN:#define __LDBL_DENORM_MIN__ 4.94065645841246544176568792868221e-324L
// PPC-DARWIN:#define __LDBL_DIG__ 31
// PPC-DARWIN:#define __LDBL_EPSILON__ 4.94065645841246544176568792868221e-324L
// PPC-DARWIN:#define __LDBL_HAS_DENORM__ 1
// PPC-DARWIN:#define __LDBL_HAS_INFINITY__ 1
// PPC-DARWIN:#define __LDBL_HAS_QUIET_NAN__ 1
// PPC-DARWIN:#define __LDBL_MANT_DIG__ 106
// PPC-DARWIN:#define __LDBL_MAX_10_EXP__ 308
// PPC-DARWIN:#define __LDBL_MAX_EXP__ 1024
// PPC-DARWIN:#define __LDBL_MAX__ 1.79769313486231580793728971405301e+308L
// PPC-DARWIN:#define __LDBL_MIN_10_EXP__ (-291)
// PPC-DARWIN:#define __LDBL_MIN_EXP__ (-968)
// PPC-DARWIN:#define __LDBL_MIN__ 2.00416836000897277799610805135016e-292L
// PPC-DARWIN:#define __LONG_DOUBLE_128__ 1
// PPC-DARWIN:#define __LONG_LONG_MAX__ 9223372036854775807LL
// PPC-DARWIN:#define __LONG_MAX__ 2147483647L
// PPC-DARWIN:#define __MACH__ 1
// PPC-DARWIN:#define __NATURAL_ALIGNMENT__ 1
// PPC-DARWIN:#define __ORDER_BIG_ENDIAN__ 4321
// PPC-DARWIN:#define __ORDER_LITTLE_ENDIAN__ 1234
// PPC-DARWIN:#define __ORDER_PDP_ENDIAN__ 3412
// PPC-DARWIN:#define __POINTER_WIDTH__ 32
// PPC-DARWIN:#define __POWERPC__ 1
// PPC-DARWIN:#define __PPC__ 1
// PPC-DARWIN:#define __PTRDIFF_TYPE__ int
// PPC-DARWIN:#define __PTRDIFF_WIDTH__ 32
// PPC-DARWIN:#define __REGISTER_PREFIX__ 
// PPC-DARWIN:#define __SCHAR_MAX__ 127
// PPC-DARWIN:#define __SHRT_MAX__ 32767
// PPC-DARWIN:#define __SIG_ATOMIC_WIDTH__ 32
// PPC-DARWIN:#define __SIZEOF_DOUBLE__ 8
// PPC-DARWIN:#define __SIZEOF_FLOAT__ 4
// PPC-DARWIN:#define __SIZEOF_INT__ 4
// PPC-DARWIN:#define __SIZEOF_LONG_DOUBLE__ 16
// PPC-DARWIN:#define __SIZEOF_LONG_LONG__ 8
// PPC-DARWIN:#define __SIZEOF_LONG__ 4
// PPC-DARWIN:#define __SIZEOF_POINTER__ 4
// PPC-DARWIN:#define __SIZEOF_PTRDIFF_T__ 4
// PPC-DARWIN:#define __SIZEOF_SHORT__ 2
// PPC-DARWIN:#define __SIZEOF_SIZE_T__ 4
// PPC-DARWIN:#define __SIZEOF_WCHAR_T__ 4
// PPC-DARWIN:#define __SIZEOF_WINT_T__ 4
// PPC-DARWIN:#define __SIZE_MAX__ 4294967295UL
// PPC-DARWIN:#define __SIZE_TYPE__ long unsigned int
// PPC-DARWIN:#define __SIZE_WIDTH__ 32
// PPC-DARWIN:#define __STDC_HOSTED__ 0
// PPC-DARWIN:#define __STDC_VERSION__ 199901L
// PPC-DARWIN:#define __STDC__ 1
// PPC-DARWIN:#define __UINTMAX_TYPE__ long long unsigned int
// PPC-DARWIN:#define __USER_LABEL_PREFIX__ _
// PPC-DARWIN:#define __WCHAR_MAX__ 2147483647
// PPC-DARWIN:#define __WCHAR_TYPE__ int
// PPC-DARWIN:#define __WCHAR_WIDTH__ 32
// PPC-DARWIN:#define __WINT_TYPE__ int
// PPC-DARWIN:#define __WINT_WIDTH__ 32
// PPC-DARWIN:#define __powerpc__ 1
// PPC-DARWIN:#define __ppc__ 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=s390x-none-none -fno-signed-char < /dev/null | FileCheck -check-prefix S390X %s
//
// S390X:#define __CHAR16_TYPE__ unsigned short
// S390X:#define __CHAR32_TYPE__ unsigned int
// S390X:#define __CHAR_BIT__ 8
// S390X:#define __CHAR_UNSIGNED__ 1
// S390X:#define __DBL_DENORM_MIN__ 4.9406564584124654e-324
// S390X:#define __DBL_DIG__ 15
// S390X:#define __DBL_EPSILON__ 2.2204460492503131e-16
// S390X:#define __DBL_HAS_DENORM__ 1
// S390X:#define __DBL_HAS_INFINITY__ 1
// S390X:#define __DBL_HAS_QUIET_NAN__ 1
// S390X:#define __DBL_MANT_DIG__ 53
// S390X:#define __DBL_MAX_10_EXP__ 308
// S390X:#define __DBL_MAX_EXP__ 1024
// S390X:#define __DBL_MAX__ 1.7976931348623157e+308
// S390X:#define __DBL_MIN_10_EXP__ (-307)
// S390X:#define __DBL_MIN_EXP__ (-1021)
// S390X:#define __DBL_MIN__ 2.2250738585072014e-308
// S390X:#define __DECIMAL_DIG__ 36
// S390X:#define __FLT_DENORM_MIN__ 1.40129846e-45F
// S390X:#define __FLT_DIG__ 6
// S390X:#define __FLT_EPSILON__ 1.19209290e-7F
// S390X:#define __FLT_EVAL_METHOD__ 0
// S390X:#define __FLT_HAS_DENORM__ 1
// S390X:#define __FLT_HAS_INFINITY__ 1
// S390X:#define __FLT_HAS_QUIET_NAN__ 1
// S390X:#define __FLT_MANT_DIG__ 24
// S390X:#define __FLT_MAX_10_EXP__ 38
// S390X:#define __FLT_MAX_EXP__ 128
// S390X:#define __FLT_MAX__ 3.40282347e+38F
// S390X:#define __FLT_MIN_10_EXP__ (-37)
// S390X:#define __FLT_MIN_EXP__ (-125)
// S390X:#define __FLT_MIN__ 1.17549435e-38F
// S390X:#define __FLT_RADIX__ 2
// S390X:#define __INT16_TYPE__ short
// S390X:#define __INT32_TYPE__ int
// S390X:#define __INT64_C_SUFFIX__ L
// S390X:#define __INT64_TYPE__ long long int
// S390X:#define __INT8_TYPE__ char
// S390X:#define __INTMAX_MAX__ 9223372036854775807LL
// S390X:#define __INTMAX_TYPE__ long long int
// S390X:#define __INTMAX_WIDTH__ 64
// S390X:#define __INTPTR_TYPE__ long int
// S390X:#define __INTPTR_WIDTH__ 64
// S390X:#define __INT_MAX__ 2147483647
// S390X:#define __LDBL_DENORM_MIN__ 6.47517511943802511092443895822764655e-4966L
// S390X:#define __LDBL_DIG__ 33
// S390X:#define __LDBL_EPSILON__ 1.92592994438723585305597794258492732e-34L
// S390X:#define __LDBL_HAS_DENORM__ 1
// S390X:#define __LDBL_HAS_INFINITY__ 1
// S390X:#define __LDBL_HAS_QUIET_NAN__ 1
// S390X:#define __LDBL_MANT_DIG__ 113
// S390X:#define __LDBL_MAX_10_EXP__ 4932
// S390X:#define __LDBL_MAX_EXP__ 16384
// S390X:#define __LDBL_MAX__ 1.18973149535723176508575932662800702e+4932L
// S390X:#define __LDBL_MIN_10_EXP__ (-4931)
// S390X:#define __LDBL_MIN_EXP__ (-16381)
// S390X:#define __LDBL_MIN__ 3.36210314311209350626267781732175260e-4932L
// S390X:#define __LONG_LONG_MAX__ 9223372036854775807LL
// S390X:#define __LONG_MAX__ 9223372036854775807L
// S390X:#define __NO_INLINE__ 1
// S390X:#define __POINTER_WIDTH__ 64
// S390X:#define __PTRDIFF_TYPE__ long int
// S390X:#define __PTRDIFF_WIDTH__ 64
// S390X:#define __SCHAR_MAX__ 127
// S390X:#define __SHRT_MAX__ 32767
// S390X:#define __SIG_ATOMIC_WIDTH__ 32
// S390X:#define __SIZEOF_DOUBLE__ 8
// S390X:#define __SIZEOF_FLOAT__ 4
// S390X:#define __SIZEOF_INT__ 4
// S390X:#define __SIZEOF_LONG_DOUBLE__ 16
// S390X:#define __SIZEOF_LONG_LONG__ 8
// S390X:#define __SIZEOF_LONG__ 8
// S390X:#define __SIZEOF_POINTER__ 8
// S390X:#define __SIZEOF_PTRDIFF_T__ 8
// S390X:#define __SIZEOF_SHORT__ 2
// S390X:#define __SIZEOF_SIZE_T__ 8
// S390X:#define __SIZEOF_WCHAR_T__ 4
// S390X:#define __SIZEOF_WINT_T__ 4
// S390X:#define __SIZE_TYPE__ long unsigned int
// S390X:#define __SIZE_WIDTH__ 64
// S390X:#define __UINTMAX_TYPE__ long long unsigned int
// S390X:#define __USER_LABEL_PREFIX__ _
// S390X:#define __WCHAR_MAX__ 2147483647
// S390X:#define __WCHAR_TYPE__ int
// S390X:#define __WCHAR_WIDTH__ 32
// S390X:#define __WINT_TYPE__ int
// S390X:#define __WINT_WIDTH__ 32
// S390X:#define __s390__ 1
// S390X:#define __s390x__ 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=sparc-none-none < /dev/null | FileCheck -check-prefix SPARC %s
//
// SPARC-NOT:#define _LP64
// SPARC:#define __BYTE_ORDER__ __ORDER_BIG_ENDIAN__
// SPARC:#define __CHAR16_TYPE__ unsigned short
// SPARC:#define __CHAR32_TYPE__ unsigned int
// SPARC:#define __CHAR_BIT__ 8
// SPARC:#define __DBL_DENORM_MIN__ 4.9406564584124654e-324
// SPARC:#define __DBL_DIG__ 15
// SPARC:#define __DBL_EPSILON__ 2.2204460492503131e-16
// SPARC:#define __DBL_HAS_DENORM__ 1
// SPARC:#define __DBL_HAS_INFINITY__ 1
// SPARC:#define __DBL_HAS_QUIET_NAN__ 1
// SPARC:#define __DBL_MANT_DIG__ 53
// SPARC:#define __DBL_MAX_10_EXP__ 308
// SPARC:#define __DBL_MAX_EXP__ 1024
// SPARC:#define __DBL_MAX__ 1.7976931348623157e+308
// SPARC:#define __DBL_MIN_10_EXP__ (-307)
// SPARC:#define __DBL_MIN_EXP__ (-1021)
// SPARC:#define __DBL_MIN__ 2.2250738585072014e-308
// SPARC:#define __DECIMAL_DIG__ 17
// SPARC:#define __FLT_DENORM_MIN__ 1.40129846e-45F
// SPARC:#define __FLT_DIG__ 6
// SPARC:#define __FLT_EPSILON__ 1.19209290e-7F
// SPARC:#define __FLT_EVAL_METHOD__ 0
// SPARC:#define __FLT_HAS_DENORM__ 1
// SPARC:#define __FLT_HAS_INFINITY__ 1
// SPARC:#define __FLT_HAS_QUIET_NAN__ 1
// SPARC:#define __FLT_MANT_DIG__ 24
// SPARC:#define __FLT_MAX_10_EXP__ 38
// SPARC:#define __FLT_MAX_EXP__ 128
// SPARC:#define __FLT_MAX__ 3.40282347e+38F
// SPARC:#define __FLT_MIN_10_EXP__ (-37)
// SPARC:#define __FLT_MIN_EXP__ (-125)
// SPARC:#define __FLT_MIN__ 1.17549435e-38F
// SPARC:#define __FLT_RADIX__ 2
// SPARC:#define __INT16_TYPE__ short
// SPARC:#define __INT32_TYPE__ int
// SPARC:#define __INT64_C_SUFFIX__ LL
// SPARC:#define __INT64_TYPE__ long long int
// SPARC:#define __INT8_TYPE__ char
// SPARC:#define __INTMAX_MAX__ 9223372036854775807LL
// SPARC:#define __INTMAX_TYPE__ long long int
// SPARC:#define __INTMAX_WIDTH__ 64
// SPARC:#define __INTPTR_TYPE__ long int
// SPARC:#define __INTPTR_WIDTH__ 32
// SPARC:#define __INT_MAX__ 2147483647
// SPARC:#define __LDBL_DENORM_MIN__ 4.9406564584124654e-324L
// SPARC:#define __LDBL_DIG__ 15
// SPARC:#define __LDBL_EPSILON__ 2.2204460492503131e-16L
// SPARC:#define __LDBL_HAS_DENORM__ 1
// SPARC:#define __LDBL_HAS_INFINITY__ 1
// SPARC:#define __LDBL_HAS_QUIET_NAN__ 1
// SPARC:#define __LDBL_MANT_DIG__ 53
// SPARC:#define __LDBL_MAX_10_EXP__ 308
// SPARC:#define __LDBL_MAX_EXP__ 1024
// SPARC:#define __LDBL_MAX__ 1.7976931348623157e+308L
// SPARC:#define __LDBL_MIN_10_EXP__ (-307)
// SPARC:#define __LDBL_MIN_EXP__ (-1021)
// SPARC:#define __LDBL_MIN__ 2.2250738585072014e-308L
// SPARC:#define __LONG_LONG_MAX__ 9223372036854775807LL
// SPARC:#define __LONG_MAX__ 2147483647L
// SPARC-NOT:#define __LP64__
// SPARC:#define __POINTER_WIDTH__ 32
// SPARC:#define __PTRDIFF_TYPE__ long int
// SPARC:#define __PTRDIFF_WIDTH__ 32
// SPARC:#define __REGISTER_PREFIX__
// SPARC:#define __SCHAR_MAX__ 127
// SPARC:#define __SHRT_MAX__ 32767
// SPARC:#define __SIG_ATOMIC_WIDTH__ 32
// SPARC:#define __SIZEOF_DOUBLE__ 8
// SPARC:#define __SIZEOF_FLOAT__ 4
// SPARC:#define __SIZEOF_INT__ 4
// SPARC:#define __SIZEOF_LONG_DOUBLE__ 8
// SPARC:#define __SIZEOF_LONG_LONG__ 8
// SPARC:#define __SIZEOF_LONG__ 4
// SPARC:#define __SIZEOF_POINTER__ 4
// SPARC:#define __SIZEOF_PTRDIFF_T__ 4
// SPARC:#define __SIZEOF_SHORT__ 2
// SPARC:#define __SIZEOF_SIZE_T__ 4
// SPARC:#define __SIZEOF_WCHAR_T__ 4
// SPARC:#define __SIZEOF_WINT_T__ 4
// SPARC:#define __SIZE_MAX__ 4294967295U
// SPARC:#define __SIZE_TYPE__ long unsigned int
// SPARC:#define __SIZE_WIDTH__ 32
// SPARC:#define __UINTMAX_TYPE__ long long unsigned int
// SPARC:#define __USER_LABEL_PREFIX__ _
// SPARC:#define __VERSION__ "4.2.1 Compatible
// SPARC:#define __WCHAR_MAX__ 2147483647
// SPARC:#define __WCHAR_TYPE__ int
// SPARC:#define __WCHAR_WIDTH__ 32
// SPARC:#define __WINT_TYPE__ int
// SPARC:#define __WINT_WIDTH__ 32
// SPARC:#define __sparc 1
// SPARC:#define __sparc__ 1
// SPARC:#define __sparcv8 1
// SPARC:#define sparc 1
// 
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=tce-none-none < /dev/null | FileCheck -check-prefix TCE %s
//
// TCE-NOT:#define _LP64
// TCE:#define __BYTE_ORDER__ __ORDER_BIG_ENDIAN__
// TCE:#define __CHAR16_TYPE__ unsigned short
// TCE:#define __CHAR32_TYPE__ unsigned int
// TCE:#define __CHAR_BIT__ 8
// TCE:#define __DBL_DENORM_MIN__ 1.40129846e-45
// TCE:#define __DBL_DIG__ 6
// TCE:#define __DBL_EPSILON__ 1.19209290e-7
// TCE:#define __DBL_HAS_DENORM__ 1
// TCE:#define __DBL_HAS_INFINITY__ 1
// TCE:#define __DBL_HAS_QUIET_NAN__ 1
// TCE:#define __DBL_MANT_DIG__ 24
// TCE:#define __DBL_MAX_10_EXP__ 38
// TCE:#define __DBL_MAX_EXP__ 128
// TCE:#define __DBL_MAX__ 3.40282347e+38
// TCE:#define __DBL_MIN_10_EXP__ (-37)
// TCE:#define __DBL_MIN_EXP__ (-125)
// TCE:#define __DBL_MIN__ 1.17549435e-38
// TCE:#define __DECIMAL_DIG__ -1
// TCE:#define __FLT_DENORM_MIN__ 1.40129846e-45F
// TCE:#define __FLT_DIG__ 6
// TCE:#define __FLT_EPSILON__ 1.19209290e-7F
// TCE:#define __FLT_EVAL_METHOD__ 0
// TCE:#define __FLT_HAS_DENORM__ 1
// TCE:#define __FLT_HAS_INFINITY__ 1
// TCE:#define __FLT_HAS_QUIET_NAN__ 1
// TCE:#define __FLT_MANT_DIG__ 24
// TCE:#define __FLT_MAX_10_EXP__ 38
// TCE:#define __FLT_MAX_EXP__ 128
// TCE:#define __FLT_MAX__ 3.40282347e+38F
// TCE:#define __FLT_MIN_10_EXP__ (-37)
// TCE:#define __FLT_MIN_EXP__ (-125)
// TCE:#define __FLT_MIN__ 1.17549435e-38F
// TCE:#define __FLT_RADIX__ 2
// TCE:#define __INT16_TYPE__ short
// TCE:#define __INT32_TYPE__ int
// TCE:#define __INT8_TYPE__ char
// TCE:#define __INTMAX_MAX__ 2147483647L
// TCE:#define __INTMAX_TYPE__ long int
// TCE:#define __INTMAX_WIDTH__ 32
// TCE:#define __INTPTR_TYPE__ int
// TCE:#define __INTPTR_WIDTH__ 32
// TCE:#define __INT_MAX__ 2147483647
// TCE:#define __LDBL_DENORM_MIN__ 1.40129846e-45L
// TCE:#define __LDBL_DIG__ 6
// TCE:#define __LDBL_EPSILON__ 1.19209290e-7L
// TCE:#define __LDBL_HAS_DENORM__ 1
// TCE:#define __LDBL_HAS_INFINITY__ 1
// TCE:#define __LDBL_HAS_QUIET_NAN__ 1
// TCE:#define __LDBL_MANT_DIG__ 24
// TCE:#define __LDBL_MAX_10_EXP__ 38
// TCE:#define __LDBL_MAX_EXP__ 128
// TCE:#define __LDBL_MAX__ 3.40282347e+38L
// TCE:#define __LDBL_MIN_10_EXP__ (-37)
// TCE:#define __LDBL_MIN_EXP__ (-125)
// TCE:#define __LDBL_MIN__ 1.17549435e-38L
// TCE:#define __LONG_LONG_MAX__ 2147483647LL
// TCE:#define __LONG_MAX__ 2147483647L
// TCE-NOT:#define __LP64__
// TCE:#define __POINTER_WIDTH__ 32
// TCE:#define __PTRDIFF_TYPE__ int
// TCE:#define __PTRDIFF_WIDTH__ 32
// TCE:#define __SCHAR_MAX__ 127
// TCE:#define __SHRT_MAX__ 32767
// TCE:#define __SIG_ATOMIC_WIDTH__ 32
// TCE:#define __SIZEOF_DOUBLE__ 4
// TCE:#define __SIZEOF_FLOAT__ 4
// TCE:#define __SIZEOF_INT__ 4
// TCE:#define __SIZEOF_LONG_DOUBLE__ 4
// TCE:#define __SIZEOF_LONG_LONG__ 4
// TCE:#define __SIZEOF_LONG__ 4
// TCE:#define __SIZEOF_POINTER__ 4
// TCE:#define __SIZEOF_PTRDIFF_T__ 4
// TCE:#define __SIZEOF_SHORT__ 2
// TCE:#define __SIZEOF_SIZE_T__ 4
// TCE:#define __SIZEOF_WCHAR_T__ 4
// TCE:#define __SIZEOF_WINT_T__ 4
// TCE:#define __SIZE_MAX__ 4294967295U
// TCE:#define __SIZE_TYPE__ unsigned int
// TCE:#define __SIZE_WIDTH__ 32
// TCE:#define __TCE_V1__ 1
// TCE:#define __TCE__ 1
// TCE:#define __UINTMAX_TYPE__ long unsigned int
// TCE:#define __USER_LABEL_PREFIX__ _
// TCE:#define __WCHAR_MAX__ 2147483647
// TCE:#define __WCHAR_TYPE__ int
// TCE:#define __WCHAR_WIDTH__ 32
// TCE:#define __WINT_TYPE__ int
// TCE:#define __WINT_WIDTH__ 32
// TCE:#define __tce 1
// TCE:#define __tce__ 1
// TCE:#define tce 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=x86_64-none-none < /dev/null | FileCheck -check-prefix X86_64 %s
//
// X86_64:#define _LP64 1
// X86_64:#define __BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__
// X86_64:#define __CHAR16_TYPE__ unsigned short
// X86_64:#define __CHAR32_TYPE__ unsigned int
// X86_64:#define __CHAR_BIT__ 8
// X86_64:#define __DBL_DENORM_MIN__ 4.9406564584124654e-324
// X86_64:#define __DBL_DIG__ 15
// X86_64:#define __DBL_EPSILON__ 2.2204460492503131e-16
// X86_64:#define __DBL_HAS_DENORM__ 1
// X86_64:#define __DBL_HAS_INFINITY__ 1
// X86_64:#define __DBL_HAS_QUIET_NAN__ 1
// X86_64:#define __DBL_MANT_DIG__ 53
// X86_64:#define __DBL_MAX_10_EXP__ 308
// X86_64:#define __DBL_MAX_EXP__ 1024
// X86_64:#define __DBL_MAX__ 1.7976931348623157e+308
// X86_64:#define __DBL_MIN_10_EXP__ (-307)
// X86_64:#define __DBL_MIN_EXP__ (-1021)
// X86_64:#define __DBL_MIN__ 2.2250738585072014e-308
// X86_64:#define __DECIMAL_DIG__ 21
// X86_64:#define __FLT_DENORM_MIN__ 1.40129846e-45F
// X86_64:#define __FLT_DIG__ 6
// X86_64:#define __FLT_EPSILON__ 1.19209290e-7F
// X86_64:#define __FLT_EVAL_METHOD__ 0
// X86_64:#define __FLT_HAS_DENORM__ 1
// X86_64:#define __FLT_HAS_INFINITY__ 1
// X86_64:#define __FLT_HAS_QUIET_NAN__ 1
// X86_64:#define __FLT_MANT_DIG__ 24
// X86_64:#define __FLT_MAX_10_EXP__ 38
// X86_64:#define __FLT_MAX_EXP__ 128
// X86_64:#define __FLT_MAX__ 3.40282347e+38F
// X86_64:#define __FLT_MIN_10_EXP__ (-37)
// X86_64:#define __FLT_MIN_EXP__ (-125)
// X86_64:#define __FLT_MIN__ 1.17549435e-38F
// X86_64:#define __FLT_RADIX__ 2
// X86_64:#define __INT16_TYPE__ short
// X86_64:#define __INT32_TYPE__ int
// X86_64:#define __INT64_C_SUFFIX__ L
// X86_64:#define __INT64_TYPE__ long int
// X86_64:#define __INT8_TYPE__ char
// X86_64:#define __INTMAX_MAX__ 9223372036854775807L
// X86_64:#define __INTMAX_TYPE__ long int
// X86_64:#define __INTMAX_WIDTH__ 64
// X86_64:#define __INTPTR_TYPE__ long int
// X86_64:#define __INTPTR_WIDTH__ 64
// X86_64:#define __INT_MAX__ 2147483647
// X86_64:#define __LDBL_DENORM_MIN__ 3.64519953188247460253e-4951L
// X86_64:#define __LDBL_DIG__ 18
// X86_64:#define __LDBL_EPSILON__ 1.08420217248550443401e-19L
// X86_64:#define __LDBL_HAS_DENORM__ 1
// X86_64:#define __LDBL_HAS_INFINITY__ 1
// X86_64:#define __LDBL_HAS_QUIET_NAN__ 1
// X86_64:#define __LDBL_MANT_DIG__ 64
// X86_64:#define __LDBL_MAX_10_EXP__ 4932
// X86_64:#define __LDBL_MAX_EXP__ 16384
// X86_64:#define __LDBL_MAX__ 1.18973149535723176502e+4932L
// X86_64:#define __LDBL_MIN_10_EXP__ (-4931)
// X86_64:#define __LDBL_MIN_EXP__ (-16381)
// X86_64:#define __LDBL_MIN__ 3.36210314311209350626e-4932L
// X86_64:#define __LITTLE_ENDIAN__ 1
// X86_64:#define __LONG_LONG_MAX__ 9223372036854775807LL
// X86_64:#define __LONG_MAX__ 9223372036854775807L
// X86_64:#define __LP64__ 1
// X86_64:#define __MMX__ 1
// X86_64:#define __NO_MATH_INLINES 1
// X86_64:#define __POINTER_WIDTH__ 64
// X86_64:#define __PTRDIFF_TYPE__ long int
// X86_64:#define __PTRDIFF_WIDTH__ 64
// X86_64:#define __REGISTER_PREFIX__ 
// X86_64:#define __SCHAR_MAX__ 127
// X86_64:#define __SHRT_MAX__ 32767
// X86_64:#define __SIG_ATOMIC_WIDTH__ 32
// X86_64:#define __SIZEOF_DOUBLE__ 8
// X86_64:#define __SIZEOF_FLOAT__ 4
// X86_64:#define __SIZEOF_INT__ 4
// X86_64:#define __SIZEOF_LONG_DOUBLE__ 16
// X86_64:#define __SIZEOF_LONG_LONG__ 8
// X86_64:#define __SIZEOF_LONG__ 8
// X86_64:#define __SIZEOF_POINTER__ 8
// X86_64:#define __SIZEOF_PTRDIFF_T__ 8
// X86_64:#define __SIZEOF_SHORT__ 2
// X86_64:#define __SIZEOF_SIZE_T__ 8
// X86_64:#define __SIZEOF_WCHAR_T__ 4
// X86_64:#define __SIZEOF_WINT_T__ 4
// X86_64:#define __SIZE_MAX__ 18446744073709551615UL
// X86_64:#define __SIZE_TYPE__ long unsigned int
// X86_64:#define __SIZE_WIDTH__ 64
// X86_64:#define __SSE2_MATH__ 1
// X86_64:#define __SSE2__ 1
// X86_64:#define __SSE_MATH__ 1
// X86_64:#define __SSE__ 1
// X86_64:#define __UINTMAX_TYPE__ long unsigned int
// X86_64:#define __USER_LABEL_PREFIX__ _
// X86_64:#define __WCHAR_MAX__ 2147483647
// X86_64:#define __WCHAR_TYPE__ int
// X86_64:#define __WCHAR_WIDTH__ 32
// X86_64:#define __WINT_TYPE__ int
// X86_64:#define __WINT_WIDTH__ 32
// X86_64:#define __amd64 1
// X86_64:#define __amd64__ 1
// X86_64:#define __x86_64 1
// X86_64:#define __x86_64__ 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=x86_64-pc-linux-gnu < /dev/null | FileCheck -check-prefix X86_64-LINUX %s
//
// X86_64-LINUX:#define _LP64 1
// X86_64-LINUX:#define __BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__
// X86_64-LINUX:#define __CHAR16_TYPE__ unsigned short
// X86_64-LINUX:#define __CHAR32_TYPE__ unsigned int
// X86_64-LINUX:#define __CHAR_BIT__ 8
// X86_64-LINUX:#define __DBL_DENORM_MIN__ 4.9406564584124654e-324
// X86_64-LINUX:#define __DBL_DIG__ 15
// X86_64-LINUX:#define __DBL_EPSILON__ 2.2204460492503131e-16
// X86_64-LINUX:#define __DBL_HAS_DENORM__ 1
// X86_64-LINUX:#define __DBL_HAS_INFINITY__ 1
// X86_64-LINUX:#define __DBL_HAS_QUIET_NAN__ 1
// X86_64-LINUX:#define __DBL_MANT_DIG__ 53
// X86_64-LINUX:#define __DBL_MAX_10_EXP__ 308
// X86_64-LINUX:#define __DBL_MAX_EXP__ 1024
// X86_64-LINUX:#define __DBL_MAX__ 1.7976931348623157e+308
// X86_64-LINUX:#define __DBL_MIN_10_EXP__ (-307)
// X86_64-LINUX:#define __DBL_MIN_EXP__ (-1021)
// X86_64-LINUX:#define __DBL_MIN__ 2.2250738585072014e-308
// X86_64-LINUX:#define __DECIMAL_DIG__ 21
// X86_64-LINUX:#define __FLT_DENORM_MIN__ 1.40129846e-45F
// X86_64-LINUX:#define __FLT_DIG__ 6
// X86_64-LINUX:#define __FLT_EPSILON__ 1.19209290e-7F
// X86_64-LINUX:#define __FLT_EVAL_METHOD__ 0
// X86_64-LINUX:#define __FLT_HAS_DENORM__ 1
// X86_64-LINUX:#define __FLT_HAS_INFINITY__ 1
// X86_64-LINUX:#define __FLT_HAS_QUIET_NAN__ 1
// X86_64-LINUX:#define __FLT_MANT_DIG__ 24
// X86_64-LINUX:#define __FLT_MAX_10_EXP__ 38
// X86_64-LINUX:#define __FLT_MAX_EXP__ 128
// X86_64-LINUX:#define __FLT_MAX__ 3.40282347e+38F
// X86_64-LINUX:#define __FLT_MIN_10_EXP__ (-37)
// X86_64-LINUX:#define __FLT_MIN_EXP__ (-125)
// X86_64-LINUX:#define __FLT_MIN__ 1.17549435e-38F
// X86_64-LINUX:#define __FLT_RADIX__ 2
// X86_64-LINUX:#define __INT16_TYPE__ short
// X86_64-LINUX:#define __INT32_TYPE__ int
// X86_64-LINUX:#define __INT64_C_SUFFIX__ L
// X86_64-LINUX:#define __INT64_TYPE__ long int
// X86_64-LINUX:#define __INT8_TYPE__ char
// X86_64-LINUX:#define __INTMAX_MAX__ 9223372036854775807L
// X86_64-LINUX:#define __INTMAX_TYPE__ long int
// X86_64-LINUX:#define __INTMAX_WIDTH__ 64
// X86_64-LINUX:#define __INTPTR_TYPE__ long int
// X86_64-LINUX:#define __INTPTR_WIDTH__ 64
// X86_64-LINUX:#define __INT_MAX__ 2147483647
// X86_64-LINUX:#define __LDBL_DENORM_MIN__ 3.64519953188247460253e-4951L
// X86_64-LINUX:#define __LDBL_DIG__ 18
// X86_64-LINUX:#define __LDBL_EPSILON__ 1.08420217248550443401e-19L
// X86_64-LINUX:#define __LDBL_HAS_DENORM__ 1
// X86_64-LINUX:#define __LDBL_HAS_INFINITY__ 1
// X86_64-LINUX:#define __LDBL_HAS_QUIET_NAN__ 1
// X86_64-LINUX:#define __LDBL_MANT_DIG__ 64
// X86_64-LINUX:#define __LDBL_MAX_10_EXP__ 4932
// X86_64-LINUX:#define __LDBL_MAX_EXP__ 16384
// X86_64-LINUX:#define __LDBL_MAX__ 1.18973149535723176502e+4932L
// X86_64-LINUX:#define __LDBL_MIN_10_EXP__ (-4931)
// X86_64-LINUX:#define __LDBL_MIN_EXP__ (-16381)
// X86_64-LINUX:#define __LDBL_MIN__ 3.36210314311209350626e-4932L
// X86_64-LINUX:#define __LITTLE_ENDIAN__ 1
// X86_64-LINUX:#define __LONG_LONG_MAX__ 9223372036854775807LL
// X86_64-LINUX:#define __LONG_MAX__ 9223372036854775807L
// X86_64-LINUX:#define __LP64__ 1
// X86_64-LINUX:#define __MMX__ 1
// X86_64-LINUX:#define __NO_MATH_INLINES 1
// X86_64-LINUX:#define __POINTER_WIDTH__ 64
// X86_64-LINUX:#define __PTRDIFF_TYPE__ long int
// X86_64-LINUX:#define __PTRDIFF_WIDTH__ 64
// X86_64-LINUX:#define __REGISTER_PREFIX__ 
// X86_64-LINUX:#define __SCHAR_MAX__ 127
// X86_64-LINUX:#define __SHRT_MAX__ 32767
// X86_64-LINUX:#define __SIG_ATOMIC_WIDTH__ 32
// X86_64-LINUX:#define __SIZEOF_DOUBLE__ 8
// X86_64-LINUX:#define __SIZEOF_FLOAT__ 4
// X86_64-LINUX:#define __SIZEOF_INT__ 4
// X86_64-LINUX:#define __SIZEOF_LONG_DOUBLE__ 16
// X86_64-LINUX:#define __SIZEOF_LONG_LONG__ 8
// X86_64-LINUX:#define __SIZEOF_LONG__ 8
// X86_64-LINUX:#define __SIZEOF_POINTER__ 8
// X86_64-LINUX:#define __SIZEOF_PTRDIFF_T__ 8
// X86_64-LINUX:#define __SIZEOF_SHORT__ 2
// X86_64-LINUX:#define __SIZEOF_SIZE_T__ 8
// X86_64-LINUX:#define __SIZEOF_WCHAR_T__ 4
// X86_64-LINUX:#define __SIZEOF_WINT_T__ 4
// X86_64-LINUX:#define __SIZE_MAX__ 18446744073709551615UL
// X86_64-LINUX:#define __SIZE_TYPE__ long unsigned int
// X86_64-LINUX:#define __SIZE_WIDTH__ 64
// X86_64-LINUX:#define __SSE2_MATH__ 1
// X86_64-LINUX:#define __SSE2__ 1
// X86_64-LINUX:#define __SSE_MATH__ 1
// X86_64-LINUX:#define __SSE__ 1
// X86_64-LINUX:#define __UINTMAX_TYPE__ long unsigned int
// X86_64-LINUX:#define __USER_LABEL_PREFIX__
// X86_64-LINUX:#define __WCHAR_MAX__ 2147483647
// X86_64-LINUX:#define __WCHAR_TYPE__ int
// X86_64-LINUX:#define __WCHAR_WIDTH__ 32
// X86_64-LINUX:#define __WINT_TYPE__ unsigned int
// X86_64-LINUX:#define __WINT_WIDTH__ 32
// X86_64-LINUX:#define __amd64 1
// X86_64-LINUX:#define __amd64__ 1
// X86_64-LINUX:#define __x86_64 1
// X86_64-LINUX:#define __x86_64__ 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=x86_64-unknown-freebsd9.1 < /dev/null | FileCheck -check-prefix X86_64-FREEBSD %s
//
// X86_64-FREEBSD:#define __FreeBSD__ 9
// X86_64-FREEBSD:#define __FreeBSD_cc_version 900001
// X86_64-FREEBSD:#define __STDC_MB_MIGHT_NEQ_WC__ 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=x86_64-netbsd < /dev/null | FileCheck -check-prefix X86_64-NETBSD %s
//
// X86_64-NETBSD:#define _LP64 1
// X86_64-NETBSD:#define __BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__
// X86_64-NETBSD:#define __CHAR16_TYPE__ unsigned short
// X86_64-NETBSD:#define __CHAR32_TYPE__ unsigned int
// X86_64-NETBSD:#define __CHAR_BIT__ 8
// X86_64-NETBSD:#define __DBL_DENORM_MIN__ 4.9406564584124654e-324
// X86_64-NETBSD:#define __DBL_DIG__ 15
// X86_64-NETBSD:#define __DBL_EPSILON__ 2.2204460492503131e-16
// X86_64-NETBSD:#define __DBL_HAS_DENORM__ 1
// X86_64-NETBSD:#define __DBL_HAS_INFINITY__ 1
// X86_64-NETBSD:#define __DBL_HAS_QUIET_NAN__ 1
// X86_64-NETBSD:#define __DBL_MANT_DIG__ 53
// X86_64-NETBSD:#define __DBL_MAX_10_EXP__ 308
// X86_64-NETBSD:#define __DBL_MAX_EXP__ 1024
// X86_64-NETBSD:#define __DBL_MAX__ 1.7976931348623157e+308
// X86_64-NETBSD:#define __DBL_MIN_10_EXP__ (-307)
// X86_64-NETBSD:#define __DBL_MIN_EXP__ (-1021)
// X86_64-NETBSD:#define __DBL_MIN__ 2.2250738585072014e-308
// X86_64-NETBSD:#define __DECIMAL_DIG__ 21
// X86_64-NETBSD:#define __FLT_DENORM_MIN__ 1.40129846e-45F
// X86_64-NETBSD:#define __FLT_DIG__ 6
// X86_64-NETBSD:#define __FLT_EPSILON__ 1.19209290e-7F
// X86_64-NETBSD:#define __FLT_EVAL_METHOD__ 0
// X86_64-NETBSD:#define __FLT_HAS_DENORM__ 1
// X86_64-NETBSD:#define __FLT_HAS_INFINITY__ 1
// X86_64-NETBSD:#define __FLT_HAS_QUIET_NAN__ 1
// X86_64-NETBSD:#define __FLT_MANT_DIG__ 24
// X86_64-NETBSD:#define __FLT_MAX_10_EXP__ 38
// X86_64-NETBSD:#define __FLT_MAX_EXP__ 128
// X86_64-NETBSD:#define __FLT_MAX__ 3.40282347e+38F
// X86_64-NETBSD:#define __FLT_MIN_10_EXP__ (-37)
// X86_64-NETBSD:#define __FLT_MIN_EXP__ (-125)
// X86_64-NETBSD:#define __FLT_MIN__ 1.17549435e-38F
// X86_64-NETBSD:#define __FLT_RADIX__ 2
// X86_64-NETBSD:#define __INT16_TYPE__ short
// X86_64-NETBSD:#define __INT32_TYPE__ int
// X86_64-NETBSD:#define __INT64_C_SUFFIX__ L
// X86_64-NETBSD:#define __INT64_TYPE__ long int
// X86_64-NETBSD:#define __INT8_TYPE__ char
// X86_64-NETBSD:#define __INTMAX_MAX__ 9223372036854775807L
// X86_64-NETBSD:#define __INTMAX_TYPE__ long int
// X86_64-NETBSD:#define __INTMAX_WIDTH__ 64
// X86_64-NETBSD:#define __INTPTR_TYPE__ long int
// X86_64-NETBSD:#define __INTPTR_WIDTH__ 64
// X86_64-NETBSD:#define __INT_MAX__ 2147483647
// X86_64-NETBSD:#define __LDBL_DENORM_MIN__ 3.64519953188247460253e-4951L
// X86_64-NETBSD:#define __LDBL_DIG__ 18
// X86_64-NETBSD:#define __LDBL_EPSILON__ 1.08420217248550443401e-19L
// X86_64-NETBSD:#define __LDBL_HAS_DENORM__ 1
// X86_64-NETBSD:#define __LDBL_HAS_INFINITY__ 1
// X86_64-NETBSD:#define __LDBL_HAS_QUIET_NAN__ 1
// X86_64-NETBSD:#define __LDBL_MANT_DIG__ 64
// X86_64-NETBSD:#define __LDBL_MAX_10_EXP__ 4932
// X86_64-NETBSD:#define __LDBL_MAX_EXP__ 16384
// X86_64-NETBSD:#define __LDBL_MAX__ 1.18973149535723176502e+4932L
// X86_64-NETBSD:#define __LDBL_MIN_10_EXP__ (-4931)
// X86_64-NETBSD:#define __LDBL_MIN_EXP__ (-16381)
// X86_64-NETBSD:#define __LDBL_MIN__ 3.36210314311209350626e-4932L
// X86_64-NETBSD:#define __LITTLE_ENDIAN__ 1
// X86_64-NETBSD:#define __LONG_LONG_MAX__ 9223372036854775807LL
// X86_64-NETBSD:#define __LONG_MAX__ 9223372036854775807L
// X86_64-NETBSD:#define __LP64__ 1
// X86_64-NETBSD:#define __MMX__ 1
// X86_64-NETBSD:#define __NO_MATH_INLINES 1
// X86_64-NETBSD:#define __POINTER_WIDTH__ 64
// X86_64-NETBSD:#define __PTRDIFF_TYPE__ long int
// X86_64-NETBSD:#define __PTRDIFF_WIDTH__ 64
// X86_64-NETBSD:#define __REGISTER_PREFIX__ 
// X86_64-NETBSD:#define __SCHAR_MAX__ 127
// X86_64-NETBSD:#define __SHRT_MAX__ 32767
// X86_64-NETBSD:#define __SIG_ATOMIC_WIDTH__ 32
// X86_64-NETBSD:#define __SIZEOF_DOUBLE__ 8
// X86_64-NETBSD:#define __SIZEOF_FLOAT__ 4
// X86_64-NETBSD:#define __SIZEOF_INT__ 4
// X86_64-NETBSD:#define __SIZEOF_LONG_DOUBLE__ 16
// X86_64-NETBSD:#define __SIZEOF_LONG_LONG__ 8
// X86_64-NETBSD:#define __SIZEOF_LONG__ 8
// X86_64-NETBSD:#define __SIZEOF_POINTER__ 8
// X86_64-NETBSD:#define __SIZEOF_PTRDIFF_T__ 8
// X86_64-NETBSD:#define __SIZEOF_SHORT__ 2
// X86_64-NETBSD:#define __SIZEOF_SIZE_T__ 8
// X86_64-NETBSD:#define __SIZEOF_WCHAR_T__ 4
// X86_64-NETBSD:#define __SIZEOF_WINT_T__ 4
// X86_64-NETBSD:#define __SIZE_MAX__ 18446744073709551615UL
// X86_64-NETBSD:#define __SIZE_TYPE__ long unsigned int
// X86_64-NETBSD:#define __SIZE_WIDTH__ 64
// X86_64-NETBSD:#define __SSE2_MATH__ 1
// X86_64-NETBSD:#define __SSE2__ 1
// X86_64-NETBSD:#define __SSE_MATH__ 1
// X86_64-NETBSD:#define __SSE__ 1
// X86_64-NETBSD:#define __UINTMAX_TYPE__ long unsigned int
// X86_64-NETBSD:#define __USER_LABEL_PREFIX__
// X86_64-NETBSD:#define __WCHAR_MAX__ 2147483647
// X86_64-NETBSD:#define __WCHAR_TYPE__ int
// X86_64-NETBSD:#define __WCHAR_WIDTH__ 32
// X86_64-NETBSD:#define __WINT_TYPE__ int
// X86_64-NETBSD:#define __WINT_WIDTH__ 32
// X86_64-NETBSD:#define __amd64 1
// X86_64-NETBSD:#define __amd64__ 1
// X86_64-NETBSD:#define __x86_64 1
// X86_64-NETBSD:#define __x86_64__ 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=sparc64-none-none < /dev/null | FileCheck -check-prefix SPARCV9 %s
// SPARCV9:#define __INT64_TYPE__ long int
// SPARCV9:#define __INTMAX_TYPE__ long int
// SPARCV9:#define __INTPTR_TYPE__ long int
// SPARCV9:#define __LONG_MAX__ 9223372036854775807L
// SPARCV9:#define __LP64__ 1
// SPARCV9:#define __SIZEOF_LONG__ 8
// SPARCV9:#define __SIZEOF_POINTER__ 8
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=sparc64-none-openbsd < /dev/null | FileCheck -check-prefix SPARC64-OBSD %s
// SPARC64-OBSD:#define __INT64_TYPE__ long long int
// SPARC64-OBSD:#define __INTMAX_TYPE__ long long int
// SPARC64-OBSD:#define __UINTMAX_TYPE__ long long unsigned int
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=x86_64-pc-kfreebsd-gnu < /dev/null | FileCheck -check-prefix KFREEBSD-DEFINE %s
// KFREEBSD-DEFINE:#define __FreeBSD_kernel__ 1
// KFREEBSD-DEFINE:#define __GLIBC__ 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=i686-pc-kfreebsd-gnu < /dev/null | FileCheck -check-prefix KFREEBSDI686-DEFINE %s
// KFREEBSDI686-DEFINE:#define __FreeBSD_kernel__ 1
// KFREEBSDI686-DEFINE:#define __GLIBC__ 1
//
// RUN: %clang_cc1 -x c++ -triple i686-pc-linux-gnu -fobjc-runtime=gcc -E -dM < /dev/null | FileCheck -check-prefix GNUSOURCE %s
// GNUSOURCE:#define _GNU_SOURCE 1
//
// RUN: %clang_cc1 -x c++ -std=c++98 -fno-rtti -E -dM < /dev/null | FileCheck -check-prefix NORTTI %s
// NORTTI: __GXX_ABI_VERSION
// NORTTI-NOT:#define __GXX_RTTI
// NORTTI: __STDC__
//
// RUN: %clang_cc1 -triple arm-linux-androideabi -E -dM < /dev/null | FileCheck -check-prefix ANDROID %s
// ANDROID: __ANDROID__ 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=powerpc64-unknown-freebsd < /dev/null | FileCheck -check-prefix PPC64-FREEBSD %s
// PPC64-FREEBSD-NOT: #define __LONG_DOUBLE_128__ 1
//
// RUN: %clang_cc1 -E -dM -ffreestanding -triple=xcore-none-none < /dev/null | FileCheck -check-prefix XCORE %s
// XCORE:#define __XS1B__ 1
