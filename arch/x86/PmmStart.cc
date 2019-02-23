//===================================================================================================================
//
//  PmmStart.cc -- Start the Physical Memory Manager Process
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  This is a purpose-built function to load the PMM process into the structures and start it working.  This
//  function is required because the point in the kernel initialization where we launch this process.  I need
//  the PMM fully initialized before I can start using the Heap, which is used to create processes.
//
//  To overcome this limitation, I am putting the stack in the PMM binary.  It will take up some additional space,
//  but the result is well worth it: I can load it very early in the kernel initialization.
//
//  The coding challenge here is that this function needs to determine where the physical stack is so that it can
//  be mapped temporarily to populate it.
//
//  Note: at the point we are executing this function, we are still a single process, so there is no need for
//  locking the spinlocks.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  --------------------------------------------------------------------------
//  2018-Nov-03  Initial   0.1.0   ADCL  Initial version
//  2019-Feb-09  Initial   0.3.0   ADCL  Relocated
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
    0x80000000,             // stack location (fixed!)
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
//    Both of these addresses already have a Page Table from the loader for address 0xff401000 for the GDT/IDT.
//    ---------------------------------------------------------------------------------------------------------
static inline void KernelMap(archsize_t addr, frame_t frame)
{
    kprintf("PmmStart(): Mapping addr %p to frame %p\n", addr, frame);
    PageEntry_t *pte = &((PageEntry_t *)0xffffd000)[(addr >> 12) & 0x3ff];

    if (pte->p) {
        kprintf("     !! Already mapped to frame %p\n", pte->frame);
        return;
    }

    pte->frame = frame;
    pte->p = 1;
    pte->rw = 1;
    pte->us = 1;
}


//
// -- This function is specific to this file and will only map data to PROCESS_PAGE_DIR or PROCESS_PAGE_TABLE.
//    Both of these addresses already have a Page Table from the loader for address 0xff401000 for the GDT/IDT.
//    ---------------------------------------------------------------------------------------------------------
static inline void KernelUnmap(archsize_t addr)
{
    PageEntry_t *pte = &((PageEntry_t *)0xffffd000)[(addr >> 12) & 0x3ff];

    if (!pte->p) return;

    kMemSetB(pte, 0, sizeof(PageEntry_t));
    // TODO: Invalidate page here
}


//
// -- Create the process for the PMM
//    ------------------------------
void PmmStart(Module_t *pmmMod)
{
    int i;
    archsize_t v, p;

    if (!pmmMod) {
        kprintf ("PmmStart(): module is NULL\n");
        return;
    }

    ListInit(&pmmProcess.messages.list);
    ListInit(&pmmProcess.lockList.list);

    kprintf("PmmStart(): installing module for %s, located at %p\n", pmmMod->modIdent, pmmMod->modStart);

    //
    // -- The first order of business is to create the Page Directory.  This will be done by mapping the Page
    //    Directory and Page Table frames to 2 temporary pages and then building them in those pages.  The
    //    frames themselves also need to be identified.  The Page Directory is allocated by the loader and
    //    the Page Table is built into the binary.
    //    ---------------------------------------------------------------------------------------------------
    frame_t pageDirFrame = pmmMod->cr3 >> 12;
    frame_t pageTblFrame = (pmmMod->modStart + pmmMod->modHdrSize + 4096) >> 12;  // This is the ELF header and stack
    archsize_t stack = (pmmMod->modStart + pmmMod->modHdrSize) >> 12;              // Only skip the ELF header

    kprintf("PmmStart(): calculated pageTblFrame to be %p; tos to be %p\n", pageTblFrame, stack);

    PageEntry_t *pd = (PageEntry_t *)PROCESS_PAGE_DIR;
    PageEntry_t *entry;

    // -- Now, temporarily map the Page Directory into the kernel page table
    kprintf("PmmStart(): Setting up the tables to be managed\n");
    KernelMap(PROCESS_PAGE_DIR, pageDirFrame);
    KernelMap(PROCESS_PAGE_TABLE, pageTblFrame);

    // -- initialize the 2 tables to 0
    kprintf("PmmStart(): clearing the mmu tables\n");
    kMemSetB((void *)PROCESS_PAGE_DIR, 0, 4096);
    kMemSetB((void *)PROCESS_PAGE_TABLE, 0, 4096);

    // -- map the page table to the page directory TODO: fix this hard-code crap
    entry = &pd[512];
    entry->frame = pageTblFrame;
    entry->p = 1;
    entry->rw = 1;
    entry->us = 1;

    // -- recursively map the page directory
    entry = &pd[1023];
    entry->frame = pageDirFrame;
    entry->p = 1;
    entry->rw = 1;
    entry->us = 1;

    // -- now map the kernel into this page directory. this is actually quite easy by mapping the kernel page tables
    //    into the process's page directory.  We need to copy PDEs 768 to 1021 (mapped or not) from the kernel PD
    //    to the process PD.
    //    ----------------------------------------------------------------------------------------------------------
    kprintf("PmmStart(): Copying the kernel Page Tables\n");
    for (i = 768; i <= 1021; i ++) {
        pd[i] = ((PageEntry_t *)RECURSIVE_PD_VADDR)[i];
    }

    // -- we also need to map the PMM into the new address space
    //    ------------------------------------------------------
    kprintf("PmmStart(): Mapping the pmm itself\n");
    for (v = 0x80000000, p = pmmMod->modStart + pmmMod->modHdrSize; p <= pmmMod->modEnd; v += 4096, p += 4096) {
        PageEntry_t *e = &((PageEntry_t *)PROCESS_PAGE_TABLE)[(v >> 12) & 0x3ff];
        e->frame = p >> 12;
        e->p = 1;
        e->rw = 1;
        e->us = 1;
    }

    // -- we can unmap these pages now
    KernelUnmap(PROCESS_PAGE_DIR);
    KernelUnmap(PROCESS_PAGE_TABLE);

    // -- Now for the magic of the initial stack.
    KernelMap(PROCESS_STACK_BUILD, stack);

    // -- clear the stack just for good measure
    kMemSetB((void *)PROCESS_STACK_BUILD, 0, 4096);

    kprintf("PmmStart(): Build the stack\n");
    pmmProcess.ss = 0x23;

    archsize_t *msp = (archsize_t *)(PROCESS_STACK_BUILD + 4096);

    // -- note there are no parameters; TODO: create a SYSCALL to self-terminate
    *(-- msp) = (archsize_t)0xff000000;                          // Force a page fault in the forbidden range
    *(-- msp) = (archsize_t)pmmMod->entry;                       // our entry point -- simulated context switch
    *(-- msp) = (archsize_t)0;                                    // ebp
    *(-- msp) = (archsize_t)INIT_FLAGS;                           // flags
    *(-- msp) = (archsize_t)0;                                    // eax
    *(-- msp) = (archsize_t)0;                                    // ebx
    *(-- msp) = (archsize_t)0;                                    // ecx
    *(-- msp) = (archsize_t)0;                                    // edx
    *(-- msp) = (archsize_t)0;                                    // esi
    *(-- msp) = (archsize_t)0;                                    // edi
    *(-- msp) = (archsize_t)0;                                    // cr0
    *(-- msp) = pmmMod->cr3;                                    // cr3

    *(-- msp) = (archsize_t)0x23;                                 // ds
    *(-- msp) = (archsize_t)0x23;                                 // es
    *(-- msp) = (archsize_t)0x23;                                 // fs
    *(-- msp) = (archsize_t)0x23;                                 // gs

    pmmProcess.stackPointer = ((archsize_t)msp - PROCESS_STACK_BUILD) + 0x80000000;
    pmmProcess.status = PROC_RUN;
    pmmProcess.pageTables = pmmMod->cr3;

    KernelUnmap(PROCESS_STACK_BUILD);
    kprintf("PmmStart(): All done\n");

    // -- These are legal without a lock because interrupts are still disabled
    procs[PID_PMM] = &pmmProcess;
    ListAddTail(&procOsPtyList, &pmmProcess.stsQueue);
}