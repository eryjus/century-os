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
//  2020-Apr-30  Initial  v0.7.0a  ADCL  Rewrite the MMU code
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
    kprintf("Preparing to unmap address %p\n", addr);

    LongDescriptor_t *lvl3;
    frame_t rv = 0;

    if (addr & 0x80000000) {
        kprintf(".. kernel-space\n");
        lvl3 = (LongDescriptor_t *)ARMV7_LONG_KERNEL_LVL3;
    } else {
        kprintf(".. user-space\n");
        lvl3 = (LongDescriptor_t *)ARMV7_LONG_USER_LVL3;
    }

    kprintf("Locking the TLB Flush IPI mechanism; its current state is %s at %p\n",
            tlbFlush.lock.lock?"locked":"unlocked", &tlbFlush.lock);

    archsize_t flags = SPINLOCK_BLOCK_NO_INT(tlbFlush.lock) {
        kprintf(".. lock obtained\n");
        tlbFlush.addr = -1;
        PicBroadcastIpi(picControl, IPI_TLB_FLUSH);

        kprintf("Completed IPI Broadcast\n");

        rv = lvl3[LEVEL3ENT(addr)].physAddress;
        *(uint64_t *)&lvl3[LEVEL3ENT(addr)] = 0;
        InvalidatePage(addr);

        kprintf("The page us unmapped\n");

        //
        // -- Finally, wait for all the CPUs to complete the flush before continuing
        //   -----------------------------------------------------------------------
        AtomicSet(&tlbFlush.count, cpus.cpusRunning - 1);
        tlbFlush.addr = addr & ~(PAGE_SIZE - 1);

        kprintf("TLB Flush IPI...\n");

        while (AtomicRead(&tlbFlush.count) != 0 && picControl->ipiReady) { DMB(); }

        kprintf("... done\n");

        SPINLOCK_RLS_RESTORE_INT(tlbFlush.lock, flags);
    }

    return rv;
}
