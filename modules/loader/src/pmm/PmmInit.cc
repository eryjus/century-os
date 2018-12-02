//===================================================================================================================
//
//  PmmInit.cc -- Initialize the Physical Memory Manger's internal OS structure
//
//        Copyright (c)  2017-2018 -- Adam Clark
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
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2017-Jun-11  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "serial-loader.h"
#include "cpu.h"
#include "pmm.h"


#ifndef DEBUG_PMM
#   define DEBUG_PMM 0
#endif


//
// -- initialize the physical portion of the memory manager
//    -----------------------------------------------------
void PmmInit(void)
{
    extern char _loaderStart[];
    extern char _loaderEnd[];
    frame_t frame;
    size_t length;

    // -- Sanity check -- we cannot continue without a memory map
    if (!HaveMMapData()) {
        SerialPutS("PANIC: Unable to determine memory map; Century OS cannot initialize\n\n");
        Halt();
    }

    // -- OK, the first order of business is to determine how big and where to put the table
    size_t pages = (GetUpperMemLimit() >> (12 + 3 + 12)) + (GetUpperMemLimit()&0x7fff?1:0);
    uint32_t *start = (uint32_t *)(GetEbda()?GetEbda():GetAvailLowerMem());

    SerialPutS("Upper memory limit: "); SerialPutHex(GetUpperMemLimit()); SerialPutChar('\n');

    if (!start) {                               // -- if we are not dealing with an ebda (and therefore not x86)
        start = (uint32_t *)0x4000;             //      make it the first page after the hardware comm area
    } else {
        start -= PmmFrameToLinear(pages);       // -- and if x86 family adjust backwards
    }

    start = (uint32_t *)((ptrsize_t)start & ~0x0fff);

#if DEBUG_PMM == 1
    SerialPutHex(pages); SerialPutS(" pages are needed to hold the PMM Bitmap\n");
    SerialPutS("   these pages will start at "); SerialPutHex((uint32_t)start); SerialPutChar('\n');
#endif

    SetPmmBitmap(start);
    SetPmmFrameCount(pages);

    // -- pages now holds the bitmap aligned to 4K right up to the EBDA or 640K boundary; set to no available memory
    kMemSetB((void *)start, 0, PmmFrameToLinear(pages));

#if DEBUG_PMM == 1
    SerialPutS("PMM Bitmap cleared and ready for marking unusable space\n");
#endif

    // -- now we loop through the available memory and set the frames to be available
    for (int i = 0; i < GetMMapEntryCount(); i ++) {
        frame = PmmLinearToFrame(GetAvailMemStart(i));
        length = PmmLinearToFrame(GetAvailMemLength(i));

        SerialPutS("Grub Reports available memory at "); SerialPutHex(frame); SerialPutS(" for ");
                SerialPutHex(length); SerialPutS(" frames\n");

        PmmFreeFrameRange(frame, length);
    }

#if DEBUG_PMM == 1
    SerialPutS("Marking frames 0 and 1 used\n");
#endif

    // -- The GDT is at linear address 0 and make it unavailable
    PmmAllocFrame(0);

    // -- Page Directory is not available
    PmmAllocFrame(1);

    // -- The area between the EBDA and 1MB is allocated (if exists)
    if (GetEbda() != 0) {
#if DEBUG_PMM == 1
        SerialPutS("Marking Ebda as used\n");
#endif

        PmmAllocFrameRange(PmmLinearToFrame(GetEbda()), 0x100 - PmmLinearToFrame(GetEbda()));
    }

    // -- now that all our memory is available, set the loader space to be not available; _loader* already aligned
    frame_t ls = PmmLinearToFrame((ptrsize_t)_loaderStart);
    frame_t le = PmmLinearToFrame((ptrsize_t)_loaderEnd);

#if DEBUG_PMM == 1
    SerialPutS("Marking the loader space as used starting at frame "); SerialPutHex(ls); SerialPutS(" for ");
            SerialPutHex(le - ls); SerialPutS(" frames.\n");
#endif

    PmmAllocFrameRange(ls, le - ls);        // _loaderEnd is already page aligned, so no need to add 1 page.

#if DEBUG_PMM == 1
    SerialPutS("Marking the frame buffer space as used starting at frame ");
            SerialPutHex((ptrsize_t)GetFrameBufferAddr()); SerialPutS(" for "); SerialPutHex((1024 * 768 * 2) >> 12);
            SerialPutS(" frames.\n");
#endif

    // -- Allocate the Frame Buffer
    if ((ptrsize_t)GetFrameBufferAddr() < GetUpperMemLimit()) {
        PmmAllocFrameRange(PmmLinearToFrame((ptrsize_t)GetFrameBufferAddr()), (1024 * 768 * 2) >> 12);
    }

    // -- Allocate the loaded modules
    if (HaveModData()) {
        for (int i = 0; i < GetModCount(); i ++) {
            frame = PmmLinearToFrame(GetAvailModuleStart(i));
            length = PmmLinearToFrame(GetAvailModuleEnd(i)) - frame + 1;

#if DEBUG_PMM == 1
            SerialPutS("Marking the module "); SerialPutS(GetAvailModuleIdent(i));
                    SerialPutS(" space as used starting at frame "); SerialPutHex(frame); SerialPutS(" for ");
                    SerialPutHex(length); SerialPutS(" frames.\n");
#endif

            PmmAllocFrameRange(frame, length);
        }
    }


#if DEBUG_PMM == 1
            SerialPutS("Finally, marking the stack, hardware communication area, kernel heap, and this "
                    "bitmap frames as used.\n");
#endif
    // -- we have a 4K stack that is upper-bound at 2MB
    PmmAllocFrame(PmmLinearToFrame(0x200000 - 4096));

    // -- There is a frame that will hold the hardware structure
    PmmAllocFrame(PmmLinearToFrame(0x00003000));

    // -- here we will allocate 16 frames for the heap (See MIN_HEAP_SIZE)
    PmmAllocFrameRange(PmmLinearToFrame((ptrsize_t)(start) - 0x00010000), 0x00010000 >> 12);

    // -- Finally, we have to mark the bitmap itself as used
    PmmAllocFrameRange(PmmLinearToFrame((ptrsize_t)start), pages);

    SerialPutS("Phyiscal Memory Manager Initialized\n");
}
