//===================================================================================================================
//
//  MmuDumpTables.cc -- Dump the Paging Tables for a virtual address
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2017-Jun-27  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "serial-loader.h"
#include "pmm.h"
#include "mmu-loader.h"


//
// -- Dump the MMU Tables for a specific address
//    ------------------------------------------
void MmuDumpTables(ptrsize_t addr)
{
    extern ptrsize_t cr3;

    SerialPutS("\nMmuDumpTables: Walking the page tables for address ");
    SerialPutHex(addr);
    SerialPutS("\n");
    SerialPutS("Level  Tabl-Addr     Index        Next Frame    us   rw   p\n");
    SerialPutS("-----  ----------    ----------   ----------    --   --   -\n");

    uint32_t i = (addr >> 22) & 0x3ff;
    pageEntry_t *w = &((pageEntry_t *)cr3)[i];

    SerialPutS("PD     ");
    SerialPutHex((uint32_t)cr3);
    SerialPutS("    ");
    SerialPutHex(i);
    SerialPutS("   ");
    SerialPutHex(w->frame);
    SerialPutS("     ");
    SerialPutS(w->us?"1":"0");
    SerialPutS("    ");
    SerialPutS(w->rw?"1":"0");
    SerialPutS("   ");
    SerialPutS(w->p?"1\n":"0\n");

    if (!w->p) return;
    pageEntry_t *wt = (pageEntry_t *)PmmFrameToLinear(w->frame);
    i = (addr >> 12) & 0x3ff;
    w = &wt[i];

    SerialPutS("PT     ");
    SerialPutHex((uint32_t)wt);
    SerialPutS("    ");
    SerialPutHex(i);
    SerialPutS("   ");
    SerialPutHex(w->frame);
    SerialPutS("     ");
    SerialPutS(w->us?"1":"0");
    SerialPutS("    ");
    SerialPutS(w->rw?"1":"0");
    SerialPutS("   ");
    SerialPutS(w->p?"1\n":"0\n");
}
