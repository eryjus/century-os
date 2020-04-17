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
#include "spinlock.h"
#include "heap.h"
#include "butler.h"


//
// -- The name of the butler process
//    ------------------------------
EXPORT KERNEL_DATA
const char *butlerName = "Butler";


//
// -- Initialize the Butler and perform the initial cleanup
//    -----------------------------------------------------
EXTERN_C EXPORT KERNEL
void ButlerInit(void)
{
    // -- kernel text location
    EXTERN uint8_t txtStart[];
    EXTERN uint8_t txtEnd[];
    EXTERN archsize_t txtPhys;
    EXTERN archsize_t txtSize;

    krnKernelTextStart = txtStart;
    krnKernelTextEnd = txtEnd;
    krnKernelTextPhys = txtPhys;
    krnKernelTextSize = txtSize;


    // -- kernel data location
    EXTERN uint8_t dataStart[];
    EXTERN uint8_t bssEnd[];
    EXTERN archsize_t dataPhys;
    EXTERN archsize_t dataSize;

    krnKernelDataStart = dataStart;
    krnKernelDataEnd = bssEnd;
    krnKernelDataPhys = dataPhys;
    krnKernelDataSize = dataSize;


    // -- kernel syscall location
    EXTERN uint8_t sysStart[];
    EXTERN uint8_t sysEnd[];
    EXTERN archsize_t sysPhys;
    EXTERN archsize_t sysSize;

    krnSyscallStart = sysStart;
    krnSyscallEnd = sysEnd;
    krnSyscallPhys = sysPhys;
    krnSyscallSize = sysSize;


    // -- stab location
    EXTERN uint8_t stabStart[];
    EXTERN uint8_t stabEnd[];
    EXTERN archsize_t stabPhys;
    EXTERN archsize_t stabSize;

    krnStabStart = stabStart;
    krnStabEnd = stabEnd;
    krnStabPhys = stabPhys;
    krnStabSize = stabSize;


    // -- data needed to clean up the entry point
    EXTERN uint8_t mbStart[];
    EXTERN uint8_t mbEnd[];
    EXTERN archsize_t mbPhys;

    uint8_t *krnMbStart = mbStart;
    uint8_t *krnMbEnd = mbEnd;
    archsize_t krnMbPhys = mbPhys;


    // -- data needed to clean up the loader
    EXTERN uint8_t ldrStart[];
    EXTERN uint8_t ldrEnd[];
    EXTERN archsize_t ldrPhys;

    uint8_t *krnLdrStart = ldrStart;
    uint8_t *krnLdrEnd = ldrEnd;
    archsize_t krnLdrPhys = ldrPhys;


    // -- data need to clean up the smp block (better be 1 page)
//    EXTERN uint8_t smpStart[];
//    EXTERN archsize_t smpPhys;
    EXTERN archsize_t smpSize;

//    uint8_t *krnSmpStart = smpStart;
//    archsize_t krnSmpPhys = smpPhys;
    archsize_t krnSmpSize = smpSize;


    // -- Change our identity
    archsize_t flags = DisableInterrupts();
    currentThread->command = (char *)butlerName;            // usually heap memory, conversion required
    currentThread->priority = PTY_LOW;
    RestoreInterrupts(flags);


    //
    // -- up to this point we have had access to the multiboot entry code; not any more
    //    -----------------------------------------------------------------------------

    // -- unmap any memory below 1 MB
    kprintf("Freeing MMU pages below 1MB\n");
    for (archsize_t addr = 0; addr < 0x100000; addr += PAGE_SIZE) {
//        if (MmuIsMapped(addr)) {
//            kprintf(".. freeing %p\n", addr);
//            MmuUnmapPage(addr);
//        }
    }


    // -- free any available memory < 4MB
    kprintf("Freeing PMM frames up to 4MB\n");
    for (frame_t frame = 0; frame < 0x400; frame ++) {
//        if (ButlerMemCheck(frame)) PmmReleaseFrame(frame);
    }


    //
    // -- up to this point, we have access to all the loader code; not any more
    //    ---------------------------------------------------------------------

    // -- Clean up the SMP code
    kprintf("Freeing SMP Init code\n");
    if (krnSmpSize) {
//        MmuUnmapPage((archsize_t)krnSmpStart);
//        PmmReleaseFrame(krnSmpPhys >> 12);
    }

    // -- Clean up the loader
    kprintf("Freeing Loader code\n");
    while (krnLdrStart < krnLdrEnd) {
//        MmuUnmapPage((archsize_t)krnLdrStart);
//        PmmReleaseFrame(krnLdrPhys >> 12);
        krnLdrStart += PAGE_SIZE;
        krnLdrPhys += PAGE_SIZE;
    }

    // -- Clean up the multiboot entry
    kprintf("Freeing multiboot entry point\n");
    while (krnMbStart < krnMbEnd) {
//        MmuUnmapPage((archsize_t)krnMbStart);
//        PmmReleaseFrame(krnMbPhys >> 12);
        krnMbStart += PAGE_SIZE;
        krnMbPhys += PAGE_SIZE;
    }
}

