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
        INVALIDATE_PAGE(ttl2Entry, MMU_CLEAR_FRAME);

        ttl2Entry->frame = frame;
        ttl2Entry->s = 1;
        ttl2Entry->apx = 0;
        ttl2Entry->ap = 0b11;
        ttl2Entry->tex = 0b001;
        ttl2Entry->c = 1;
        ttl2Entry->b = 1;
        ttl2Entry->nG = 0;
        ttl2Entry->fault = 0b10;

        INVALIDATE_PAGE(ttl2Entry, MMU_CLEAR_FRAME);

        kMemSetB((void *)MMU_CLEAR_FRAME, 0, FRAME_SIZE);
        MmuUnmapPage(MMU_CLEAR_FRAME);
        SPINLOCK_RLS_RESTORE_INT(frameClearLock, flags);
    }
}

