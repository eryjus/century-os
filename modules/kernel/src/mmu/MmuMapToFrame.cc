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
#include "mmu.h"


//
// -- Map a page to a frame
//    ---------------------
void MmuMapToFrame(ptrsize_t addr, frame_t frame, int flags)
{
    // -- page-align the address
    addr &= 0xfffff000;

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

    // -- Check the page directory entry to make sure there is a page table
    pageEntry_t *pde = MmuGetPDEntry(addr);
    if (!pde->p) {
        kprintf("We need to allocate a page table!\n");
        pde->frame = PmmAllocFrame();
        pde->k = (flags | PG_KRN);
        pde->rw = (flags | PG_WRT);
        pde->p = 1;
        pde->us = 1;
    }

    // -- Check the page table to make sure the page is not mapped
    pageEntry_t *pte = MmuGetPTEntry(addr);
    if (pte->p) {
        kprintf("Request to overwrite frame for page %p refused; use MmuUnmapPage() first\n", addr);
        return;
    }

    // -- finally we can map the page to the frame as requested
    pte->frame = frame;
    pte->k = (flags | PG_KRN);
    pte->rw = (flags | PG_WRT);
    pte->p = 1;
    pte->us = 1;
}

