//===================================================================================================================
//
// loader/src/pmm/PmmInit.cc -- Initialize the Physical Memory Manger's internal OS structure
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// This function will initialize the Physical Memory Manager (PMM).  The PMM is implemented as a bitmap, where
// a bit flag in an array of bits will indicate if the frame is available or taken.
//
// There are several steps that are needed to be completed to completely initialize the PMM.  These are:
// 1. Determine where and how big to make the PMM -- this is dependent on the upper memory limit which will
//    determine the number of frames we need to keep track of.  Allocate this memory space in the physical memory.
// 2. Set every thing to be allocated.  This is a necessary step as we cannot guarantee that the multiboot
//    information contains all the unusable holes in memory.  So we will assume that unless explicitly available,
//    the frame is not available.
// 3. Set all the available memory from the multiboot information to be available.  This will be all the volatile
//    memory on the system.
// 4. Go through and mark all the frames that have been used appropriately.  This wil be the loader, several
//    additional OS structures, the loaded modules, and even the video buffer.  This will include the bitmap itself.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date     Tracker  Version  Pgmr  Description
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2017-06-11  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "console.h"
#include "serial.h"
#include "cpu.h"
#include "pmm.h"


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

    start -= PmmFrameToLinear(pages);
    start = (uint32_t *)((ptrsize_t)start & ~0x0fff);

    SetPmmBitmap(start);
    SetPmmFrameCount(pages);

    // -- pages now holds the bitmap aligned to 4K right up to the EBDA or 640K boundary; set to no available memory
    kMemSetB((void *)start, 0, PmmFrameToLinear(pages));

    // -- now we loop through the available memory and set the frames to be available
    for (int i = 0; i < GetMMapEntryCount(); i ++) {
        frame = PmmLinearToFrame(GetAvailMemStart(i));
        length = PmmLinearToFrame(GetAvailMemLength(i));

        PmmFreeFrameRange(frame, length);
    }

    // -- The GDT is at linear address 0 and make it unavailable
    PmmAllocFrame(0);

    // -- Page Directory is not available
    PmmAllocFrame(1);

    // -- The area between the EBDA and 1MB is allocated
    PmmAllocFrameRange(PmmLinearToFrame(GetEbda()), 0x100 - PmmLinearToFrame(GetEbda()));

    // -- now that all our memory is available, set the loader space to be not available; _loader* already aligned
    frame_t ls = PmmLinearToFrame((ptrsize_t)_loaderStart);
    frame_t le = PmmLinearToFrame((ptrsize_t)_loaderEnd);
    PmmAllocFrameRange(ls, le - ls);        // _loaderEnd is already page aligned, so no need to add 1 page.

    // -- Allocate the Frame Buffer
    PmmAllocFrameRange(PmmLinearToFrame((ptrsize_t)GetFrameBufferAddr()), 1024 * 768 * 2);

    // -- Allocate the loaded modules
    if (HaveModData()) {
        for (int i = 0; i < GetModCount(); i ++) {
            frame = PmmLinearToFrame(GetAvailModuleStart(i));
            length = PmmLinearToFrame(GetAvailModuleEnd(i)) - frame + 1;

            PmmAllocFrameRange(frame, length);
        }
    }

    // -- Finally, we have to mark the bitmap itself as used
    PmmAllocFrameRange(PmmLinearToFrame((ptrsize_t)start), pages);

    SerialPutS("Phyiscal Memory Manager Initialized\n");
}
