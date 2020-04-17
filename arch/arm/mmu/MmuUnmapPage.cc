//===================================================================================================================
//
//  MmuUnmapPage.cc -- Unmap a page in virtual address space, returning the frame in case something else needs done.
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-21  Initial   0.2.0   ADCL  Initial version
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "cpu.h"
#include "printf.h"
#include "pic.h"
#include "process.h"
#include "mmu.h"


//
// -- Check for the page and unmap if it is mapped.
//    ---------------------------------------------
EXTERN_C EXPORT KERNEL
frame_t MmuUnmapPage(archsize_t addr)
{
    frame_t rv = 0;

    archsize_t flags = SPINLOCK_BLOCK_NO_INT(tlbFlush.lock) {
        tlbFlush.addr = -1;
        PicBroadcastIpi(picControl, IPI_TLB_FLUSH);

        Ttl1_t *ttl1Table = (Ttl1_t *)(ARMV7_TTL1_TABLE_VADDR);
        Ttl1_t *ttl1Entry = &ttl1Table[addr >> 20];
        Ttl2_t *ttl2Tables = (Ttl2_t *)(ARMV7_TTL2_TABLE_VADDR);
        Ttl2_t *ttl2Entry = &ttl2Tables[addr >> 12];

        if (ttl1Entry->fault == ARMV7_MMU_FAULT) goto exit;
        if (ttl2Entry->fault == ARMV7_MMU_FAULT) goto exit;

        rv = ttl2Entry->frame;
        *(uint32_t *)ttl2Entry = 0;

exit:
        WriteDCCMVAC((uint32_t)ttl2Entry);
        InvalidatePage(addr);


        //
        // -- Finally, wait for all the CPUs to complete the flush before continuing
        //   -----------------------------------------------------------------------
        AtomicSet(&tlbFlush.count, cpus.cpusRunning - 1);
        tlbFlush.addr = addr & ~(PAGE_SIZE - 1);

        while (AtomicRead(&tlbFlush.count) != 0 && picControl->ipiReady) {
            ProcessMilliSleep(150);
        }

        SPINLOCK_RLS_RESTORE_INT(tlbFlush.lock, flags);
    }

    return rv;
}
