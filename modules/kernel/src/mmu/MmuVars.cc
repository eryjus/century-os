//===================================================================================================================
//
//  MmuVars.cc -- Common variables for the MMU
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Dec-22  Initial  v0.5.0b  ADCL  Initial Version
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "spinlock.h"
#include "mmu.h"


//
// -- This spinlock is used to control access to the address space to clear the frame
//    -------------------------------------------------------------------------------
EXPORT KERNEL_BSS
Spinlock_t frameClearLock;


//
// -- This is used to control the flushes for the TLB buffer
//    ------------------------------------------------------
EXPORT KERNEL_BSS
TlbFlush_t tlbFlush;


//
// -- This is the lock used for MMU_TOP_TABLE_FROM and MMU_TOP_TABLE_TO
//    -----------------------------------------------------------------
EXPORT KERNEL_BSS
Spinlock_t mmuCopyLock;


