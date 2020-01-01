//===================================================================================================================
//
//  MmuNewVirtualSpace.cc -- For a new process, create the user virtual address space
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Mar-16  Initial   0.3.2   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "spinlock.h"
#include "pmm.h"
#include "mmu.h"


//
// -- for arm, all we need is a blank address space
//    ---------------------------------------------
frame_t __krntext MmuNewVirtualSpace(frame_t stack)
{
    frame_t rv = PmmAllocAlignedFrames(4, 14);
    for (int i = 0; i < 4; i ++) MmuClearFrame(rv + i);
    return rv;
}
