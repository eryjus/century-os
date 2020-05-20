//===================================================================================================================
//
// ProcessPrepareFromImage.cc -- Create a new process from the image passed in
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Mar-16  Initial   0.3.2   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "mmu.h"
#include "pmm.h"
#include "heap.h"
#include "elf.h"
#include "process.h"


//
// -- Create a new process and get it ready to be scheduled
//    -----------------------------------------------------
EXTERN_C EXPORT KERNEL
Process_t *ProcessPrepareFromImage(ElfImage_t *img, ElfHdrCommon_t *hdrShort, const char *name)
{
    archsize_t save = MmuGetTopTable(0);
    archsize_t newMmu = PmmAllocateFrame();

    kprintf("The temporary top level paging tables will be %p\n", newMmu);

    MmuMakeTopUserTable(newMmu);
    MmuSetTopUserTable(newMmu << 12);

    kprintf("New top level paging tables set\n");


    // -- create the process structure
    Process_t *rv = NEW(Process_t);
    if (!assert_msg(rv != NULL, "Out of memory allocating a new Process_t")) {
        CpuPanicPushRegs("Out of memory allocating a new Process_t");
    }

    kMemSetB(rv, 0, sizeof(Process_t));
    rv->pid = scheduler.nextPID ++;

    // -- set the name of the process
    int len = kStrLen(name + 1);
    rv->command = (char *)HeapAlloc(len, false);
    rv->command[len + 1] = 0;
    kStrCpy(rv->command, name);

    rv->policy = POLICY_0;
    rv->priority = PTY_NORM;
    rv->status = PROC_INIT;
    AtomicSet(&rv->quantumLeft, 0);
    rv->timeUsed = 0;
    rv->wakeAtMicros = 0;
    ListInit(&rv->stsQueue);
    ListInit(&rv->references.list);

    //
    // -- Put this process on the queue to execute
    //    ----------------------------------------
    ProcessAddGlobal(rv);


    // -- read the elf header and perform the mappings here
    Elf32EHdr_t *hdr = (Elf32EHdr_t *)hdrShort;
    Elf32PHdr_t *pgmHdr32 = (Elf32PHdr_t *)((archsize_t)hdr + hdr->ePhOff);
    void (*startingAddr)() = (void(*)())hdr->eEntry;

    // -- loop through all the load instructions
    for (int pSeg = 0; pSeg < hdr->ePhNum; pSeg ++) {
        size_t offsetStart = pgmHdr32[pSeg].pOffset >> 12;
        size_t offsetEnd = offsetStart + ((pgmHdr32[pSeg].pMemSz + (PAGE_SIZE - 1)) >> 12) - 1;
        size_t offsetFEnd = offsetStart + ((pgmHdr32[pSeg].pFileSz + (PAGE_SIZE - 1)) >> 12) - 1;

        // -- Now, loop through each page to be mapped for each program header

        size_t offset;
        archsize_t vAddr;

        for (offset = offsetStart, vAddr = pgmHdr32[pSeg].pVAddr; offset <= offsetEnd; offset ++, vAddr += 0x1000) {
            //    -----------------------------------------------------------------------------------------------------
            // -- OK, here there are 3 situations to handle: 1) The page gets mapped to a frame clean; 2) The page
            //    gets mapped to a frame and there is some memory at the last byte to clear (either some or all of
            //    the rest of the page); or 3) Another page needs to be allocated and cleared for additional bss space.
            //
            //    PROGRAMMER'S NOTE:  be careful of this next section, which is a chain of if-then-elseif's, improperly
            //    indented.
            //    -----------------------------------------------------------------------------------------------------


            //
            // -- Ok Start with scenario 1, clearly when offset < offsetFEnd
            //    ----------------------------------------------------------
            if (offset < offsetFEnd) {
                MmuMapToFrame(vAddr, img->frame[offset], (pgmHdr32[pSeg].pFlags&PF_W?PG_WRT:0));
            }

            // -- scenario 1 part 2: offset == offsetPEnd && pgmHdr32[pSeg].pMemSz == pgmHdr32[pSeg].pFileSz
            else if (offset == offsetFEnd && pgmHdr32[pSeg].pMemSz == pgmHdr32[pSeg].pFileSz) {
                MmuMapToFrame(vAddr, img->frame[offset], (pgmHdr32[pSeg].pFlags&PF_W?PG_WRT:0));
            }


            //
            // -- Scenario 2: offset == offsetPEnd && pgmHdr32[pSeg].pMemSz > pgmHdr32[pSeg].pFileSz
            //    ----------------------------------------------------------------------------------
            else if (offset == offsetFEnd && pgmHdr32[pSeg].pMemSz > pgmHdr32[pSeg].pFileSz) {
                MmuMapToFrame(vAddr, img->frame[offset], (pgmHdr32[pSeg].pFlags&PF_W?PG_WRT:0));
                size_t remainingBytes = pgmHdr32[pSeg].pFileSz - pgmHdr32[pSeg].pMemSz;
                archsize_t bytesToPage = PAGE_SIZE - (pgmHdr32[pSeg].pMemSz - (pgmHdr32[pSeg].pMemSz & ~(PAGE_SIZE - 1)));
                size_t bytes = (remainingBytes<bytesToPage?remainingBytes:bytesToPage);

                kMemSetB((void *)(vAddr + (pgmHdr32[pSeg].pMemSz & ~(PAGE_SIZE - 1))), 0, bytes);
            }


            //
            // -- whatever is left is a new frame (already guaranteed to be zeroed)
            //    -----------------------------------------------------------------
            else {
                MmuMapToFrame(vAddr, PmmAllocateFrame(), (pgmHdr32[pSeg].pFlags&PF_W?PG_WRT:0));
            }
        }
    }


    // -- Finally prepare the kernel stack
    rv->tosKernel = KERNEL_STACK + STACK_SIZE;
    rv->ssKernFrame = PmmAllocateFrame();
    MmuMapToFrame(KERNEL_STACK, rv->ssKernFrame, PG_KRN | PG_WRT);


    //
    // -- Construct the user stack for the architecture
    //    ---------------------------------------------
    rv->ssProcFrame = ProcessNewUserStack(rv, startingAddr);


    //
    // -- Construct the new addres space for the process
    //    ----------------------------------------------
    rv->virtAddrSpace = newMmu << 12;           // -- shift to proper bits

#if DEBUG_ENABLED(ProcessCreate)
    kprintf("ProcessCreate() created a new process at %p\n", rv);
#endif


    MmuSetTopUserTable(save);
    return rv;
}
