//===================================================================================================================
//
//  MmuNewVirtualSpace.cc -- For a new process, create the user virtual address space
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Mar-16  Initial   0.3.2   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "spinlock.h"
#include "pmm.h"
#include "mmu.h"


//
// -- for x86, we need to copy the kernel address space from PD[512] to PD[1023]
//    --------------------------------------------------------------------------
frame_t __krntext MmuNewVirtualSpace(frame_t stack)
{
    frame_t rv = PmmAllocateFrame();

    MmuClearFrame(rv);

    archsize_t flags = SPINLOCK_BLOCK_NO_INT(mmuTableInitLock) {
        MmuMapToFrame(MMU_NEW_TABLE_INIT, rv, PG_KRN | PG_WRT);
        PageEntry_t *tgtPD = (PageEntry_t *)MMU_NEW_TABLE_INIT;
        PageEntry_t *srcPD = (PageEntry_t *)PAGE_DIR_VADDR;

        for (int i = 512; i < 1024; i ++) tgtPD[i] = srcPD[i];

        MmuUnmapPage(MMU_NEW_TABLE_INIT);
        SPINLOCK_RLS_RESTORE_INT(mmuTableInitLock, flags);
    }

    return rv;
}
