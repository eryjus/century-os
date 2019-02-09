//===================================================================================================================
//
//  PmmStart.cc -- Start the Physical Memory Manager Process
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  **** NOTE TO ANYONE INTERESTED IN THIS IMPLEMENTATION: ****
//  This implementation is VERY VERY VERY WRONG!  I am using the same paging tables as the kernel to implement
//  this user table.  This will only every work for a single process.  Period.  If knowing this, you look any
//  further at this implementation, you are dumber than I am for implementing it in the first place.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  --------------------------------------------------------------------------
//  2018-Nov-21  Initial   0.2.0   ADCL  Initial version
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "mmu.h"
#include "hw-disc.h"
#include "process.h"


//
// -- This is the purposeful process structure for the PMM
//    ----------------------------------------------------
Process_t pmmProcess = {
    0,                      // esp
    0,                      // ss
    0,                      // cr3
    PID_PMM,                // pid
    0x00007000,             // stack location (fixed!)
    4096,                   // stack length
    "pmm",                  // process name
    0,                      // total quantum
    PROC_INIT,              // process status
    PTY_OS,                 // the priority
    0,                      // quantum left
    false,                  // not held
    {0},                    // the spinlock for this process
    {0},                    // the status queue
    {0},                    // the lock list
    {0},                    // pending messages
    NULL,                   // the previous payload
};


//
// -- This function is specific to this file and will only map data to TTL1_KRN_VADDR or TTL2_KRN_VADDR.
//    Both of these addresses already have a Page Table from the loader for address 0xff401000 for the
//    Exception Vector Table.
//    ---------------------------------------------------------------------------------------------------------
static inline void KernelMap(archsize_t addr, frame_t frame)
{
    kprintf("PmmStart(): Mapping addr %p to frame %p\n", addr, frame);
    Ttl2_t *ttl2 = &((Ttl2_t *)TTL2_KRN_VADDR)[(addr >> 12) & 0xfffff];
    kprintf("  The address of the TTL2 Entry is at: %p\n", ttl2);

    if (ttl2->fault) {
        kprintf("     !! Already mapped to frame %p\n", ttl2->frame);
        return;
    }

    ttl2->frame = frame;
    ttl2->s = 1;
    ttl2->apx = 0;
    ttl2->ap = 0b11;
    ttl2->tex = 0b001;
    ttl2->c = 0;
    ttl2->b = 0;
    ttl2->nG = 0;
    ttl2->fault = 0b10;
}


//
// -- This function is specific to this file and will only map data to TTL1_KRN_VADDR or TTL2_KRN_VADDR.
//    Both of these addresses already have a Page Table from the loader for address 0xff401000 for the GDT/IDT.
//    ---------------------------------------------------------------------------------------------------------
static inline void KernelUnmap(archsize_t addr)
{
    Ttl2_t *ttl2 = &((Ttl2_t *)0xffffd000)[(addr >> 12) & 0x3ff];

    if (!ttl2->fault) return;

    kMemSetB(ttl2, 0, sizeof(Ttl2_t));
    // TODO: Invalidate page here
}


//
// -- Create the process for the PMM
//    ------------------------------
void PmmStart(Module_t *pmmMod)
{
//    archsize_t v, p;

    if (!pmmMod) {
        kprintf ("PmmStart(): module is NULL\n");
        return;
    }

    ListInit(&pmmProcess.messages.list);
    ListInit(&pmmProcess.lockList.list);

    kprintf("PmmStart(): installing module for %s, located at %p\n", pmmMod->modIdent, pmmMod->modStart);
    kprintf("PmmStart(): Build the \"Magic Stack\"\n");
    pmmProcess.ss = (archsize_t)0x13;

    archsize_t *msp = (archsize_t *)(0x8000 + 4096);

    // -- note there are no parameters; TODO: create a SYSCALL to self-terminate
    *(-- msp) = (archsize_t)0;                                    // r12
    *(-- msp) = (archsize_t)0;                                    // r11
    *(-- msp) = (archsize_t)0;                                    // r10
    *(-- msp) = (archsize_t)0;                                    // r9
    *(-- msp) = (archsize_t)0;                                    // r8
    *(-- msp) = (archsize_t)0;                                    // r7
    *(-- msp) = (archsize_t)0;                                    // r6
    *(-- msp) = (archsize_t)0;                                    // r5
    *(-- msp) = (archsize_t)0;                                    // r4
    *(-- msp) = (archsize_t)0;                                    // r3
    *(-- msp) = (archsize_t)0;                                    // r2
    *(-- msp) = (archsize_t)0;                                    // r1
    *(-- msp) = (archsize_t)0;                                    // r0
    *(-- msp) = (archsize_t)pmmMod->entry;                       // our entry point -- `lr` or r14

    kprintf("Setting the PMM Entry Point to be %p\n", pmmMod->entry);

    pmmProcess.stackPointer = (archsize_t)msp;
    pmmProcess.status = PROC_RUN;
    pmmProcess.pageTables = pmmMod->cr3;

    kprintf("PmmStart(): All done\n");

    // -- These are legal without a lock because interrupts are still disabled
    procs[PID_PMM] = &pmmProcess;
    ListAddTail(&procOsPtyList, &pmmProcess.stsQueue);
}

