//===================================================================================================================
//
//  stacks.h -- Some helpers to managing kernel stacks
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  There are several kernel stack locations that need to be managed.  These will all use the same address space.
//  These functions will assist in this.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Dec-01  Initial   0.4.6d  ADCL  Initial version
//
//===================================================================================================================


#pragma once


#include "types.h"
#include "cpu.h"
#include "spinlock.h"


//
// -- The number of stacks we will manage, divided into 32-bit dwords
//    Notice the `+ 31`.. this will take care of rounding partial dwords
//    ------------------------------------------------------------------
#define STACK_COUNT     ((((4 * 1024 * 1024) / STACK_SIZE) + 31) / 32)


//
// -- This will be the bitmat we will use to keep track of the stacks
//    ---------------------------------------------------------------
EXTERN EXPORT KERNEL_DATA
uint32_t stacks[STACK_COUNT];


//
// -- This is the lock that will protect the bitmap
//    ---------------------------------------------
EXTERN EXPORT KERNEL_DATA
Spinlock_t stackBitmapLock;


//
// -- Allocate a stack
//    ----------------
EXTERN_C EXPORT KERNEL
void StackDoAlloc(archsize_t stackBase);

EXPORT INLINE
void StackAlloc(archsize_t stackBase) {
    archsize_t flags = SPINLOCK_BLOCK_NO_INT(stackBitmapLock) {
        StackDoAlloc(stackBase);
        SPINLOCK_RLS_RESTORE_INT(stackBitmapLock, flags);
    }
}


//
// -- Release a stack
//    ---------------
EXTERN_C EXPORT KERNEL
void StackRelease(archsize_t stackBase);


//
// -- Find an available stack
//    -----------------------
EXTERN_C EXPORT KERNEL
archsize_t StackFind(void);

