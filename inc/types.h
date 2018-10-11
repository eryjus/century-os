//===================================================================================================================
//
// inc/types.h -- Common type definitions for all architectures
//
// These types are architecture independent.  In the end, we add the architecture-specific types with the proper
// size.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date     Tracker  Version  Pgmr  Description
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-05-24  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#ifndef __TYPES_H__
#define __TYPES_H__


//
// -- these are the only 2 standard include files that are safe to include
//    --------------------------------------------------------------------
#include <stdint.h>
#include <stddef.h>


//
// -- Define UNUSED, based on which parser we are using
//    -------------------------------------------------
#ifdef UNUSED
#elif defined(__GNUC__)
# define UNUSED(x) x __attribute__((unused))
#elif defined(__LCLINT__)
# define UNUSED(x) /*@unused@*/ x
#else
# define UNUSED(x) x
#endif


/* adapted from http: *research.microsoft.com/... */
typedef char *  va_list;
#define _INTSIZEOF(n)   ((sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1))
#define va_start(ap,v)  (ap = (va_list)&v + _INTSIZEOF(v))
#define va_arg(ap,t)    (*(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)))
#define va_end(ap)      (ap = (va_list)0)


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


#include "arch-types.h"


//
// -- The ISR Handlers
//    ----------------
extern isrFunc_t isrHandlers[256];


#endif
