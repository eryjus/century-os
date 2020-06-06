//===================================================================================================================
//
// MmuMakeTopUserTable.cc -- Make a top-level table for user processes
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Apr-21  Initial  v0.7.0a  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "mmu.h"


//
// -- Make a top-level MMU table for user space
//    -----------------------------------------
EXTERN_C EXPORT KERNEL
void MmuMakeTopUserTable(archsize_t frame)
{
    archsize_t flags = SPINLOCK_BLOCK_NO_INT(mmuCopyLock) {
        MmuMapToFrame(MMU_TOP_TABLE_FROM, MmuGetTopTable(0) >> 12, PG_KRN | PG_WRT);
        MmuMapToFrame(MMU_TOP_TABLE_TO, frame, PG_KRN | PG_WRT);

        PageEntry_t *from = (PageEntry_t *)MMU_TOP_TABLE_FROM;
        PageEntry_t *to = (PageEntry_t *)MMU_TOP_TABLE_TO;

        for (int i = 512; i < 1023; i ++) to[i] = from[i];

        // -- clean up the recursive mapping
        to[1023].frame = frame;
        to[1023].rw = X86_MMU_WRITE;
        to[1023].us = X86_MMU_USER;
        to[1023].p = X86_MMU_PRESENT_TRUE;


        SPINLOCK_RLS_RESTORE_INT(mmuCopyLock, flags);
    }
}

