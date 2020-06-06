//===================================================================================================================
//
//  MmuDumpTables.cc -- Dump the Paging Tables for a virtual address
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-19  Initial   0.2.0   ADCL  Initial version
//  2019-Dec-31  Initial  v0.5.0c  ADCL  Recover from the old rpi2b loader and refactor for the kernel
//  2020-Apr-27  Initial  v0.7.0a  ADCL  Rewrite the MMU code
//
//===================================================================================================================


#include "types.h"
#include "entry.h"
#include "printf.h"
#include "mmu.h"


//
// -- Dump the MMU Tables for a specific address
//    ------------------------------------------
EXTERN_C EXPORT KERNEL
void MmuDumpTables(archsize_t addr)
{
    kprintf("\n");
    kprintf("MMU Tables Dump: Walking the page tables for address %p\n", addr);
    kprintf("\n");
    kprintf(".. TTBR0 entry is %p\n", MmuGetTopUserTable());
    kprintf(".. TTBR1 entry is %p\n", MmuGetTopKernelTable());
    kprintf("\n");
    kprintf("Level  Tabl-Addr     Index        Entry Addr    Next Frame    Attr Bits\n");
    kprintf("-----  ----------    ----------   ----------    ----------    ---------------------\n");

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
    uint64_t entBits = *(uint64_t *)entry;
    entBits &= ~(entry->physAddress << 12);

    kprintf("  2    %p    %p   %p    %p    %p %p\n", ((archsize_t)entry) & ~(PAGE_SIZE - 1), LEVEL2ENT(addr),
            entry, entry->physAddress, (uint32_t)(entBits >> 32), (uint32_t)entBits);

    entry = &lvl3[LEVEL3ENT(addr)];
    entBits = *(uint64_t *)entry;
    entBits &= ~(entry->physAddress << 12);

    kprintf("  3    %p    %p   %p    %p    %p %p\n", ((archsize_t)entry) & ~(PAGE_SIZE - 1),
            LEVEL3ENT(addr) & 0x1ff, entry, entry->physAddress, (uint32_t)(entBits >> 32), (uint32_t)entBits);

    kprintf("\n");
}
