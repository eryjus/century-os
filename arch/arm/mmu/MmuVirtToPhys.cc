//===================================================================================================================
//
//  MmuVirtToPhys.cc -- Walk the page tables to convert a virtual address to a physical one
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Jan-12  Initial  v0.5.0e  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "printf.h"
#include "mmu.h"


//
// -- Check for the page and unmap if it is mapped.
//    ---------------------------------------------
EXTERN_C EXPORT KERNEL
archsize_t MmuVirtToPhys(void *addr)
{
    archsize_t a = (archsize_t)addr;

    Ttl1_t *ttl1Table = (Ttl1_t *)(ARMV7_TTL1_TABLE_VADDR);
    Ttl1_t *ttl1Entry = &ttl1Table[a >> 20];
    Ttl2_t *ttl2Tables = (Ttl2_t *)(ARMV7_TTL2_TABLE_VADDR);
    Ttl2_t *ttl2Entry = &ttl2Tables[a >> 12];

    if (ttl1Entry->fault == ARMV7_MMU_FAULT) return -1;
    if (ttl2Entry->fault == ARMV7_MMU_FAULT) return -1;

    // -- apply the proper offset to the physical frame!
    return (ttl2Entry->frame << 12) | (a & 0xfff);
}
