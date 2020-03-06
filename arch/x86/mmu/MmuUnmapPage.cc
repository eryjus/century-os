//===================================================================================================================
//
//  MmuUnmapPage.cc -- Unmap a page in virtual address space, returning the frame in case something else needs done.
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  This function will walk the current paging tables and remove the page from the virtual address space if it is
//  mapped.  If not, frame 0 is returned as an invalid page mapping.  Note that frame 0 really does exist and
//  really is mapped into this space but will never be unmapped.  Since it also contains the GDT, IDT, and TSS, it
//  is also the least likely to be attempted to be unmapped and has a carefully located position in the virtual
//  address space.  The changes of the kernel doing this legitimately are nearly NULL.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-10  Initial   0.1.0   ADCL  Initial version
//  2019-Feb-09  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "cpu.h"
#include "pic.h"
#include "printf.h"
#include "mmu.h"


//
// -- Check for the page and unmap if it is mapped.
//    ---------------------------------------------
EXTERN_C EXPORT KERNEL
frame_t MmuUnmapPage(archsize_t addr)
{
    frame_t rv;

    archsize_t flags = SPINLOCK_BLOCK_NO_INT(tlbFlush.lock) {
        tlbFlush.addr = -1;
        PicBroadcastIpi(picControl, IPI_TLB_FLUSH);

        rv = PT_ENTRY(addr)->frame;
        *(uint32_t *)PT_ENTRY(addr) = 0;
        InvalidatePage(addr);

        //
        // -- Finally, wait for all the CPUs to complete the flush before continuing
        //   -----------------------------------------------------------------------
        AtomicSet(&tlbFlush.count, cpus.cpusRunning - 1);
        tlbFlush.addr = addr & ~(PAGE_SIZE - 1);

        while (AtomicRead(&tlbFlush.count) != 0 && picControl->ipiReady) {}

        SPINLOCK_RLS_RESTORE_INT(tlbFlush.lock, flags);
    }

    return rv;
}
