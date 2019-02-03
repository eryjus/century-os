//===================================================================================================================
//
//  PmmStart.cc -- Start the Physical Memory Manager Process
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  --------------------------------------------------------------------------
//  2018-Nov-21  Initial   0.2.0   ADCL  Initial version
//
//===================================================================================================================


#include "mmu-kernel.h"
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
    0x00001000,             // stack location (fixed!)
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
// -- Some sanity checks to make sure that we do not mess up in `mmu.h`
//    -----------------------------------------------------------------
#if ((PROCESS_PAGE_DIR < 0xff400000) || (PROCESS_PAGE_DIR >= 0xff800000))
#   error "PmmStart() to work, PROCESS_PAGE_DIR must be between 0xff400000 and 0xff800000"
#endif

#if ((PROCESS_PAGE_TABLE < 0xff400000) || (PROCESS_PAGE_TABLE >= 0xff800000))
#   error "PmmStart() to work, PROCESS_PAGE_TABLE must be between 0xff400000 and 0xff800000"
#endif

#if ((PROCESS_STACK_BUILD < 0xff400000) || (PROCESS_STACK_BUILD >= 0xff800000))
#   error "PmmStart() to work, PROCESS_STACK_BUILD must be between 0xff400000 and 0xff800000"
#endif


//
// -- This function is specific to this file and will only map data to PROCESS_PAGE_DIR or PROCESS_PAGE_TABLE.
//    Both of these addresses already have a Page Table from the loader for address 0xff401000 for the
//    Exception Vector Table.
//    ---------------------------------------------------------------------------------------------------------
static inline void KernelMap(ptrsize_t addr, frame_t frame)
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
// -- This function is specific to this file and will only map data to PROCESS_PAGE_DIR or PROCESS_PAGE_TABLE.
//    Both of these addresses already have a Page Table from the loader for address 0xff401000 for the GDT/IDT.
//    ---------------------------------------------------------------------------------------------------------
static inline void KernelUnmap(ptrsize_t addr)
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
    int i;
    ptrsize_t v, p;

    if (!pmmMod) {
        kprintf ("PmmStart(): module is NULL\n");
        return;
    }

    ListInit(&pmmProcess.messages.list);
    ListInit(&pmmProcess.lockList.list);

    kprintf("PmmStart(): installing module for %s, located at %p\n", pmmMod->modIdent, pmmMod->modStart);

    //
    // -- The first order of business is to create the MMU Tables.
    //    --------------------------------------------------------
    frame_t pageTtl1Frame = pmmMod->cr3 >> 12;
    frame_t pageTtl2Frame = (pmmMod->modStart + pmmMod->modHdrSize + 4096) >> 12; // This is the ELF header and stack
    ptrsize_t stack = (pmmMod->modStart + pmmMod->modHdrSize) >> 12;              // Only skip the ELF header

    kprintf("PmmStart(): calculated pageTtl2Frame to be %p; tos to be %p\n", pageTtl2Frame, stack);

    Ttl1_t *ttl1 = (Ttl1_t *)PROCESS_PAGE_DIR;
    Ttl1_t *ttl1Entry;

    // -- Now, temporarily map the Page Directory into the kernel page table
    kprintf("PmmStart(): Setting up the tables to be managed\n");
    KernelMap(PROCESS_PAGE_DIR, pageTtl1Frame);
    KernelMap(PROCESS_PAGE_DIR + 0x1000, pageTtl1Frame + 1);
    KernelMap(PROCESS_PAGE_DIR + 0x2000, pageTtl1Frame + 2);
    KernelMap(PROCESS_PAGE_DIR + 0x3000, pageTtl1Frame + 3);
    KernelMap(PROCESS_PAGE_TABLE, pageTtl2Frame);

    // -- initialize the 2 tables to 0
    kprintf("PmmStart(): clearing the mmu tables\n");
    kMemSetB((void *)PROCESS_PAGE_DIR, 0, 4 * 4096);
    kMemSetB((void *)PROCESS_PAGE_TABLE, 0, 4096);

    // -- map the page table in the page directory TODO: fix this hard-code crap
    ttl1Entry = (Ttl1_t *)&ttl1[0];
    ttl1Entry->ttl2 = pageTtl2Frame;
    ttl1Entry->fault = 0b01;

    // -- map the ttl2 tables
    ttl1Entry = (Ttl1_t *)&ttl1[4092];
    ttl1Entry[0].ttl2 = pageTtl1Frame;
    ttl1Entry[0].fault = 0b01;
    ttl1Entry[1].ttl2 = pageTtl1Frame + 1;
    ttl1Entry[1].fault = 0b01;
    ttl1Entry[2].ttl2 = pageTtl1Frame + 2;
    ttl1Entry[2].fault = 0b01;
    ttl1Entry[3].ttl2 = pageTtl1Frame + 3;
    ttl1Entry[4].fault = 0b01;

    // -- now map the kernel into this page directory. this is actually quite easy by mapping the kernel page tables
    //    into the process's page directory.  We need to copy PDEs 768 to 1021 (mapped or not) from the kernel PD
    //    to the process PD.
    //    ----------------------------------------------------------------------------------------------------------
    kprintf("PmmStart(): Copying the kernel Page Tables\n");
    for (i = 2048; i < 4092; i ++) {
        ttl1[i] = ((Ttl1_t *)TTL1_KRN_VADDR)[i];
    }

    // -- we also need to map the PMM into the new address space
    //    ------------------------------------------------------
    kprintf("PmmStart(): Mapping the pmm itself\n");
    for (v = 0x00001000, p = pmmMod->modStart + pmmMod->modHdrSize; p <= pmmMod->modEnd; v += 4096, p += 4096) {
        Ttl2_t *e = &((Ttl2_t *)PROCESS_PAGE_TABLE)[(v >> 12) & 0xff];
        e->frame = p >> 12;
        e->s = 1;
        e->apx = 0;
        e->ap = 0b11;
        e->tex = 0b001;
        e->c = 1;
        e->b = 1;
        e->nG = 0;
        e->fault = 0b10;
    }

    // -- we can unmap these pages now
    KernelUnmap(PROCESS_PAGE_DIR);
    KernelUnmap(PROCESS_PAGE_TABLE);

    // -- Now for the magic of the initial stack.
    KernelMap(PROCESS_STACK_BUILD, stack);

    // -- clear the stack just for good measure
    kMemSetB((void *)PROCESS_STACK_BUILD, 0, 4096);

    kprintf("PmmStart(): Build the stack\n");
    pmmProcess.ss = 0x00;

    regval_t *msp = (regval_t *)(PROCESS_STACK_BUILD + 4096);

    // -- note there are no parameters; TODO: create a SYSCALL to self-terminate
    *(-- msp) = (ptrsize_t)0xff000000;                          // Force a page fault in the forbidden range
    *(-- msp) = (ptrsize_t)pmmMod->entry;                       // our entry point -- `lr`
    *(-- msp) = (regval_t)0;                                    // r13 or stack (thrown away)
    *(-- msp) = (regval_t)0;                                    // r12
    *(-- msp) = (regval_t)0;                                    // r11
    *(-- msp) = (regval_t)0;                                    // r10
    *(-- msp) = (regval_t)0;                                    // r9
    *(-- msp) = (regval_t)0;                                    // r8
    *(-- msp) = (regval_t)0;                                    // r7
    *(-- msp) = (regval_t)0;                                    // r6
    *(-- msp) = (regval_t)0;                                    // r5
    *(-- msp) = (regval_t)0;                                    // r4
    *(-- msp) = (regval_t)0;                                    // r3
    *(-- msp) = (regval_t)0;                                    // r2
    *(-- msp) = (regval_t)0;                                    // r1
    *(-- msp) = (regval_t)0;                                    // r0
    *(-- msp) = (regval_t)0;                                    // cspr

    pmmProcess.stackPointer = (regval_t)msp;
    pmmProcess.status = PROC_RUN;
    pmmProcess.pageTables = pmmMod->cr3;

    KernelUnmap(PROCESS_STACK_BUILD);
    kprintf("PmmStart(): All done\n");

    // -- These are legal without a lock because interrupts are still disabled
    procs[PID_PMM] = &pmmProcess;
    ListAddTail(&procOsPtyList, &pmmProcess.stsQueue);
}

