//===================================================================================================================
//
//  mmu.h -- This is the kernel MMU manager header.
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  All fo these functions are written so that PD[1023] is recursively mapped.  As a result, we know some things
//  are in certain locations.  For example, the Page Directory itself starts at virtual address 0xfffff000, no
//  matter what process we are in.  The Page Tables will be located at 0xffc00000 each.  This then means that in
//  order to manage any given Page Table structure, the calculations are consistent -- which differs from the
//  loader MMU requirements.  In particular, I do not need to know which cr3 I am managing.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  --------------------------------------------------------------------------
//  2018-Nov-10  Initial   0.1.0   ADCL  Initial version
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#pragma once


#define __MMU_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "types.h"
#include "spinlock.h"
#include "arch-mmu.h"


//
// -- Some constants to help with mapping flags
//    -----------------------------------------
enum {
    PG_KRN = 0x00000001,
    PG_WRT = 0x00000002,
    PG_DEVICE = 0x80000000,         // used for ARM
};


//
// -- With the page table structures given, map a virtual address to a physical frame
//    -------------------------------------------------------------------------------
EXTERN_C EXPORT KERNEL
void MmuMapToFrame(archsize_t addr, frame_t frame, int flags);


//
// -- Unmap a page from the page table
//    --------------------------------
EXTERN_C EXPORT KERNEL
frame_t MmuUnmapPage(archsize_t addr);


//
// -- Clear a frame before formally adding it to the paging tables
//    ------------------------------------------------------------
EXTERN_C EXPORT KERNEL
void MmuClearFrame(frame_t frame);


//
// -- Check of the address is mapped
//    ------------------------------
EXTERN_C EXPORT KERNEL
bool MmuIsMapped(archsize_t addr);


//
// -- Create a new set of paging tables for a new process
//    ---------------------------------------------------
EXTERN_C EXPORT KERNEL
frame_t MmuNewVirtualSpace(frame_t stack);


//
// -- Convert a virtual address to physical for the current paging tables
//    returns -1 if not mapped, which should be an invalid (unaligned) address for most archs
//    ---------------------------------------------------------------------------------------
EXTERN_C EXPORT KERNEL
archsize_t MmuVirtToPhys(void *addr);


//
// -- Check a structure to see if it is fully mapped
//    ----------------------------------------------
#define IS_MAPPED(a,z) ({                                                                                       \
        bool __rv = true;                                                                                       \
        for (archsize_t __va = ((archsize_t)a) & ~0x0fff; __va <= (((archsize_t)a) + z); __va += PAGE_SIZE) {   \
            __rv = __rv && MmuIsMapped(__va);                                                                   \
        }                                                                                                       \
        __rv; })


//
// -- The spinlock for clearing a page before giving it to the MMU
//    ------------------------------------------------------------
EXTERN EXPORT KERNEL_DATA
Spinlock_t frameClearLock;


