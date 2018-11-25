//===================================================================================================================
//
//  MmuMapToFrame.cc -- Map a page to point to a physical frame
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  This function will walk the current paging tables and insert tables as needed to map a virtual address or
//  page to a physical frame.  If the page is already mapped, it will not be replaced.  Also, frame 0 is
//  explicitly not allowed to be mapped.  The loader takes care of this and there is no reason whatsoever why any
//  other task should need to map to this frame.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-10  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "cpu.h"
#include "printf.h"
#include "pmm.h"
#include "mmu-kernel.h"


//
// -- Map a page to a frame
//    ---------------------
void MmuMapToFrame(ptrsize_t addr, frame_t frame, int flags)
{
    kprintf("Map a page for the MMU for rpi2b here\n");
    Halt();
}

