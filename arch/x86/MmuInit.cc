//===================================================================================================================
//
//  MmuInit.cc -- Complete the initialization for the MMU for x86
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  Finally, complete all the remaining mapping needed to get into the kernel.
//
//  These are:
//  * framebuffer mappings
//  * IDT/GDT/TSS mapping
//  * Map a kernel stack
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Feb-15  Initial   0.3.0   ADCL  Initial version
//
//===================================================================================================================


#include "loader.h"
#include "types.h"
#include "pmm.h"
#include "interrupt.h"
#include "mmu.h"


//
// -- complete the mmu initialization
void __ldrtext MmuInit(void)
{
    //
    // -- Start by mapping the frame buffer
    //    ---------------------------------
    archsize_t fbAddr = ((archsize_t)GetFrameBufferAddr());        // better be aligned to frame boundary!!!
    size_t fbSize = GetFrameBufferPitch() * GetFrameBufferHeight();
    fbSize += (fbSize&0xfff?0x1000:0);      // -- this is adjusted so that when we `>> 12` we are mapping enough
    fbSize >>= 12;                          // -- now, the number of frames to map
    archsize_t off = 0;

    kprintf("MMU: Mapping the frame buffer at %p for %x frames\n", fbAddr, fbSize);

    while (fbSize) {
        kprintf(".. Executing map of %p to %p\n", MMU_FRAMEBUFFER + off, fbAddr >> 12);
        MmuMapToFrame(MMU_FRAMEBUFFER + off, fbAddr >> 12, PG_KRN);
        off += 0x1000;
        fbAddr += 0x1000;
        fbSize --;
    }

    // -- goose the config to the correct fb address
    SetFrameBufferAddr((uint16_t *)MMU_FRAMEBUFFER);


    //
    // -- Next up is the VBAR -- which needs to be mapped.  This one is rather trivial.
    //    -----------------------------------------------------------------------------
    MmuMapToFrame(EXCEPT_VECTOR_TABLE, intTableAddr >> 12, PG_KRN);


    //
    // -- the next order of business is to set up the stack (but do not yet change to it -- we are in a function
    //    and will need to clean up and return from this function and getting that right is risky).
    //    ------------------------------------------------------------------------------------------------------
    archsize_t stackLoc = STACK_LOCATION;
    for (int i = 0; i < STACK_SIZE; i += 0x1000, stackLoc += 0x1000) {
        MmuMapToFrame(stackLoc, PmmAllocateFrame(), PG_KRN);
    }


    //
    // -- Take care of some additional archicecture-specific initialization dependent on the MMU Setup being
    //    complete.
    //    --------------------------------------------------------------------------------------------------
    ExceptionInit();
    CpuTssInit();

    kprintf("MMU: The MMU is initialized\n");
}



