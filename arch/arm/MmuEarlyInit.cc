//===================================================================================================================
//
//  MmuEarlyInit.cc -- Handle the early MMU initialization for the arm architecture
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  The goal of this function is to make sure that MMU is ready to run all the kernel functions as well as the
//  loader ones.  We have already mapped the lower 4MB of memory and we should be able to use plenty of PMM frames
//  to get the initialization complete.
//
//  One thing I will want to watch for is that I am encroaching on the kernel.  It is a possibility and so I want
//  to build this check into the function so that I can panic the kernel if I encroach on the kernel data.  This
//  will be done by comapring to `_kernelEnd`.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Feb-13  Initial   0.3.0   ADCL  Initial version
//
//===================================================================================================================


#include "loader.h"
#include "types.h"
#include "cpu.h"
#include "serial.h"
#include "mmu.h"
#include "printf.h"

#ifndef DEBUG_MMU
#   define DEBUG_MMU 0
#endif

//
// -- Complete the initialization of the Mmu for the loader to function properly
//    --------------------------------------------------------------------------
__CENTURY_FUNC__ void __ldrtext MmuEarlyInit(void)
{
    extern uint8_t _kernelStart[];
    extern uint8_t _kernelEnd[];
    archsize_t kernelStart = (archsize_t)_kernelStart;
    archsize_t kernelEnd = (archsize_t)_kernelEnd;
    archsize_t sectionCurr = kernelStart >> 20;
    archsize_t sectionEnd = (kernelEnd >> 20) + (kernelEnd&0xfffff?1:0);
    Ttl1_t *ttl1 = (Ttl1_t *)mmuLvl1Table;

#if DEBUG_MMU == 1
    LoaderSerialPutS("For TTL1 Table at ");
    LoaderSerialPutHex((uint32_t)ttl1);
    LoaderSerialPutChar('\n');
#endif

    //
    // -- First we need to complete the initialization of the TTL1 table.  Recall that only the first 4MB were
    //    mapped and the MMIO addresses were mapped to get serial output.  Therefore, I need to loop through
    //    the ttl1 table and make sure I complete the initialization to `0x00000000`.
    //    ----------------------------------------------------------------------------------------------------
    uint32_t *wrk = (uint32_t *)ttl1;
    for (int i = 0; i < 0x1000; i ++) {
        if (i < 4) continue;
        if (i >= 0x3f0 && i < 0x400) continue;
        wrk[i] = 0;
    }


    //
    // -- we start by mapping the ttl2 table the kernel in upper memory.  These mappings will start at
    //    0x80000000 and continue up until we have enough ttl2 tables to cover the kernel.  Hopefully that will be
    //    1 table, but there may be more.
    //    --------------------------------------------------------------------------------------------------------
    while (sectionCurr <= sectionEnd) {
        Ttl1_t *ttl1Entry = &ttl1[sectionCurr];
        if (ttl1Entry->fault == 0b00) {
            frame_t newFrame = NextEarlyFrame();

            if (newFrame < PHYS_OF(_kernelEnd) >> 12) {
                LoaderSerialPutS("Out of memory in MmuEarlyInit(); add another 4MB section\n");
                LoaderSerialPutS(".. Frame was "); LoaderSerialPutHex(newFrame); LoaderSerialPutChar('\n');
                LoaderSerialPutS(".. Limit is "); LoaderSerialPutHex(PHYS_OF(_kernelEnd) >> 12);
                        LoaderSerialPutChar('\n');
                Halt();
            }

#if DEBUG_MMU == 1
            LoaderSerialPutS("The new frame number is ");
            LoaderSerialPutHex(newFrame);
            LoaderSerialPutChar('\n');
#endif

            Ttl2_t *ttl2 = (Ttl2_t *)(newFrame << 12);
            lMemSetB(ttl2, 0, FRAME_SIZE);

            for (int i = 0; i < 4; i ++) {
#if DEBUG_MMU == 1
                LoaderSerialPutS("Installing new ttl2 table: ");
                LoaderSerialPutHex((newFrame << 2) + i);
                LoaderSerialPutS(" for section ");
                LoaderSerialPutHex(sectionCurr);
                LoaderSerialPutChar('\n');
#endif

                ttl1Entry[i].fault = 0b01;
                ttl1Entry[i].ttl2 = (newFrame << 2) + i;

                BPIALLIS();
            }
        }

        sectionCurr += 4;
    }

    //
    // -- Next, we make a second pass through and map the kernel pages into our newly created ttl2 table(s).
    //    --------------------------------------------------------------------------------------------------
    archsize_t pageCurr = kernelStart >> 12;
    archsize_t pageEnd = (kernelEnd >> 12) + (kernelEnd&0xfff?1:0);

    while (pageCurr <= pageEnd) {
        Ttl1_t *ttl1Entry = &ttl1[pageCurr >> 8];
        Ttl2_t *ttl2Entry = &((Ttl2_t *)(ttl1Entry->ttl2 << 10))[pageCurr & 0xff];

#if DEBUG_MMU == 1
        LoaderSerialPutS("For TTL1 Table at ");
        LoaderSerialPutHex((uint32_t)ttl1);
        LoaderSerialPutS(" and TTL2 Table at ");
        LoaderSerialPutHex(ttl1Entry->ttl2 << 10);
        LoaderSerialPutS(": Mapping page ");
        LoaderSerialPutHex(pageCurr<<12);
        LoaderSerialPutS("; ttl1 index: ");
        LoaderSerialPutHex(pageCurr>>8);
        LoaderSerialPutS("; ttl2 index: ");
        LoaderSerialPutHex(pageCurr & 0xff);
        LoaderSerialPutChar('\n');
#endif

        if (ttl2Entry->fault != 0b00) {
            LoaderSerialPutS("This page is already mapped!!!  ");
            LoaderSerialPutHex((uint32_t)*((uint32_t *)ttl2Entry));

            HaltCpu();
        }

        // TODO: make sure that these are set up properly for the kernel
        // -- Use the PHYS_OF() macro to convert the page to an physical address; and then a frame
        ttl2Entry->frame = PHYS_OF(pageCurr<<12)>>12;
        ttl2Entry->s = 1;
        ttl2Entry->apx = 0;
        ttl2Entry->ap = 0b11;
        ttl2Entry->tex = 0b001;
        ttl2Entry->c = 1;
        ttl2Entry->b = 1;
        ttl2Entry->nG = 0;
        ttl2Entry->fault = 0b10;

        BPIALLIS();

        pageCurr ++;
    }

    //
    // -- From here on, we have access to the kernel functions without restriction; the laoder code will still be
    //    at ~1MB while many of the support functions will be at ~`0x80000000`.  Data may be at either location
    //    depending on the variable and the loader data will be at ~1MB.  The following `kprintf()` call illustrates
    //    this fact as the function is in kernel address space and the constant .rodata string is also in
    //    kernel address space, but we are still running in loader code at ~1MB.
    //    ----------------------------------------------------------------------------------------------------------
#if DEBUG_MMU == 1
    LoaderSerialPutS("kprintf() should work:\n");
    kprintf("At this point, the kernel is fully mapped!!!\n");
#endif
}


