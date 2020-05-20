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
//  2020-Apr-30  Initial  v0.7.0a  ADCL  Rewrite the MMU code
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "printf.h"
#include "mmu.h"



//
// -- Convert the virtual address to a physical one
//    ---------------------------------------------
EXTERN_C EXPORT KERNEL
archsize_t MmuVirtToPhys(archsize_t addr)
{
    LongDescriptor_t *lvl2;
    LongDescriptor_t *lvl3;

    if (addr & 0x80000000) {
        lvl2 = (LongDescriptor_t *)ARMV7_LONG_KERNEL_LVL2;
        lvl3 = (LongDescriptor_t *)ARMV7_LONG_KERNEL_LVL3;
    } else {
        lvl2 = (LongDescriptor_t *)ARMV7_LONG_USER_LVL2;
        lvl3 = (LongDescriptor_t *)ARMV7_LONG_USER_LVL3;
    }

    LongDescriptor_t *entry = &lvl2[LEVEL2ENT(addr)];

    if (entry->present == 0) return -1;

    entry = &lvl3[LEVEL3ENT(addr)];

    if (entry->present == 0) return -1;

    return (entry->physAddress << 12) | (addr & (PAGE_SIZE - 1));
}
