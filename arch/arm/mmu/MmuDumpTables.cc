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
    kprintf("\nMmuDumpTables: Walking the page tables for address %p\n", addr);
    kprintf("Level  Tabl-Addr     Index        Entry Addr    Next PAddr    fault\n");
    kprintf("-----  ----------    ----------   ----------    ----------    -----\n");

    Ttl1_t *t1 = MMU_TTL1_ENTRY(addr);

    kprintf("TTL1   %p    %d         %p    %p     %x\n", mmuLvl1Table, addr >> 20, t1, t1->ttl2 << 10, t1->fault);

    if (!t1->fault) return;
    archsize_t t2tab = (t1->ttl2 << 10);
    int i = (addr >> 12) & 0xff;
    Ttl2_t *t2 = MMU_TTL2_ENTRY(addr);

    kprintf("TTL2   %p    %d         %p    %p     %x\n", t2tab, i, t2, t2->frame, t2->fault);
}