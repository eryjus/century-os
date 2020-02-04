//===================================================================================================================
//
//  PmmInit.cc -- Initialize the Physical Memory Manger's internal OS structure
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  This function will initialize the Physical Memory Manager (PMM).  The PMM is implemented as a bitmap, where
//  a bit flag in an array of bits will indicate if the frame is available or taken.
//
//  There are several steps that are needed to be completed to completely initialize the PMM.  These are:
//  1. Determine where and how big to make the PMM -- this is dependent on the upper memory limit which will
//     determine the number of frames we need to keep track of.  Allocate this memory space in the physical memory.
//  2. Set every thing to be allocated.  This is a necessary step as we cannot guarantee that the multiboot
//     information contains all the unusable holes in memory.  So we will assume that unless explicitly available,
//     the frame is not available.
//  3. Set all the available memory from the multiboot information to be available.  This will be all the volatile
//     memory on the system.
//  4. Go through and mark all the frames that have been used appropriately.  This wil be the loader, several
//     additional OS structures, the loaded modules, and even the video buffer.  This will include the bitmap
//     itself.
//
//  A change has been made so that I no longer need to worry about storing the pmm bitmap in low memory on x86.
//  Therefore, this function simplifies greatly.
//
//  At the same time, I have been able to remove the crappy pre-allocation of heap frames.  Since we have the
//  ability to allocate now in real time.  I have eliminated the chicken-and-egg problem I had before.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2017-Jun-11  Initial   0.1.0   ADCL  Initial version
//  2019-Feb-14  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "types.h"
#include "serial.h"
#include "printf.h"
#include "cpu.h"
#include "heap.h"
#include "hw-disc.h"
#include "pmm.h"


#define DEBUG_PMM 1

#ifndef DEBUG_PMM
#   define DEBUG_PMM 0
#endif


//
// -- initialize the physical portion of the memory manager
//    -----------------------------------------------------
EXTERN_C EXPORT LOADER
void PmmInit(void)
{
    extern bool pmmInitialized;

    kprintf("Startng PMM initialization\n");


    // -- regardless of anything else, we need to initialize the lists
    ListInit(&pmm.normalStack.list);
    ListInit(&pmm.lowStack.list);
    ListInit(&pmm.scrubStack.list);


    // -- Sanity check -- we cannot continue without a memory map
    if (!HaveMMapData()) {
        kprintf("PANIC: Unable to determine memory map; Century OS cannot initialize\n\n");
        HaltCpu();
    }


    //
    // -- Now simply loop through the memory map and add the blocks to the scrubStack.  The only catch here is that
    //    we will not deal with the first 4MB of memory, saving that for the cleanup after boot.
    //    ---------------------------------------------------------------------------------------------------------
    for (int i = 0; i < GetMMapEntryCount(); i ++) {
        uint64_t start = GetAvailMemStart(i);
        uint64_t end = start + GetAvailMemLength(i);

        if ((start & 0xffffffff00000000) != 0) continue;      // -- if it is above 32-bit space, it is not usable
        if (end > 0x100000000) end = 0x100000000;      // -- yes, this is a 9-digit hex number!!

        frame_t frame = (archsize_t)(start >> 12);
        size_t count = (archsize_t)((end - start) >> 12);

        // -- skip anything before 4MB
        if (frame < earlyFrame && count < earlyFrame) continue;
        if (frame < earlyFrame) {
            count -= (earlyFrame - frame);
            frame = earlyFrame;
        }

        kprintf("Releasing block of memory from frame %x for a count of %x frames\n", frame, count);

        PmmBlock_t *block = NEW(PmmBlock_t);
        ListInit(&block->list);
        block->frame = frame;
        block->count = count;
        CLEAN_PMM_BLOCK(block);

        //
        // -- since we are guaranteed to be above 1MB, this is all the normal queue
        //    ---------------------------------------------------------------------
        Push(&pmm.normalStack, &block->list);
        pmm.normalStack.count = block->count;
    }

    CLEAN_PMM();


    //
    // -- TODO: Do I need to address the framebuffer here?
    //    ------------------------------------------------
    kprintf("The frame buffer is located at %p\n", GetFrameBufferAddr());


    pmmInitialized = true;
    kprintf("Phyiscal Memory Manager Initialized\n");
}
