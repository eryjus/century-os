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
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-10  Initial   0.1.0   ADCL  Initial version
//  2019-Feb-09  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "cpu.h"
#include "printf.h"
#include "pmm.h"
#include "pic.h"
#include "mmu.h"
#include "process.h"


//
// -- Map a page to a frame
//    ---------------------
EXTERN_C EXPORT KERNEL
void MmuMapToFrame(archsize_t addr, frame_t frame, int flags)
{
#if DEBUG_ENABLED(MmuMapToFrame)
    kprintf("Mapping page %p to frame %x\n", addr, frame);
    kprintf("... Kernel: %s\n", flags&PG_KRN?"yes":"no");
    kprintf("... Device: %s\n", flags&PG_DEVICE?"yes":"no");
    kprintf("... Write.: %s\n", flags&PG_WRT?"yes":"no");
#endif

    // -- refuse to map frame 0 for security reasons
    if (!frame || !addr) {
        return;
    }

#if DEBUG_ENABLED(MmuMapToFrame)
    kprintf(".. %s sanity checks passed\n", __func__);
#endif

    PageEntry_t *pde = PD_ENTRY(addr);

    if (!pde->p) {
        frame_t fr = PmmAllocateFrame();
        MmuClearFrame(fr);
        pde->frame = fr;
        pde->rw = X86_MMU_WRITE;
        pde->us = X86_MMU_USER;
        pde->p = X86_MMU_PRESENT_TRUE;
    }

    PageEntry_t *pte = PT_ENTRY(addr);

    if (pte->p) {
        return;
    }

    archsize_t flg = SPINLOCK_BLOCK_NO_INT(tlbFlush.lock) {
#if DEBUG_ENABLED(MmuMapToFrame)
    kprintf("About to flush TLB via IPI....\n");
#endif

        tlbFlush.addr = -1;
        PicBroadcastIpi(picControl, IPI_TLB_FLUSH);

#if DEBUG_ENABLED(MmuMapToFrame)
    kprintf(".. Completed TLB flush\n");
#endif


        // -- finally we can map the page to the frame as requested
        pte->frame = frame;
        pte->rw = (flags & PG_WRT?X86_MMU_WRITE:X86_MMU_READ);
        pte->us = (flags & PG_KRN?X86_MMU_SUPERVISOR:X86_MMU_USER);
        pte->pcd = (flags & PG_DEVICE?X86_MMU_PCD_TRUE:X86_MMU_PCD_FALSE);
        pte->pwt = (flags & PG_DEVICE?X86_MMU_PWT_ENABLED:X86_MMU_PWT_DISABLED);
        pte->p = X86_MMU_PRESENT_TRUE;


        //
        // -- Finally, wait for all the CPUs to complete the flush before continuing
        //   -----------------------------------------------------------------------
        int expected = cpus.cpusRunning - 1;
        AtomicSet(&tlbFlush.count, expected);
        tlbFlush.addr = addr & ~(PAGE_SIZE - 1);

        while (AtomicRead(&tlbFlush.count) != 0 && picControl->ipiReady) {
#if DEBUG_ENABLED(MmuMapToFrame)
            kprintf("CPU %d: Current response count is %d of %d at %p\n", thisCpu->cpuNum,
                    AtomicRead(&tlbFlush.count), expected, picControl);
#endif
            ProcessMilliSleep(1);
        }

        SPINLOCK_RLS_RESTORE_INT(tlbFlush.lock, flg);
    }
#if DEBUG_ENABLED(MmuMapToFrame)
    kprintf("... The contents of the PTE is at %p: %p\n", pte, ((*(uint32_t *)pte) & 0xffffffff));
#endif
}

