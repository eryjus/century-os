//===================================================================================================================
//
//  ProcessInit.h -- Initialize the process structures
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-21  Initial   0.2.0   ADCL  Initial version
//  2018-Dec-02  Initial   0.2.0   ADCL  Provide the details of this function
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//  2019-Mar-15  Initial   0.3.2   ADCL  Update for new Process_t
//
//===================================================================================================================


#include "cpu.h"
#include "heap.h"
#include "process.h"


//
// -- we need the `mmuLvl1Table` variable from the loader, but do not want to include all of
//    what we did in the loader
//    --------------------------------------------------------------------------------------
extern archsize_t mmuLvl1Table;


//
// -- Initialize the process structures
//    ---------------------------------
void __krntext ProcessInit(void)
{
    kprintf("ProcessInit(): mmuLvl1Table = %p\n", mmuLvl1Table);
    currentProcess = NEW(Process_t);

    currentProcess->topOfStack = 0;
    currentProcess->virtAddrSpace = mmuLvl1Table;
    currentProcess->pid = nextPID ++;          // -- this is the butler process ID
    currentProcess->ssAddr = STACK_LOCATION;
    currentProcess->command = NULL;
    currentProcess->policy = POLICY_0;
    currentProcess->priority = PTY_OS;
    currentProcess->status = PROC_RUN;
    currentProcess->quantumLeft = PTY_OS;
    ListInit(&currentProcess->stsQueue);
}
