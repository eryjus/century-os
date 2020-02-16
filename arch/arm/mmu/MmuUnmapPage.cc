//===================================================================================================================
//
//  MmuUnmapPage.cc -- Unmap a page in virtual address space, returning the frame in case something else needs done.
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-21  Initial   0.2.0   ADCL  Initial version
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "cpu.h"
#include "printf.h"
#include "mmu.h"


//
// -- Check for the page and unmap if it is mapped.
//    ---------------------------------------------
EXTERN_C EXPORT KERNEL
frame_t MmuUnmapPage(archsize_t addr)
{
    Ttl1_t *ttl1Table = (Ttl1_t *)(ARMV7_TTL1_TABLE_VADDR);
    Ttl1_t *ttl1Entry = &ttl1Table[addr >> 20];
    Ttl2_t *ttl2Tables = (Ttl2_t *)(ARMV7_TTL2_TABLE_VADDR);
    Ttl2_t *ttl2Entry = &ttl2Tables[addr >> 12];

    if (ttl1Entry->fault == ARMV7_MMU_FAULT) return 0;
    if (ttl2Entry->fault == ARMV7_MMU_FAULT) return 0;

    frame_t rv = ttl2Entry->frame;
    *(uint32_t *)ttl2Entry = 0;

    InvalidatePage((uint32_t)ttl2Entry, addr);

    return rv;
}