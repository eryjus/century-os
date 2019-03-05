//===================================================================================================================
//
//  mmu.h -- This is the kernel MMU manager header.
//
//        Copyright (c)  2017-2019 -- Adam Clark
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


#ifndef __MMU_H__
#define __MMU_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "types.h"
#include "arch-mmu.h"


//
// -- Some constants to help with mapping flags
//    -----------------------------------------
enum {
    PG_KRN = 0x00000001,
    PG_WRT = 0x00000002,
};


#define MMU_CLEAR_FRAME 0xff400000
#define MMU_FRAMEBUFFER 0xfb000000
#define MMU_HEAP_START 0x90000000


//
// -- With the page table structures given, map a virtual address to a physical frame
//    -------------------------------------------------------------------------------
void MmuMapToFrame(archsize_t addr, frame_t frame, int flags);


//
// -- Unmap a page from the page table
//    --------------------------------
frame_t MmuUnmapPage(archsize_t addr);


//
// -- Complete the initialization of the MMU
//    --------------------------------------
void MmuInit(void);


//
// -- Invalidate a page in the TLB
//    ----------------------------
extern "C" void InvalidatePage(archsize_t addr);


#endif
