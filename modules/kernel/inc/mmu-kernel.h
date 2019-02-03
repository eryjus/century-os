//===================================================================================================================
//
//  mmu-kernel.h -- This is the kernel MMU manager header.
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  All fo these functions are written so that PD[1023] is recursively mapped.  As a result, we know some things
//  are in certain locations.  For example, the Page Directory itself starts at virtual address 0xfffff000, no
//  matter what process we are in.  The Page Tables will be located at 0xffc00000 each.  This then means that in
//  order to manage any given Page Table structure, the calculations are consistent -- which differes from the
//  loader MMU requirements.  In particular, I do not need to know which cr3 I am managing.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  --------------------------------------------------------------------------
//  2018-Nov-10  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#ifndef __MMU_H__
#define __MMU_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "types.h"
#include "arch-mmu-prevalent.h"
#include "arch-mmu-kernel.h"


//
// -- Some constants to help with mapping flags
//    -----------------------------------------
enum {
    PG_KRN = 0x00000001,
    PG_WRT = 0x00000002,
};


//
// -- With the page table structures given, map a virtual address to a physical frame
//    -------------------------------------------------------------------------------
void MmuMapToFrame(ptrsize_t addr, frame_t frame, int flags);


//
// -- Unmap a page from the page table
//    --------------------------------
frame_t MmuUnmapPage(ptrsize_t addr);


//
// -- Some specific memory locations
//    ------------------------------
#define PROCESS_PAGE_DIR    0xff430000
#define PROCESS_PAGE_TABLE  0xff434000

// -- these are dedicated to the function `MmuGetFrameForAddr()`, but documented here.
#define MMU_FRAME_ADDR_PD   0xff436000
#define MMU_FRAME_ADDR_PT   0xff43a000

#define PROCESS_STACK_BUILD 0xff441000


#endif
