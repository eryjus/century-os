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
//  Note that this function is called before any serial port mapping/setup is complete; therefore, not debugging
//  code can exist in this function yet.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-10  Initial   0.1.0   ADCL  Initial version
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "cpu.h"
#include "printf.h"
#include "pmm.h"
#include "process.h"
#include "spinlock.h"
#include "pic.h"
#include "mmu.h"


//
// -- Map a page to a frame
//    ---------------------
EXTERN_C EXPORT KERNEL
void MmuMapToFrame(archsize_t addr, frame_t frame, int pgFlags)
{
    // -- refuse to map addr/frame 0 for security reasons
    if (!frame || !addr) {
        return;
    }

    LongDescriptor_t *lvl2;
    LongDescriptor_t *lvl3;

    if (addr & 0x80000000) {
        lvl2 = (LongDescriptor_t *)ARMV7_LONG_KERNEL_LVL2;
        lvl3 = (LongDescriptor_t *)ARMV7_LONG_KERNEL_LVL3;
    } else {
        lvl2 = (LongDescriptor_t *)ARMV7_LONG_USER_LVL2;
        lvl3 = (LongDescriptor_t *)ARMV7_LONG_USER_LVL3;
    }

    LongDescriptor_t *entry = &lvl2[LEVEL2ENT(addr)];

    if (entry->present == 0) {
        entry->present = 1;
        entry->flag = 1;
        entry->attrIndex = 0b010;
        entry->ns = 1;
        entry->ap = 0b01;
        entry->sh = 0b11;
        entry->af = 1;
        entry->nG = 0;
        entry->physAddress = PmmAllocateFrame();
        entry->contiguous = 0;
        entry->pxn = 1;
        entry->xn = 1;
        entry->software = 0;
        entry->tblPxn = 0;
        entry->tblXn = 0;
        entry->tblAp = 0b00;
        entry->tblNs = 1;

        MmuClearFrame(entry->physAddress);

        WriteDCCMVAC((uint32_t)entry);
        InvalidatePage((archsize_t)entry);
        MemoryResynchronization();
    }

    entry = &lvl3[LEVEL3ENT(addr)];
    InvalidatePage((archsize_t)entry);
    MemoryResynchronization();

    if (!assert_msg(entry->present == 0, "Refusing to remap already mapped page")) {
        return;
    }

    entry->present = 1;
    entry->flag = 1;
    entry->attrIndex = (pgFlags & PG_DEVICE ? 0b010 : 0b000);
    entry->ns = 1;
    entry->ap = 0b01;
    entry->sh = 0b11;
    entry->af = 1;
    entry->nG = 0;
    entry->physAddress = frame;
    entry->contiguous = 0;
    entry->pxn = (pgFlags & PG_WRT ? 1 : 0);
    entry->xn = (pgFlags & PG_WRT ? 1 : 0);
    entry->software = 0;
    entry->tblPxn = 0;
    entry->tblXn = 0;
    entry->tblAp = 0b00;
    entry->tblNs = 1;

    WriteDCCMVAC((uint32_t)addr);
    InvalidatePage(addr);
    MemoryResynchronization();
}

