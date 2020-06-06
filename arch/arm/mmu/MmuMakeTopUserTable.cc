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
//  2020-May-20  Initial  v0.7.0a  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "pmm.h"
#include "mmu.h"


//
// -- Make a top-level MMU table for user space
//    -----------------------------------------
EXTERN_C EXPORT KERNEL
void MmuMakeTopUserTable(archsize_t frame)
{
    archsize_t flags = SPINLOCK_BLOCK_NO_INT(mmuCopyLock) {
        MmuMapToFrame(MMU_TOP_TABLE_TO, frame, PG_KRN | PG_WRT);
        frame_t lvl2a = PmmAllocateFrame();
        frame_t lvl2b = PmmAllocateFrame();
        MmuMapToFrame(MMU_TOP_TABLE_FROM, lvl2b, PG_KRN | PG_WRT);

        LongDescriptor_t *l1 = (LongDescriptor_t *)MMU_TOP_TABLE_TO;
        LongDescriptor_t *l2 = (LongDescriptor_t *)MMU_TOP_TABLE_FROM;

        kMemSetB(l1, 0, PAGE_SIZE);
        kMemSetB(l2, 0, PAGE_SIZE);

        l1[0].physAddress = lvl2a;
        l1[0].xn = 1;
        l1[0].pxn = 1;
        l1[0].software = 0;
        l1[0].contiguous = 0;
        l1[0].nG = 0;
        l1[0].af = 1;
        l1[0].sh = 0b11;
        l1[0].ap = 0b01;
        l1[0].ns = 1;
        l1[0].attrIndex = 0b000;
        l1[0].flag = 1;
        l1[0].present = 1;

        l1[1].physAddress = lvl2b;
        l1[1].xn = 1;
        l1[1].pxn = 1;
        l1[1].software = 0;
        l1[1].contiguous = 0;
        l1[1].nG = 0;
        l1[1].af = 1;
        l1[1].sh = 0b11;
        l1[1].ap = 0b01;
        l1[1].ns = 1;
        l1[1].attrIndex = 0b000;
        l1[1].flag = 1;
        l1[1].present = 1;

        l2[510].physAddress = lvl2a;
        l2[510].xn = 1;
        l2[510].pxn = 1;
        l2[510].software = 0;
        l2[510].contiguous = 0;
        l2[510].nG = 0;
        l2[510].af = 1;
        l2[510].sh = 0b11;
        l2[510].ap = 0b01;
        l2[510].ns = 1;
        l2[510].attrIndex = 0b000;
        l2[510].flag = 1;
        l2[510].present = 1;

        l2[511].physAddress = lvl2b;
        l2[511].xn = 1;
        l2[511].pxn = 1;
        l2[511].software = 0;
        l2[511].contiguous = 0;
        l2[511].nG = 0;
        l2[511].af = 1;
        l2[511].sh = 0b11;
        l2[511].ap = 0b01;
        l2[511].ns = 1;
        l2[511].attrIndex = 0b000;
        l2[511].flag = 1;
        l2[511].present = 1;

        MmuUnmapPage(MMU_TOP_TABLE_FROM);
        MmuMapToFrame(MMU_TOP_TABLE_FROM, lvl2a, PG_KRN | PG_WRT);
        kMemSetB(l2, 0, PAGE_SIZE);

        SPINLOCK_RLS_RESTORE_INT(mmuCopyLock, flags);
    }
}

