//===================================================================================================================
//
//  atomic.h -- These are atomic integer type and functions
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  Sometimes a lock is a little heavy-handed for handling a simple integer operation.  An Atomic Integer is a
//  set of operations that are guaranteed to be completed atomically -- or in one cpu instruction.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Apr-28  Initial   0.4.2   ADCL  Initial version
//
//===================================================================================================================


#ifndef __ATOMIC_H__
#define __ATOMIC_H__


#include "types.h"


//
// -- this is the atomic integer type
//    -------------------------------
typedef struct AtomicInt_t {
    int32_t counter;
} AtomicInt_t;


//
// -- Initialize in AtomicInt_t at compile time
//    -----------------------------------------
#define ATOMIC_INT_INIT(n)      { (n) }


//
// -- Set an AtomicInt_t to a value, returning the previous value
//    -----------------------------------------------------------
__CENTURY_FUNC__ int32_t AtomicSet(AtomicInt_t *a, int32_t v);


//
// -- Add a value to an AtomicInt_t, returning the previous value
//    -----------------------------------------------------------
__CENTURY_FUNC__ int32_t AtomicAdd(AtomicInt_t *a, int32_t v);


//
// -- Subtract a value from an AtomicInt_t, returning the previous value
//    ------------------------------------------------------------------
static inline int32_t AtomicSub(AtomicInt_t *a, int32_t v) { return AtomicAdd(a, -v); }


//
// -- This function will read an integer atomically (which happens without any special work)
//    --------------------------------------------------------------------------------------
static inline int32_t AtomicRead(AtomicInt_t *a) { return a->counter; }


//
// -- This function will increment a value for an atomic interger
//    -----------------------------------------------------------
static inline int32_t AtomicInc(AtomicInt_t *a) { return AtomicAdd(a, 1); }


//
// -- This function will decrement a value for an atomic integer
//    ----------------------------------------------------------
static inline int32_t AtomicDec(AtomicInt_t *a) { return AtomicSub(a, 1); }


//
// -- This function will atomically add and test if the result is 0
//    -------------------------------------------------------------
static inline bool AtomicAddAndNegative(AtomicInt_t *a, int32_t v) { return (AtomicAdd(a, v) < 0); }


//
// -- This function will atomically subtract and test if the result is 0
//    ------------------------------------------------------------------
static inline bool AtomicSubAndTest(AtomicInt_t *a, int32_t v) { return (AtomicSub(a, v) == 0); }


//
// -- This function will atomically increment and test if the result is 0
//    -------------------------------------------------------------------
static inline bool AtomicIncAndTest(AtomicInt_t *a) { return (AtomicAdd(a, 1) == 0); }


//
// -- This function will atomically decrement and test if the result is 0
//    -------------------------------------------------------------------
static inline bool AtomicDecAndTest(AtomicInt_t *a) { return (AtomicSub(a, 1) == 0); }


#endif

