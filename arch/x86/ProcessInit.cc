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
//  2018-Oct-28  Initial   0.1.0   ADCL  Initial version
//  2019-Feb-09  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "cpu.h"
#include "process.h"


//
// -- Initialize the process structures
//    ---------------------------------
void ProcessInit(void)
{
    //
    // -- Establish the idle process
    //    --------------------------
    extern uint32_t idleStack[512];
    extern Process_t idleProcess;
    extern void idleMain(void);

    ListInit(&idleProcess.messages.list);
    ListInit(&idleProcess.lockList.list);

    idleProcess.ss = 0x10;

    archsize_t *msp = (archsize_t *)(idleStack + 512);

    // -- note there are no parameters; TODO: create a SYSCALL to self-terminate
    *(-- msp) = (archsize_t)0xff000000;                          // Force a page fault in the forbidden range
    *(-- msp) = (archsize_t)idleMain;                            // our entry point -- simulated context switch
    *(-- msp) = (archsize_t)0;                                    // ebp
    *(-- msp) = (archsize_t)INIT_FLAGS;                           // flags
    *(-- msp) = (archsize_t)0;                                    // eax
    *(-- msp) = (archsize_t)0;                                    // ebx
    *(-- msp) = (archsize_t)0;                                    // ecx
    *(-- msp) = (archsize_t)0;                                    // edx
    *(-- msp) = (archsize_t)0;                                    // esi
    *(-- msp) = (archsize_t)0;                                    // edi
    *(-- msp) = (archsize_t)0;                                    // cr0
    *(-- msp) = GetCr3();                                       // cr3

    *(-- msp) = (archsize_t)0x10;                                 // ds
    *(-- msp) = (archsize_t)0x10;                                 // es
    *(-- msp) = (archsize_t)0x10;                                 // fs
    *(-- msp) = (archsize_t)0x10;                                 // gs

    idleProcess.stackPointer = (archsize_t)msp;
    idleProcess.status = PROC_RUN;
    idleProcess.pageTables = GetCr3();

    // -- These are legal without a lock because interrupts are still disabled
    procs[PID_IDLE] = &idleProcess;
    ListAddTail(&procIdlePtyList, &idleProcess.stsQueue);


    //
    // -- Establish the Butler Process
    //    ----------------------------
    ListInit(&butler.stsQueue);
    butler.lockList.lock = {0};
    butler.messages.lock = {0};
    ListInit(&butler.lockList.list);
    ListInit(&butler.messages.list);

    // -- no lock needed during initialization
    ListAddTail(&procOsPtyList, &butler.stsQueue);

    procs[PID_BUTLER] = &butler;
    currentPID = PID_BUTLER;
}
