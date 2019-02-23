//===================================================================================================================
//
//  PmmInit.cc -- Initialize the Physical Memory Manger's internal OS structure
//
//        Copyright (c)  2017-2019 -- Adam Clark
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
#include "hw-disc.h"
#include "loader.h"
#include "pmm.h"


#define DEBUG_PMM 1

#ifndef DEBUG_PMM
#   define DEBUG_PMM 0
#endif


//
// -- initialize the physical portion of the memory manager
//    -----------------------------------------------------
void __ldrtext PmmInit(void)
{
    extern char _kernelEnd[];           // This is the 1 byte past last frame that was loaded
    extern frame_t pmmLastLook;
    extern frame_t pmmLimit;
    frame_t frame;
    size_t length;


    kprintf("Startng PMM initialization\n");


    // -- Sanity check -- we cannot continue without a memory map
    if (!HaveMMapData()) {
        kprintf("PANIC: Unable to determine memory map; Century OS cannot initialize\n\n");
        HaltCpu();
    }


    //
    // -- OK, the first order of business is to determine how big and where to put the table.  This will need to
    //    be big enough to hold the last byte on the system.  Therefore the range will be from byte 0 to the
    //    biggest mmap entry we have available for allocation.
    //    ------------------------------------------------------------------------------------------------------
    uint64_t bmLength  = 0;

    for (int i = 0; i < GetMMapEntryCount(); i ++) {
        if (bmLength < GetAvailMemStart(i) + GetAvailMemLength(i)) {
            bmLength = GetAvailMemStart(i) + GetAvailMemLength(i);
        }
    }

#if DEBUG_PMM == 1
    kprintf("Upper memory limit: %p : %p\n", (uint32_t)(bmLength >> 32), (uint32_t)(bmLength & 0xffffffff));
#endif


    //
    // -- now, calculate the number of frames needed to hold the pmm bitmap.  Each single bit indicates the
    //    allocation of a single frame.  Therefore a page (4096 bytes) can map 4096 * 8 frames or 32768 frames.
    //    With a frame being 4096 bytes, the amount of memory that can be managed by a page is 32768 * 4096 bytes
    //    or 128MB.  128MB can be addressed up to 0x08000000, or with 27 bits.  Therefore the math below.
    //    -------------------------------------------------------------------------------------------------------
    size_t pages = (bmLength >> (12 + 3 + 12)) + (bmLength&0x7ffffff?1:0);
    pmmEarlyFrame -= pages;
    frame_t bitmap = pmmEarlyFrame + 1;
    uint32_t *start = (uint32_t *)(bitmap << 12);


#if DEBUG_PMM == 1
    kprintf("%x pages are needed to hold the PMM Bitmap\n", pages);
    kprintf("   these pages will start at %p\n", (uint32_t)start);
#endif

    SetPmmBitmap(start);
    SetPmmFrameCount(pages);


    //
    // -- clear the bitmap -- everything is allocated to start
    //    ----------------------------------------------------
    kMemSetB((void *)start, 0, PmmFrameToLinear(pages));

#if DEBUG_PMM == 1
    kprintf("PMM Bitmap cleared and ready for marking unusable space\n");
#endif


    //
    // -- now we loop through the available memory and set the frames to be available
    //    ---------------------------------------------------------------------------
    for (int i = 0; i < GetMMapEntryCount(); i ++) {
        frame = PmmLinearToFrame(GetAvailMemStart(i));
        length = PmmLinearToFrame(GetAvailMemLength(i));

#if DEBUG_PMM == 1
        kprintf("Grub Reports available memory at %p for %p frames\n", frame, length);
#endif

        PmmFreeFrameRange(frame, length);
    }

#if DEBUG_PMM == 1
    kprintf("Marking the first 1MB and kernel binary frames as used, up to %x\n", PHYS_OF(_kernelEnd) >> 12);
#endif

    PmmAllocFrameRange(0, PHYS_OF(_kernelEnd) >> 12);
    PmmAllocFrameRange((pmmEarlyFrame >> 12) + 1, 0x3ff - (pmmEarlyFrame >> 12));


    // -- Allocate the Frame Buffer
    if ((uint64_t)GetFrameBufferAddr() < GetUpperMemLimit() && GetFrameBufferAddr()) {
#if DEBUG_PMM == 1
            kprintf("Marking the frame buffer space as used starting at frame %p for %p frames.\n",
                    GetFrameBufferAddr(), (1024 * 768 * 2) >> 12);
#endif

        PmmAllocFrameRange(PmmLinearToFrame((archsize_t)GetFrameBufferAddr()), (1024 * 768 * 2) >> 12);
    }

    // -- Allocate the loaded modules
    if (HaveModData()) {
        for (int i = 0; i < GetModCount(); i ++) {
            frame = PmmLinearToFrame(GetAvailModuleStart(i));
            length = PmmLinearToFrame(GetAvailModuleEnd(i)) - frame + 1;

#if DEBUG_PMM == 1
            kprintf("Marking the module %s space as used starting at frame %p for %p frames.\n",
                    GetAvailModuleIdent(i), frame, length);
#endif

            PmmAllocFrameRange(frame, length);
        }
    }

#if DEBUG_PMM == 1
    kprintf("Finally, marking the stack, hardware communication area, kernel heap, and this "
                "bitmap frames as used.\n");
#endif

    //
    // -- Finally, we have to mark the bitmap itself as used and other early init allocations.
    //    ------------------------------------------------------------------------------------
    PmmAllocFrameRange(pmmEarlyFrame, 0x000003ff - pmmEarlyFrame);


    //
    // -- Complete the setup so that we can use the general `PmmNewFrame()` function to allocate
    //    --------------------------------------------------------------------------------------
    pmmLastLook = 0x00000400;                        // -- we will look from 4MB on...
    pmmLimit = (frame_t)(bmLength >> 12);            // -- this is how far we will look

    kprintf("Phyiscal Memory Manager Initialized\n");
}
