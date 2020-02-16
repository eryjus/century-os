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
    archsize_t flags = SPINLOCK_BLOCK_NO_INT(frameClearLock) {
        PageEntry_t *pte = PT_ENTRY(MMU_CLEAR_FRAME);

        if (!pte->p) {
            pte->frame = frame;
            pte->rw = X86_MMU_WRITE;
            pte->us = X86_MMU_USER;
            pte->p = X86_MMU_PRESENT_TRUE;
            InvalidatePage(MMU_CLEAR_FRAME);
        }

        kMemSetB((void *)MMU_CLEAR_FRAME, 0, FRAME_SIZE);
        MmuUnmapPage(MMU_CLEAR_FRAME);
        SPINLOCK_RLS_RESTORE_INT(frameClearLock, flags);
    }
}

