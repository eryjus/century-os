//===================================================================================================================
//
//  PmmStart.cc -- Start the Physical Memory Manager Process
//
//        Copyright (c)  2017-2018 -- Adam Clark
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
static inline void KernelMap(ptrsize_t addr, frame_t frame)
{
}


//
// -- This function is specific to this file and will only map data to PROCESS_PAGE_DIR or PROCESS_PAGE_TABLE.
//    Both of these addresses already have a Page Table from the loader for address 0xff401000 for the GDT/IDT.
//    ---------------------------------------------------------------------------------------------------------
static inline void KernelUnmap(ptrsize_t addr)
{
}


//
// -- Create the process for the PMM
//    ------------------------------
void PmmStart(Module_t *pmmMod)
{
    kprintf("Start the PMM process here in rpi2b\n");
}