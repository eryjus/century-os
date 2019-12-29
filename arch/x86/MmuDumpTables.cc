//===================================================================================================================
//
//  MmuDumpTables.cc -- Dump the Paging Tables for a virtual address
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2017-Jun-27  Initial   0.1.0   ADCL  Initial version
//  2019-Apr-28            0.4.1   ADCL  Resurrected from an old commit (and updated) for debugging
//
//===================================================================================================================


#include "types.h"
#include "printf.h"
#include "cpu.h"
#include "pmm.h"
#include "mmu.h"


//
// -- Dump the MMU Tables for a specific address
//    ------------------------------------------
EXTERN_C EXPORT KERNEL
void MmuDumpTables(archsize_t addr)
{
    archsize_t cr3 = RECURSIVE_PD_VADDR;

    kprintf("\nMmuDumpTables: Walking the page tables for address %p\n", addr);
    kprintf("Level  Tabl-Addr   Index       Next Frame  us  rw  pr\n");
    kprintf("-----  ----------  ----------  ----------  --  --  --\n");

    uint32_t i = (addr >> 22) & 0x3ff;
    PageEntry_t *w = &((PageEntry_t *)cr3)[i];

    kprintf("PD     %p  %p  %p   %s   %s   %s\n", cr3, i, w->frame, w->us?"1":"0", w->rw?"1":"0", w->p?"1":"0");

    if (!w->p) return;
    PageEntry_t *wt = (PageEntry_t *)(RECURSIVE_VADDR + 0x1000 * i);
    i = (addr >> 12) & 0x3ff;
    w = &wt[i];

    kprintf("PT     %p  %p  %p   %s   %s   %s\n", wt, i, w->frame, w->us?"1":"0", w->rw?"1":"0", w->p?"1":"0");
}