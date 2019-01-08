//===================================================================================================================
//
//  MmuMapToFrame.cc -- Map a page to point to a physical frame
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  This function will walk the current paging tables and insert tables as needed to map a virtual address or
//  page to a physical frame.  If the page is already mapped, it will not be replaced.  Also, frame 0 is
//  explicitly not allowed to be mapped.  The loader takes care of this and there is no reason whatsoever why any
//  other task should need to map to this frame.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-10  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "cpu.h"
#include "printf.h"
#include "pmm.h"
#include "mmu-kernel.h"


//
// -- These are the the tables we use to manage the paging tables
//    -----------------------------------------------------------
static Ttl1_t *ttl1Table = (Ttl1_t *)TTL1_VADDR;
static Ttl2_t *ttl2Table = (Ttl2_t *)TTL2_VADDR;


//
// -- Helper function to create and map a new table
//    ---------------------------------------------
static void MmuMakeTtl2Table(ptrsize_t addr)
{
    frame_t f = PmmAllocFrame();

    int i = (addr >> 20) & 0x0ffc;
    uint32_t ttl2Loc = f << 2;

    ttl1Table[i].ttl2 = ttl2Loc;
    ttl1Table[i].fault = 0b01;

    ttl1Table[i + 1].ttl2 = ttl2Loc + 1;
    ttl1Table[i + 1].fault = 0b01;

    ttl1Table[i + 2].ttl2 = ttl2Loc + 2;
    ttl1Table[i + 2].fault = 0b01;

    ttl1Table[i + 3].ttl2 = ttl2Loc + 3;
    ttl1Table[i + 3].fault = 0b01;
}


//
// -- Map a page to a frame
//    ---------------------
void MmuMapToFrame(ptrsize_t addr, frame_t frame, int flags)
{
    // -- page-align the address
    addr &= 0xfffff000;

    // -- We will not map address 0x00000000
    if (addr == 0) {
        kprintf("Explicit request to map virtual address 0 refused.\n");
        return;
    }

    Ttl1_t *ttl1Entry = &ttl1Table[addr >> 20];

    // -- do we need a new TTL2 table?
    if (ttl1Entry->ttl2 == 0) {
        MmuMakeTtl2Table(addr);

        // -- here we need to determine the management address for this new table and map that as well
        int ttl2Offset = (addr >> 20) & 0xfff;
        ptrsize_t mgmtTtl2Addr = TTL2_VADDR + (ttl2Offset * 1024) + (((addr >> 12) & 0xff) * 4);
        int mgmtTtl1Index = mgmtTtl2Addr >> 20;
        Ttl1_t *mgmtTtl1Entry = &ttl1Table[mgmtTtl1Index];

        if (mgmtTtl1Entry->fault == 0b00) {
            MmuMakeTtl2Table(mgmtTtl2Addr);
        }
    }

    // -- get the proper TTL2 entry
    Ttl2_t *ttl2Entry = &ttl2Table[addr >> 12];

    if (ttl2Entry->fault != 0b00) {
        kprintf("Attempting to map already mapped address %p\n", addr);
        return;
    }

    ttl2Entry->frame = frame;
    ttl2Entry->s = 1;
    ttl2Entry->apx = 0;
    ttl2Entry->ap = 0b11;
    ttl2Entry->tex = 0b001;
    ttl2Entry->c = 0;
    ttl2Entry->b = 0;
    ttl2Entry->nG = 0;
    ttl2Entry->fault = 0b10;
}

