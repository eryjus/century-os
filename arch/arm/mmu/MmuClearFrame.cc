//===================================================================================================================
//
//  MmuClearFrame.cc -- Clear a frame before adding it into the paging structures formally
//
//        Copyright (c)  2017-2020 -- Adam Clark
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
// -- Mount a frame into the kernel address space and clear its contents
//    ------------------------------------------------------------------
EXTERN_C EXPORT KERNEL
void MmuClearFrame(frame_t frame)
{
    //
    // -- The next order of business is to map this frame to clear it.  We carefully chose this
    //    location to be in the same TTL2 table as the management addresses for the TTL1 table.  However.
    //    it is a critical section and needs to be synchronized.  Therefore, obtain a lock before
    //    attempting to use that address.  This will always be done in kernel space.
    //    -----------------------------------------------------------------------------------------------
    Ttl2_t *ttl2Entry = KRN_TTL2_ENTRY(MMU_CLEAR_FRAME);
    archsize_t flags = SPINLOCK_BLOCK_NO_INT(frameClearLock) {
        InvalidatePage((uint32_t)ttl2Entry, MMU_CLEAR_FRAME);

        ttl2Entry->frame = frame;
        ttl2Entry->s = ARMV7_SHARABLE_TRUE;
        ttl2Entry->apx = ARMV7_MMU_APX_FULL_ACCESS;
        ttl2Entry->ap = ARMV7_MMU_AP_FULL_ACCESS;
        ttl2Entry->tex = ARMV7_MMU_TEX_NORMAL;
        ttl2Entry->c = ARMV7_MMU_CACHED;
        ttl2Entry->b = ARMV7_MMU_BUFFERED;
        ttl2Entry->nG = ARMV7_MMU_GLOBAL;
        ttl2Entry->fault = ARMV7_MMU_DATA_PAGE;

        InvalidatePage((uint32_t)ttl2Entry, MMU_CLEAR_FRAME);

        kMemSetB((void *)MMU_CLEAR_FRAME, 0, FRAME_SIZE);
        MmuUnmapPage(MMU_CLEAR_FRAME);
        SPINLOCK_RLS_RESTORE_INT(frameClearLock, flags);
    }
}

