//===================================================================================================================
//
//  MmuClearFrame.cc -- Clear a frame before adding it into the paging structures formally
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  Note that this function does not need to trigger a TLB flush on other cores since this is not a shared mapping.
//  Only one CPU can get a lock to perform this function at a time, so by definition, no other cores require a TLB
//  flush -- the state when the lock is released is the same as it was when the lock was obtained: nothing is mapped.
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
    archsize_t flags = SPINLOCK_BLOCK_NO_INT(frameClearLock) {
        MmuMapToFrame(MMU_CLEAR_FRAME, frame, PG_KRN | PG_WRT);
        kMemSetB((void *)MMU_CLEAR_FRAME, 0, FRAME_SIZE);
        MmuUnmapPage(MMU_CLEAR_FRAME);
        SPINLOCK_RLS_RESTORE_INT(frameClearLock, flags);
    }
}

