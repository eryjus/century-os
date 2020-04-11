//===================================================================================================================
//
//  ButlerInit.cc -- Initialize the Butler process and perform the initial cleanup
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  Notice that this function appears in the kernel section, not the loader section as with all the other init jobs.
//  The reason is that while this will be called once, it will also remove the loader section from memory and free
//  the PMM frames, meaning it would be destroying itself.  Undesirable results.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Apr-10  Initial  v0.6.1b  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "process.h"
#include "msgq.h"
#include "mmu.h"
#include "pmm.h"
#include "butler.h"


//
// -- The name of the butler process
//    ------------------------------
EXPORT KERNEL_DATA
const char *butlerName = "Butler";


//
// -- This is the message queue the butler will use
//    ---------------------------------------------
EXPORT KERNEL_BSS
MessageQueue_t *butlerMsgq;


//
// -- Initialize the Butler and perform the initial cleanup
//    -----------------------------------------------------
EXTERN_C EXPORT KERNEL
void ButlerInit(void)
{
    archsize_t flags = DisableInterrupts();
    currentThread->command = (char *)butlerName;            // usually heap memory, conversion required
    currentThread->priority = PTY_LOW;
    RestoreInterrupts(flags);


    // -- unmap any memory below 1 MB
    for (archsize_t addr = 0; addr < 0x100000; addr += PAGE_SIZE) {
        MmuUnmapPage(addr);
    }


    // -- free any available low memory
    for (frame_t frame = 0; frame < 0x100; frame ++) {
        if (LowMemCheck(frame)) PmmReleaseFrame(frame);
    }
}

