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
//static Ttl1_t *ttl1Table = (Ttl1_t *)TTL1_KRN_VADDR;
//static Ttl2_t *ttl2Table = (Ttl2_t *)TTL2_KRN_VADDR;


//
// -- Helper function to create and map a new table
//    ---------------------------------------------
static frame_t MmuMakeTtl2Table(ptrsize_t addr)
{
    ptrsize_t mmu = TTL1_KRN_VADDR;

    kprintf("Creating a new TTL2 table for address %p\n", addr);

    frame_t ttl2Frame = PmmAllocFrame();            // This cannot be called until the heap is initialized!!

    kprintf("  The new frame is %p\n", ttl2Frame);

    ptrsize_t ttl2Loc = ttl2Frame << 2;
    int i = (addr >> 20) & 0xffc;   // align this to the "% 4 == 0" TTL1 entry

    kprintf("  The base ttl2 1K location is %p\n", ttl2Loc);
    kprintf("  The ttl1 index is %x\n", i);

    // -- map the "% 4 == 0" TTL2 table
    Ttl1_t *ttl1Table = (Ttl1_t *)mmu;
    ttl1Table[i].ttl2 = ttl2Loc;
    ttl1Table[i].fault = 0b01;

    kprintf("  Set the TTL1 table index %x to 1K location %p\n", i, ttl2Loc);

    // -- map the "% 4 == 1" TTL2 table
    ttl1Table[i + 1].ttl2 = ttl2Loc + 1;
    ttl1Table[i + 1].fault = 0b01;

    kprintf("  Set the TTL1 table index %x to 1K location %p\n", i + 1, ttl2Loc + 1);

    // -- map the "% 4 == 2" TTL2 table
    ttl1Table[i + 2].ttl2 = ttl2Loc + 2;
    ttl1Table[i + 2].fault = 0b01;

    kprintf("  Set the TTL1 table index %x to 1K location %p\n", i + 2, ttl2Loc + 2);

    // -- map the "% 4 == 3" TTL2 table
    ttl1Table[i + 3].ttl2 = ttl2Loc + 3;
    ttl1Table[i + 3].fault = 0b01;

    kprintf("  Set the TTL1 table index %x to 1K location %p\n", i + 3, ttl2Loc + 3);

    // Here we need to get the TTL1 entry for the management address.
    int ttl2Offset = (addr >> 20) & 0xfff;
    ptrsize_t mgmtTtl2Addr = TTL2_KRN_VADDR + (ttl2Offset * 1024) + (((addr >> 12) & 0xff) * 4);
    int mgmtTtl1Index = mgmtTtl2Addr >> 20;
//    Ttl1_t *mgmtTtl1Entry = &ttl1Table[mgmtTtl1Index];

    kprintf("  The management address for this Ttl2 table is %p\n", mgmtTtl2Addr);
    kprintf("    The base location is 0xffc00000\n");
    kprintf("    The table offset is  %p\n", ttl2Offset * 1024);
    kprintf("    The entry offset is  %p\n", ((addr >> 12) & 0xff) * 4);
    kprintf("  The TTL1 management index for this address is %p\n", mgmtTtl1Index);

    // -- Map the management address anyway -- will throw errors!!! but can be ignored
    // If the TTL1 Entry for the management address is faulted; create a new TTL2 table
    kprintf("This next call to map address %p may throw an error\n", mgmtTtl2Addr);

    MmuMapToFrame(mgmtTtl2Addr, ttl2Frame, 0);

    kprintf("< Completed the table creation for %p\n", addr);

    return ttl2Frame;

#if 0
#endif
}


//
// -- Map a page to a frame
//    ---------------------
void MmuMapToFrame(ptrsize_t addr, frame_t frame, int flags)
{
    ptrsize_t mmu = TTL1_KRN_VADDR;

    kprintf("Mapping address %p to frame %p using physical table %p\n", addr, frame, mmu);

    // first, go find the ttl1 entry
    Ttl1_t *ttl1Table = (Ttl1_t *)mmu;
    int ttl1Index = ((addr >> 20) & 0x0fff);
    Ttl1_t *ttl1Entry = &ttl1Table[ttl1Index];

    kprintf(".. Ttl1 index is: %p [%x]\n", ttl1Table, ttl1Index);

    // now, do we have a ttl2 table for this address?
    if (ttl1Entry->fault == 0b00) MmuMakeTtl2Table(addr);

    // get the ttl2 entry
    Ttl2_t *ttl2Table = (Ttl2_t *)TTL2_KRN_VADDR;
    int ttl2Index = ((addr >> 12) & 0xfffff);
    Ttl2_t *ttl2Entry = &ttl2Table[ttl2Index];

    kprintf("  Ttl2 location is: %p [%x]\n", ttl2Table, ttl2Index);

    // if the page is already mapped, report an error
    if (ttl2Entry->fault != 0b00) {
        kprintf("Attempting to map already mapped address %p (mapped to: %p)\n", addr, ttl2Entry->frame);
        kprintf("   The TTL1 Address is: %p\n", ttl1Table);
        kprintf("   The Ttl1 Index is: %x\n", ttl1Index);
        kprintf("   The TTL2 Address is: %p\n", ttl2Table);
        kprintf("   The Ttl2 Index is: %x\n", ttl2Index);
        kprintf("   Frame is: %p\n", ttl2Entry->frame);
        kprintf("   Fault is: %x\n", ttl2Entry->fault);
        kprintf("   The address of the ttl2 entry is: %p\n", ttl2Entry);
        kprintf("   The overall contents are: %x\n", ttl2Entry);

        return;
    }

    ttl2Entry->frame = frame;
    ttl2Entry->s = 1;
    ttl2Entry->apx = 0;
    ttl2Entry->ap = 0b11;
    ttl2Entry->tex = 0b001;
    ttl2Entry->c = 1;
    ttl2Entry->b = 1;
    ttl2Entry->nG = 0;
    ttl2Entry->fault = 0b10;

#if 0
    kprintf("Attemping to map address %p to frame %p\n", addr, frame);
    // -- page-align the address
    addr &= 0xfffff000;

    // -- We will not map address 0x00000000
    if (addr == 0) {
        kprintf("Explicit request to map virtual address 0 refused.\n");
        return;
    }

    Ttl1_t *ttl1Entry = &ttl1Table[addr >> 20];
    kprintf(".. The TTL1 Address to manage is %p\n", ttl1Entry);

    // -- do we need a new TTL2 table?
    if (ttl1Entry->ttl2 == 0) {
        MmuMakeTtl2Table(addr);

        // -- here we need to determine the management address for this new table and map that as well
        int ttl2Offset = (addr >> 20) & 0xfff;
        ptrsize_t mgmtTtl2Addr = ((addr&0x80000000)?TTL2_KRN_VADDR:TTL2_KRN_VADDR) + (ttl2Offset * 1024) + (((addr >> 12) & 0xff) * 4);
        int mgmtTtl1Index = mgmtTtl2Addr >> 20;
        Ttl1_t *mgmtTtl1Entry = &ttl1Table[mgmtTtl1Index];

        if (mgmtTtl1Entry->fault == 0b00) {
            MmuMapToFrame(mgmtTtl2Addr, MmuMakeTtl2Table(mgmtTtl2Addr), flags);
        }
    }

    // -- get the proper TTL2 entry
    kprintf(".. Finally we are able to map the frame\n");
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

    kprintf("`MmuMapToFrame()` complete\n");
#endif
}

