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
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "cpu.h"
#include "printf.h"
#include "pmm.h"
#include "spinlock.h"
#include "loader.h"
#include "mmu.h"


#ifndef DEBUG_MMU
#   define DEBUG_MMU 0
#endif


Spinlock_t frameClearLock = {0};

//
// -- Helper function to create and map a new table
//
//    TODO: This is currently only works for kernel space; it will need to be updated for user space
//    ----------------------------------------------------------------------------------------------
static frame_t MmuMakeTtl2Table(archsize_t addr)
{
    //
    // -- We have been asked to create a new TTL2 table.  We got here, so we know we need a frame.
    //    Go get it.
    //    ----------------------------------------------------------------------------------------
    frame_t frame = PmmNewFrame(1);
#if DEBUG_MMU == 1
    kprintf("The frame just allocated is at %p\n", frame);
#endif


    //
    // -- The next order of business is to map this frame to clear it.  We carefully chose this
    //    location to be in the same TTL2 table as the management addresses for the TTL1 table.  However.
    //    it is a critical section and needs to be synchronized.  Therefore, obtain a lock before
    //    attempting to use that address.
    //    -----------------------------------------------------------------------------------------------
    Ttl2_t *ttl2Entry = KRN_TTL2_ENTRY(MMU_CLEAR_FRAME);
#if DEBUG_MMU == 1
    kprintf("Attempting to update data for TTL2 entry at %p\n", ttl2Entry);
#endif
    SPIN_BLOCK(frameClearLock) {
#if DEBUG_MMU == 1
        kprintf("Lock obtained\n");
#endif
        InvalidatePage(MMU_CLEAR_FRAME);
        ttl2Entry->frame = frame;
        ttl2Entry->s = 1;
        ttl2Entry->apx = 0;
        ttl2Entry->ap = 0b11;
        ttl2Entry->tex = 0b001;
        ttl2Entry->c = 1;
        ttl2Entry->b = 1;
        ttl2Entry->nG = 0;
        ttl2Entry->fault = 0b10;
        InvalidatePage(MMU_CLEAR_FRAME);


#if DEBUG_MMU == 1
        kprintf("Entry mapped\n");

        kprintf("Pre-clear checks to see what is wrong:\n");
        kprintf(".. The TTL1 Entry is at address %p\n", KRN_TTL1_ENTRY(MMU_CLEAR_FRAME));
        kprintf(".. The entry does%s have a ttl2 table at frame %p\n",
                (KRN_TTL1_ENTRY(MMU_CLEAR_FRAME))->fault==0b00?" not":"",
                (KRN_TTL1_ENTRY(MMU_CLEAR_FRAME))->ttl2 >> 2);
        kprintf(".. The Management TTL2 entry is at %p\n", KRN_TTL2_MGMT(MMU_CLEAR_FRAME));
        kprintf(".. The entry does%s have a page table at frame %p\n",
                (KRN_TTL2_MGMT(MMU_CLEAR_FRAME))->fault==0b00?" not":"",
                (KRN_TTL2_MGMT(MMU_CLEAR_FRAME))->frame);
        kprintf(".. The TTL2 entry itself is at %p\n", KRN_TTL2_ENTRY(MMU_CLEAR_FRAME));
        kprintf(".. The entry does%s have a page table at frame %p\n",
                (KRN_TTL2_ENTRY(MMU_CLEAR_FRAME))->fault==0b00?" not":"",
                (KRN_TTL2_ENTRY(MMU_CLEAR_FRAME))->frame);
        kprintf("Performing a table walk for address %p\n", MMU_CLEAR_FRAME);
        kprintf(".. TTL1 table is at %p\n", mmuLvl1Table);
        kprintf(".. The index for this table is %x\n", MMU_CLEAR_FRAME >> 20);
        Ttl1_t *t1e = &((Ttl1_t *)mmuLvl1Table)[MMU_CLEAR_FRAME >> 20];
        kprintf(".. The TTL1 Entry is at address %p\n", t1e);
        Ttl2_t *t2t = (Ttl2_t *)(t1e->ttl2 << 10);
        kprintf(".. The TTL2 table is located at phys address %p\n", t2t);
        kprintf(".. The TTl2 index is %x\n", (MMU_CLEAR_FRAME >> 12) & 0xff);
        Ttl2_t *t2e = (Ttl2_t *)&t2t[(MMU_CLEAR_FRAME >> 12) & 0xff];
        kprintf(".. The TTL2 Entry is located at physical address %p\n", t2e);
        kprintf(".. The actual frame for this address is %p\n", t2e->frame << 12);
#endif


        kMemSetB((void *)MMU_CLEAR_FRAME, 0, FRAME_SIZE);
        MmuUnmapPage(MMU_CLEAR_FRAME);
        SpinlockUnlock(&frameClearLock);
    }


    //
    // -- The next order of business is to map this into the Management table.  This needs to be done for
    //    every new table, so there is nothing to check -- we know we need to do this.
    //    -----------------------------------------------------------------------------------------------
    InvalidatePage(addr & 0xfffff000);
    ttl2Entry = KRN_TTL2_MGMT(addr);
    ttl2Entry->frame = frame;
    ttl2Entry->s = 1;
    ttl2Entry->apx = 0;
    ttl2Entry->ap = 0b11;
    ttl2Entry->tex = 0b001;
    ttl2Entry->c = 1;
    ttl2Entry->b = 1;
    ttl2Entry->nG = 0;
    ttl2Entry->fault = 0b10;
    InvalidatePage(addr & 0xfffff000);


    //
    // -- Note that we will not actually map this into the TTL1 table.  The calling function holds that
    //    responsibility.  Therefore, the only thing left to do is to return the frame we have allocated
    //    and prepared to be a TTL2 table.
    //    ----------------------------------------------------------------------------------------------
#if DEBUG_MMU == 1
    kprintf("TTL2 table prepared\n");
#endif
    return frame;
}


//
// -- Map a page to a frame
//    ---------------------
void MmuMapToFrame(archsize_t addr, frame_t frame, int flags)
{
    // -- refuse to map frame 0 for security reasons
    if (!frame) {
        kprintf("Explicit request to map frame 0 refused.\n");
        return;
    }

    // -- refuse to map the NULL address for security reasons
    if (!addr) {
        kprintf("Explicit request to map virtual address 0 refused.\n");
        return;
    }


#if DEBUG_MMU == 1
    kprintf("Mapping address %p to frame %p\n", addr, frame);
#endif


    //
    // -- The first order of business is to check if we have a TTL2 table for this address.  We will know this
    //    by checking the TTL1 Entry and checking the fault field.
    //    ----------------------------------------------------------------------------------------------------
    if (KRN_TTL1_ENTRY(addr)->fault == 0b00) {
        frame_t ttl2 = MmuMakeTtl2Table(addr);
        Ttl1_t *ttl1Entry = KRN_TTL1_ENTRY4(addr);

        for (int i = 0; i < 4; i ++) {
            ttl1Entry[i].ttl2 = (ttl2 << 2) + i;
            ttl1Entry[i].fault = 0b01;
        }
    }


    //
    // -- At this point, we know we have a ttl2 table and the management entries are all set up properly.  It
    //    is just a matter of mapping the address.
    //    ---------------------------------------------------------------------------------------------------
    Ttl2_t *ttl2Entry = KRN_TTL2_ENTRY(addr);

    if (ttl2Entry->fault != 0b00) return;

    InvalidatePage(addr & 0xfffff000);
    ttl2Entry->frame = frame;
    ttl2Entry->s = 1;
    ttl2Entry->apx = 0;
    ttl2Entry->ap = 0b11;
    ttl2Entry->tex = 0b001;
    ttl2Entry->c = 1;
    ttl2Entry->b = 1;
    ttl2Entry->nG = 0;
    ttl2Entry->fault = 0b10;
    InvalidatePage(addr & 0xfffff000);
}

