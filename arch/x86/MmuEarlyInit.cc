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

#define DEBUG_MMU 1
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
    PageEntry_t *pageDirectory = (PageEntry_t *)mmuLvl1Table;


    //
    // -- First we need to complete the initialization of the Level 1 table.  Recall that only the first 4MB were
    //    mapped.  Therefore, I need to loop through the page directory and make sure I complete the
    //    initialization to `0x00000000`.
    //    -------------------------------------------------------------------------------------------------------
    LoaderSerialPutS("Clearing the remainder of the Page Directory\n");
    uint32_t *wrk = (uint32_t *)mmuLvl1Table;
    for (int i = 1; i < 0x400; i ++) {
        wrk[i] = 0;
    }


    //
    // -- The next step is to recursively map the Page Directory
    //    ------------------------------------------------------
    LoaderSerialPutS("Recursively mapping the Page Directory\n");
    PageEntry_t *recurs = &pageDirectory[1023];
    recurs->frame = ((archsize_t)mmuLvl1Table) >> 12;
    recurs->rw = 1;
    recurs->us = 1;
    recurs->p = 1;


    //
    // -- From here, we are able to use the standard mmu addresses to perform mappings.  However, we do not yet
    //    have enough built to be able to use the kernel functions -- namely the kernel is not yet mapped.
    //
    //    We start with creating page tables as needed to map the entire kernel.
    //    -----------------------------------------------------------------------------------------------------
    LoaderSerialPutS("Preparing to create page tables as needed for the kernel\n");
    for (archsize_t section = kernelStart; section < kernelEnd; section += 0x400000) {
        LoaderSerialPutS("Mapping Page Table for "); LoaderSerialPutHex(section); LoaderSerialPutChar('\n');
        PageEntry_t *pde = PD_ENTRY(section);
        LoaderSerialPutS(".. Checking PDE at "); LoaderSerialPutHex((uint32_t)pde); LoaderSerialPutChar('\n');

        if (pde->p == 0) {
            frame_t newFrame = NextEarlyFrame();
            LoaderSerialPutS("Making new table at frame "); LoaderSerialPutHex(newFrame); LoaderSerialPutChar('\n');

            if (newFrame < PHYS_OF(_kernelEnd) >> 12) {
                LoaderSerialPutS("Out of memory in MmuEarlyInit(); add another 4MB section\n");
                LoaderSerialPutS(".. Frame was "); LoaderSerialPutHex(newFrame); LoaderSerialPutChar('\n');
                LoaderSerialPutS(".. Limit is "); LoaderSerialPutHex(PHYS_OF(_kernelEnd) >> 12);
                        LoaderSerialPutChar('\n');
                Halt();
            }

            void *addr = (void *)(newFrame << 12);
            lMemSetB(addr, 0, FRAME_SIZE);

            pde->frame = newFrame;
            pde->us = 1;
            pde->rw = 1;
            pde->p = 1;
        }
    }


    //
    // -- pass #2, we will go back and map the individual pages to the kernel as loaded in memory
    //    ---------------------------------------------------------------------------------------
    LoaderSerialPutS("Mapping the actual pages for the kernel\n");
    for (archsize_t section = kernelStart; section < kernelEnd; section += 0x1000) {
        LoaderSerialPutS("Mapping Page Table for "); LoaderSerialPutHex(section); LoaderSerialPutChar('\n');
        PageEntry_t *pte = PT_ENTRY(section);
        if (pte->p != 0) {
            LoaderSerialPutS("This page is already mapped!!!  ");
            LoaderSerialPutHex((uint32_t)*((uint32_t *)pte));
            LoaderSerialPutChar('\n');

            HaltCpu();
        }

        frame_t frame = PHYS_OF(section) >> 12;
        LoaderSerialPutS(".. The PTE address is "); LoaderSerialPutHex((uint32_t)pte); LoaderSerialPutChar('\n');
        LoaderSerialPutS(".. The resulting frame is "); LoaderSerialPutHex(frame); LoaderSerialPutChar('\n');

        pte->frame = frame;
        pte->us = 1;
        pte->rw = 1;
        pte->p = 1;
    }


    //
    // -- From here on, we have access to the kernel functions without restriction; the laoder code will still be
    //    at ~1MB while many of the support functions will be at ~`0x80000000`.  Data may be at either location
    //    depending on the variable and the loader data will be at ~1MB.  The following `kprintf()` call illustrates
    //    this fact as the function is in kernel address space and the constant .rodata string is also in
    //    kernel address space, but we are still running in loader code at ~1MB.  The final task is to create a
    //    page table for the temporary addresses around `0xff400000`.
    //    ----------------------------------------------------------------------------------------------------------
    frame_t frame = NextEarlyFrame();
    kMemSetB((void *)(frame << 12), 0, FRAME_SIZE);

    PageEntry_t *pde = PD_ENTRY(MMU_CLEAR_FRAME);
    pde->frame = frame;
    pde->us = 1;
    pde->rw = 1;
    pde->p = 1;


#if DEBUG_MMU == 1
    LoaderSerialPutS("MmuEarlyInit() is complete\n");
    kprintf("At this point, the kernel is fully mapped!!!\n");
#endif
}


