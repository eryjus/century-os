//===================================================================================================================
//
//  MmuUnmapPage.cc -- Unmap a page in virtual address space, returning the frame in case something else needs done.
//
//        Copyright (c)  2017-2020 -- Adam Clark
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
//  2019-Feb-09  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "cpu.h"
#include "printf.h"
#include "mmu.h"


//
// -- Check for the page and unmap if it is mapped.
//    ---------------------------------------------
frame_t MmuUnmapPage(archsize_t addr)
{
    frame_t rv = PT_ENTRY(addr)->frame;
    *(uint32_t *)PT_ENTRY(addr) = 0;
    InvalidatePage(addr);

    return rv;
}
