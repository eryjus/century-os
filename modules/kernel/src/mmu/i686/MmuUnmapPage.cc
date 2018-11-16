//===================================================================================================================
//
//  MmuUnmapPage.cc -- Unmap a page in virtual address space, returning the frame in case something else needs done.
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  This function will walk the current paging tables and remove the page from the virtual address space if it is
//  mapped.  If not, frame 0 is returned as an invalid page mapping.  Note that frame 0 really does exist and
//  really is mapped into this space but will never be unmapped.  Since it also contains the GDT, IDT, and TSS, it
//  is also the least likely to be attempted to be unmapped and has a carefully located position in the virtual
//  address space.  The changes of the kernel doing this legitimately are nearly NULL.
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
#include "mmu.h"


//
// -- Check for the page and unmap if it is mapped.
//    ---------------------------------------------
frame_t MmuUnmapPage(ptrsize_t addr)
{
    pageEntry_t *pde = MmuGetPDEntry(addr);
    if (!pde->p) return 0;

    pageEntry_t *pte = MmuGetPTEntry(addr);
    if (!pte->p) return 0;

    frame_t rv = pte->frame;
    if (rv == 0) return 0;

    kMemSetB(pte, 0, sizeof(pageEntry_t));
    return rv;
}
