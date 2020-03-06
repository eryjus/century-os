//===================================================================================================================
//
//  MmuMapToFrame.cc -- Map a page to point to a physical frame
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  This function will walk the current paging tables and insert tables as needed to map a virtual address or
//  page to a physical frame.  If the page is already mapped, it will not be replaced.  Also, frame 0 is
//  explicitly not allowed to be mapped.  The loader takes care of this and there is no reason whatsoever why any
//  other task should need to map to this frame.
//
//  Note that this function is called before any serial port mapping/setup is complete; therefore, not debugging
//  code can exist in this function yet.
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
#include "pic.h"
#include "mmu.h"


//
// -- Helper function to create and map a new table
//    ----------------------------------------------------------------------------------------------
EXTERN_C HIDDEN KERNEL
frame_t MmuMakeTtl2Table(archsize_t addr, int flags)
{
    //
    // -- We have been asked to create a new TTL2 table.  We got here, so we know we need a frame.
    //    Go get it.
    //    ----------------------------------------------------------------------------------------
    frame_t frame = PmmAllocateFrame();
    MmuClearFrame(frame);


    //
    // -- The next order of business is to map this into the Management table.  This needs to be done for
    //    every new table, so there is nothing to check -- we know we need to do this.
    //    -----------------------------------------------------------------------------------------------
    Ttl2_t *ttl2Entry = KRN_TTL2_ENTRY(MMU_CLEAR_FRAME);

    WriteDCCMVAC((uint32_t)ttl2Entry);
    InvalidatePage(addr);

    ttl2Entry = KRN_TTL2_MGMT(addr);
    ttl2Entry->frame = frame;
    ttl2Entry->s = ARMV7_SHARABLE_TRUE;
    ttl2Entry->apx = ARMV7_MMU_APX_FULL_ACCESS;
    ttl2Entry->ap = ARMV7_MMU_AP_FULL_ACCESS;
    ttl2Entry->tex = ARMV7_MMU_TEX_NORMAL;
    ttl2Entry->c = ARMV7_MMU_CACHED;
    ttl2Entry->b = ARMV7_MMU_BUFFERED;
    ttl2Entry->nG = ARMV7_MMU_GLOBAL;
    ttl2Entry->fault = ARMV7_MMU_DATA_PAGE;

    WriteDCCMVAC((uint32_t)ttl2Entry);
    InvalidatePage(addr);


    //
    // -- Note that we will not actually map this into the TTL1 table.  The calling function holds that
    //    responsibility.  Therefore, the only thing left to do is to return the frame we have allocated
    //    and prepared to be a TTL2 table.
    //    ----------------------------------------------------------------------------------------------
    WriteBPIALLIS();
    MemoryBarrier();

    return frame;
}



//==================================================================================================================


//
// -- Map a page to a frame
//    ---------------------
EXTERN_C EXPORT KERNEL
void MmuMapToFrame(archsize_t addr, frame_t frame, int flags)
{
    // -- refuse to map frame 0 for security reasons
    if (!frame || !addr) {
        return;
    }


    //
    // -- The first order of business is to check if we have a TTL2 table for this address.  We will know this
    //    by checking the TTL1 Entry and checking the fault field.
    //    ----------------------------------------------------------------------------------------------------
//    kprintf("Checking for TTL1 entry (%p; %x)....\n", addr, TTL1_ENTRY(addr, flags)->fault);
    if (KRN_TTL1_ENTRY(addr)->fault == ARMV7_MMU_FAULT) {
//        kprintf("TTL1 entry is not mapped to a TTL2 table; creating\n");
        frame_t ttl2 = MmuMakeTtl2Table(addr, flags);
        Ttl1_t *ttl1Entry = KRN_TTL1_ENTRY4(addr);

        for (int i = 0; i < 4; i ++) {
            ttl1Entry[i].ttl2 = (ttl2 << 2) + i;
            ttl1Entry[i].fault = ARMV7_MMU_TTL2;

            WriteDCCMVAC((uint32_t)&ttl1Entry[i]);
            InvalidatePage((uint32_t)&ttl1Entry[i]);
            MemoryBarrier();
        }
    }



    //
    // -- first, obtain the lock for managing the MMU structures
    //    ------------------------------------------------------
    archsize_t flg = SPINLOCK_BLOCK_NO_INT(tlbFlush.lock) {
        tlbFlush.addr = -1;
        PicBroadcastIpi(picControl, IPI_TLB_FLUSH);


        //
        // -- At this point, we know we have a ttl2 table and the management entries are all set up properly.  It
        //    is just a matter of mapping the address.
        //    ---------------------------------------------------------------------------------------------------
    //    kprintf("Checking for TTL2 entry....\n");
        Ttl2_t *ttl2Entry = KRN_TTL2_ENTRY(addr);
    //    kprintf("ttl2Entry has been set: %p\n", ttl2Entry);

        if (ttl2Entry->fault != ARMV7_MMU_FAULT) return;

    //    kprintf("mapping the page\n");

        WriteDCCMVAC((uint32_t)ttl2Entry);
        InvalidatePage(addr);

        ttl2Entry->frame = frame;
        ttl2Entry->s = ARMV7_SHARABLE_TRUE;
        ttl2Entry->apx = ARMV7_MMU_APX_FULL_ACCESS;
        ttl2Entry->ap = ARMV7_MMU_AP_FULL_ACCESS;
        ttl2Entry->tex = (flags&PG_DEVICE?ARMV7_MMU_TEX_DEVICE:ARMV7_MMU_TEX_NORMAL);
        ttl2Entry->c = (flags&PG_DEVICE?ARMV7_MMU_UNCACHED:ARMV7_MMU_CACHED);
        ttl2Entry->b = (flags&PG_DEVICE?ARMV7_MMU_UNBUFFERED:ARMV7_MMU_BUFFERED);
        ttl2Entry->nG = ARMV7_MMU_GLOBAL;
        ttl2Entry->fault = ARMV7_MMU_CODE_PAGE;

        WriteDCCMVAC((uint32_t)ttl2Entry);
        InvalidatePage(addr);
        MemoryBarrier();


        //
        // -- Finally, wait for all the CPUs to complete the flush before continuing
        //   -----------------------------------------------------------------------
        AtomicSet(&tlbFlush.count, cpus.cpusRunning - 1);
        tlbFlush.addr = addr & ~(PAGE_SIZE - 1);

        while (AtomicRead(&tlbFlush.count) != 0 && picControl->ipiReady) {}

        SPINLOCK_RLS_RESTORE_INT(tlbFlush.lock, flg);
    }
}

