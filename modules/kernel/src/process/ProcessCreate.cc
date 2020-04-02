//===================================================================================================================
//
// ProcessCreate.cc -- Create a new process, setting everything up to be able to schedule it
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
#include "heap.h"
#include "mmu.h"
#include "printf.h"
#include "process.h"


//
// -- Create a new process and get it ready to be scheduled
//    -----------------------------------------------------
EXPORT KERNEL
Process_t *ProcessCreate(void (*startingAddr)(void))
{
    extern archsize_t mmuLvl1Table;

    Process_t *rv = NEW(Process_t);
    if (!assert_msg(rv != NULL, "Out of memory allocating a new Process_t")) {
        CpuPanicPushRegs("Out of memory allocating a new Process_t");
    }

    rv->pid = scheduler.nextPID ++;
    rv->command = NULL;
    rv->policy = POLICY_0;
    rv->priority = PTY_OS;
    rv->status = PROC_INIT;
    AtomicSet(&rv->quantumLeft, PTY_OS);
    rv->timeUsed = 0;
    ListInit(&rv->stsQueue);


    //
    // -- Construct the stack for the architecture
    //    ----------------------------------------
    rv->ssAddr = ProcessNewStack(rv, startingAddr);


    //
    // -- Construct the new addres space for the process
    //    ----------------------------------------------
    rv->virtAddrSpace = mmuLvl1Table;

#if DEBUG_ENABLED(ProcessCreate)
    kprintf("ProcessCreate() created a new process at %p\n", rv);
#endif


    //
    // -- Put this process on the queue to execute
    //    ----------------------------------------
    ProcessLockAndPostpone();
    rv->status = PROC_READY;
    ProcessDoReady(rv);
    ProcessUnlockAndSchedule();

    return rv;
}

