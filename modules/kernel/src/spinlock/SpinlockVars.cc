//===================================================================================================================
//
//  SpinlockVars.cc -- Spinlock Global Variables
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Mar-18  Initial   0.3.2   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "spinlock.h"


//
// -- This is a count of the number of locks that are currently held
//    --------------------------------------------------------------
__krndata int locksHeld = 0;


//
// -- The above counter needs a lock as well to prevent race conditions
//    -----------------------------------------------------------------
__krndata Spinlock_t lockCounterLock;

//
// -- This is a lock for use when initializing a stack
//    ------------------------------------------------
__krndata Spinlock_t mmuStackInitLock;


//
// -- This is a lock for use when initializing a level 2 table for the mmu
//    --------------------------------------------------------------------
__krndata Spinlock_t mmuTableInitLock;
