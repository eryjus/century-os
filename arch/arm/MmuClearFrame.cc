//===================================================================================================================
//
//  MmuClearFrame.cc -- Clear a frame before adding it into the paging structures formally
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Mar-06  Initial   0.3.0   ADCL  Initial Version
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "spinlock.h"
#include "mmu.h"


//
// -- This spinlock is used to control access to the address space to clear the frame
//    -------------------------------------------------------------------------------
Spinlock_t frameClearLock = {0};


//
// -- Mount a frame into the kernel address space and clear its contents
//    ------------------------------------------------------------------
void MmuClearFrame(frame_t frame)
{
    //
    // -- The next order of business is to map this frame to clear it.  We carefully chose this
    //    location to be in the same TTL2 table as the management addresses for the TTL1 table.  However.
    //    it is a critical section and needs to be synchronized.  Therefore, obtain a lock before
    //    attempting to use that address.  This will always be done in kernel space.
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
}
