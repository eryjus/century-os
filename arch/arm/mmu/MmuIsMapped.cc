//===================================================================================================================
//
//  MmuIsMapped.cc -- Determine if the address is mapped in the current address space
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-May-01  Initial   0.4.3   ADCL  Initial version
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
bool MmuIsMapped(archsize_t addr)
{
    Ttl1_t *ttl1Table = (Ttl1_t *)(ARMV7_TTL1_TABLE_VADDR);
    Ttl1_t *ttl1Entry = &ttl1Table[addr >> 20];
    Ttl2_t *ttl2Tables = (Ttl2_t *)(ARMV7_TTL2_TABLE_VADDR);
    Ttl2_t *ttl2Entry = &ttl2Tables[addr >> 12];

    if (ttl1Entry->fault == 0b00) return false;
    if (ttl2Entry->fault == 0b00) return false;
    return true;
}
