//===================================================================================================================
//
//  types.h -- Common type definitions for all architectures
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  These types are architecture independent.  In the end, we add the architecture-specific types with the proper
//  size.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-May-24  Initial   0.1.0   ADCL  Initial version
//  2018-Nov-04  Initial   0.1.0   ADCL  Added Compile Time Assertions from
//                                       http://www.pixelbeat.org/programming/gcc/static_assert.html
//  2018-Nov-11  Initial   0.2.0   ADCL  Address architecture abstraction issues
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#pragma once
#define __TYPES_H__


//
// -- these are the only 2 standard include files that are safe to include
//    --------------------------------------------------------------------
#include <stdint.h>
#include <stddef.h>


//
// -- manage the release compiler flag
//    --------------------------------
#if !defined(RELEASE)
#   define RELEASE 0
#else
#   if RELEASE > 1
#       undef RELEASE
#       define RELEASE 1
#   endif
#endif


//
// -- This will set up for a __cfunc -- a custom #define to stop name mangling
//    ------------------------------------------------------------------------
#define __CENTURY_FUNC__         extern "C"


//
// -- these defined are to help with the placement of functions and data elements into the correct sections
//    -----------------------------------------------------------------------------------------------------
#define __ldrtext       __attribute__((section(".ldrtext")))
#define __ldrdata       __attribute__((section(".ldrdata")))
#define __ldrbss        __attribute__((section(".ldrbss")))

#define __krntext       __attribute__((section(".text")))
#define __krndata       __attribute__((section(".data")))


//
// -- some things to add readability/direction to the linker
//    ------------------------------------------------------
#define EXPORT          __attribute__((visibility("default")))
#define HIDDEN          __attribute__((visibility("hidden")))
#define EXTERN          extern
#define EXTERN_C        EXTERN "C"


//
// -- Things that might appear on the ENTRY section
//    ---------------------------------------------
#define ENTRY           __attribute__((section(".text.entry")))
#define ENTRY_DATA      __attribute__((section(".data.entry")))
#define ENTRY_BSS       __attribute__((section(".bss.entry")))

#define KERNEL          __attribute__((section(".text")))
#define KERNEL_DATA     __attribute__((section(".data")))


#define LOADER          __attribute__((section(".ldrtext")))
#define LOADER_DATA     __attribute__((section(".ldrdata")))
#define LOADER_BSS      __attribute__((section(".ldrbss")))


//
// -- Define UNUSED, based on which parser we are using
//    -------------------------------------------------
#ifdef UNUSED
#elif defined(__GNUC__)
#   define UNUSED(x) x __attribute__((unused))
#elif defined(__LCLINT__)
#   define UNUSED(x) /*@unused@*/ x
#else
#   define UNUSED(x) x
#endif


//
// -- some basic macros to help with coding
//    -------------------------------------
#define ABS(x)      ((x)>=0?(x):-(x))
#define MIN(x,y)    ((x)<=(y)?(x):(y))
#define MAX(x,y)    ((x)>=(y)?(x):(y))


/* adapted from http: *research.microsoft.com/... */
typedef char *  va_list;
#define _INTSIZEOF(n)   ((sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1))
#define va_start(ap,v)  (ap = (va_list)&v + _INTSIZEOF(v))
#define va_arg(ap,t)    (*(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)))
#define va_end(ap)      (ap = (va_list)0)


//
// -- Some compiler hints
//    -------------------
#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)


//
// -- Some compile-time assertions to help with size checking!
//    --------------------------------------------------------
/* Note we need the 2 concats below because arguments to ##
 * are not expanded, so we need to expand __LINE__ with one indirection
 * before doing the actual concatenation. */
#define ASSERT_CONCAT_(a, b) a##b
#define ASSERT_CONCAT(a, b) ASSERT_CONCAT_(a, b)
#define ct_assert(e) enum { ASSERT_CONCAT(assert_line_, __LINE__) = 1/(!!(e)) }


//
// -- Some additional runtime assertion checking; purposefully set up for use in conditions
//    -------------------------------------------------------------------------------------
extern "C" {
    EXPORT KERNEL
    bool AssertFailure(const char *expr, const char *msg, const char *file, int line);
}

#ifdef assert
#   undef assert
#endif

#if RELEASE == 1
#   define assert(e) true
#   define assert_msg(e,m) true
#else
#   define assert(e) (likely((e)) ? true : AssertFailure(#e, NULL, __FILE__, __LINE__))
#   define assert_msg(e,m) (likely((e)) ? true : AssertFailure(#e, (m), __FILE__, __LINE__))
#endif


//
// -- Define the types that will be used by the ELF loader
//    ----------------------------------------------------
typedef uint64_t elf64Addr_t;
typedef uint64_t elf64Off_t;

typedef uint32_t elf32Addr_t;
typedef uint32_t elf32Off_t;

typedef int64_t elfSXWord_t;
typedef uint64_t elfXWord_t;
typedef int32_t elfSWord_t;
typedef uint32_t elfWord_t;
typedef uint16_t elfHalf_t;


//
// -- Now include the architecture-specific types
//    -------------------------------------------
#include "arch-types.h"


//
// -- This is a process ID (or PID) -- the same width regardless of arch
//    ------------------------------------------------------------------
typedef uint32_t PID_t;


//
// -- This is the size of a frame for the PMM (which is tied to the address width for this architecture)
//    --------------------------------------------------------------------------------------------------
typedef archsize_t frame_t;


//
// -- This is a generic byte definition
//    ---------------------------------
typedef uint8_t byte_t;


//
// -- The current PID
//    ---------------
extern volatile PID_t currentPID;


//
// -- This is the prototype definition for an ISR handler routine
//    -----------------------------------------------------------
typedef void (*isrFunc_t)(isrRegs_t *);


//
// -- The definition of a NULL ISR Handler Function
//    ---------------------------------------------
const isrFunc_t NULL_ISR = (isrFunc_t)NULL;


//
// -- The ISR Handlers
//    ----------------
extern isrFunc_t isrHandlers[256];


