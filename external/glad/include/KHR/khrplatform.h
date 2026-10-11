#ifndef __khrplatform_h_
#define __khrplatform_h_

/*
** Copyright (c) 2008-2018 The Khronos Group Inc.
**
** Permission is hereby granted, free of charge, to any person obtaining a
** copy of this software and/or associated documentation files (the
** "Materials"), to deal in the Materials without restriction, including
** without limitation the rights to use, copy, modify, merge, publish,
** distribute, sublicense, and/or sell copies of the Materials, and to
** permit persons to whom the Materials are furnished to do so, subject to
** the following conditions:
**
** The above copyright notice and this permission notice shall be included
** in all copies or substantial portions of the Materials.
**
** THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
** CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
** TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
** MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
*/

#if defined(_WIN32) && !defined(__SC__) && !defined(__INTEL_COMPILER)
    #define KHRONOS_APICALL __declspec(dllimport)
    #define KHRONOS_APIENTRY __stdcall
#else
    #define KHRONOS_APICALL
    #define KHRONOS_APIENTRY
#endif

#if defined (__ARM_PCS_VFP)
    #define KHRONOS_APIATTRIBUTES __attribute__((pcs("aapcs-vfp")))
#else
    #define KHRONOS_APIATTRIBUTES
#endif

#if defined(_MSC_VER) && !defined(__clang__)
    typedef signed   __int8         khronos_int8_t;
    typedef unsigned __int8         khronos_uint8_t;
    typedef signed   __int16        khronos_int16_t;
    typedef unsigned __int16        khronos_uint16_t;
    typedef signed   __int32        khronos_int32_t;
    typedef unsigned __int32        khronos_uint32_t;
    typedef signed   __int64        khronos_int64_t;
    typedef unsigned __int64        khronos_uint64_t;
    #ifdef _WIN64
        typedef signed   __int64    khronos_intptr_t;
        typedef unsigned __int64    khronos_uintptr_t;
        typedef signed   __int64    khronos_ssize_t;
        typedef unsigned __int64    khronos_usize_t;
    #else
        typedef signed   __int32    khronos_intptr_t;
        typedef unsigned __int32    khronos_uintptr_t;
        typedef signed   __int32    khronos_ssize_t;
        typedef unsigned __int32    khronos_usize_t;
    #endif
#else
    #include <stdint.h>
    typedef int8_t                  khronos_int8_t;
    typedef uint8_t                 khronos_uint8_t;
    typedef int16_t                 khronos_int16_t;
    typedef uint16_t                khronos_uint16_t;
    typedef int32_t                 khronos_int32_t;
    typedef uint32_t                khronos_uint32_t;
    typedef int64_t                 khronos_int64_t;
    typedef uint64_t                khronos_uint64_t;
    typedef intptr_t                khronos_intptr_t;
    typedef uintptr_t               khronos_uintptr_t;
    typedef ptrdiff_t               khronos_ssize_t;
    typedef size_t                  khronos_usize_t;
#endif

typedef float                   khronos_float_t;
typedef khronos_int32_t         khronos_time_nsec_t;
typedef khronos_uint64_t        khronos_utime_nanoseconds_t;

#define KHRONOS_BOOLEAN_TRUE    1
#define KHRONOS_BOOLEAN_FALSE   0

#endif /* __khrplatform_h_ */
