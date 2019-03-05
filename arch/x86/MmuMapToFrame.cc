//===================================================================================================================
//
//  MmuMapToFrame.cc -- Map a page to point to a physical frame
//
//        Copyright (c)  2017-2019 -- Adam Clark
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
//  2019-Feb-09  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "cpu.h"
#include "printf.h"
#include "pmm.h"
#include "mmu.h"


//
// -- Map a page to a frame
//    ---------------------
void MmuMapToFrame(archsize_t addr, frame_t frame, int flags)
{
    // -- refuse to map frame 0 for security reasons
    if (!frame) {
        kprintf("Explicit request to map frame 0 refused.\n");
        return;
    }

    // -- refuse to map the NULL address for security reasons
    if (!addr) {
        kprintf("Explicit request to map virtual address 0 refused.\n");
        return;
    }

    PageEntry_t *pde = PD_ENTRY(addr);

    if (!pde->p) {
        pde->frame = PmmNewFrame(1);
        pde->rw = 1;
        pde->us = 1;
        pde->p = 1;
    }

    PageEntry_t *pte = PT_ENTRY(addr);

    if (pte->p) {
        kprintf("Request to overwrite frame for page %p refused; use MmuUnmapPage() first\n", addr);
        return;
    }

    // -- finally we can map the page to the frame as requested
    pte->frame = frame;
    pte->rw = 1;
    pte->us = 1;
    pte->p = 1;
}
