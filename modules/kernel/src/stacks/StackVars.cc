//===================================================================================================================
//
//  StackVars.cc -- Kernel Stack Cariables
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Dec-01  Initial   0.4.6d  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "stacks.h"


//
// -- This will be the bitmat we will use to keep track of the stacks
//    ---------------------------------------------------------------
EXPORT KERNEL_DATA uint32_t stacks[STACK_COUNT] = {0};


//
// -- This is the lock that will protect the bitmap
//    ---------------------------------------------
EXPORT KERNEL_DATA Spinlock_t stackBitmapLock = {0};



