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
//  2018-Nov-19  Initial   0.2.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "serial.h"
#include "pmm.h"
#include "mmu-loader.h"


//
// -- Dump the MMU Tables for a specific address
//    ------------------------------------------
void MmuDumpTables(ptrsize_t addr)
{
    SerialPutS("\nMmuDumpTables: Walking the page tables for address ");
    SerialPutHex(addr);
    SerialPutS("\n");
    SerialPutS("Level  Tabl-Addr     Index        Entry Addr    Next PAddr    fault\n");
    SerialPutS("-----  ----------    ----------   ----------    ----------    -----\n");

    uint32_t i = (addr >> 20) & 0xfff;
    Ttl1_t *t1 = &((Ttl1_t *)GetMmuTopAddr())[i];

    SerialPutS("TTL1   ");
    SerialPutHex((uint32_t)GetMmuTopAddr());
    SerialPutS("    ");
    SerialPutHex(i);
    SerialPutS("   ");
    SerialPutHex((uint32_t)t1);
    SerialPutS("    ");
    SerialPutHex(t1->ttl2 << 10);
    SerialPutS("     ");
    switch (t1->fault) {
        case 0b00: SerialPutS("00\n"); break;
        case 0b01: SerialPutS("01\n"); break;
        case 0b10: SerialPutS("10\n"); break;
        case 0b11: SerialPutS("11\n"); break;
    }

    if (!t1->fault) return;
    Ttl2_t *t2tab = (Ttl2_t *)(t1->ttl2 << 10);
    i = (addr >> 12) & 0x0ff;
    Ttl2_t *t2 = &t2tab[i];

    SerialPutS("TTL2   ");
    SerialPutHex((uint32_t)t2tab);
    SerialPutS("    ");
    SerialPutHex(i);
    SerialPutS("   ");
    SerialPutHex((uint32_t)t2);
    SerialPutS("    ");
    SerialPutHex(t2->frame);
    SerialPutS("     ");
    switch (t2->fault) {
        case 0b00: SerialPutS("00\n"); break;
        case 0b01: SerialPutS("01\n"); break;
        case 0b10: SerialPutS("10\n"); break;
        case 0b11: SerialPutS("11\n"); break;
    }
}
