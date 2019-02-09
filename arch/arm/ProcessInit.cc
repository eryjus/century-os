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

    kprintf("Initializing the idle process structure\n");
    kprintf("Initializing the list at %p\n", &idleProcess.messages.list);
    ListInit(&idleProcess.messages.list);
    kprintf("Initializing the list at %p\n", &idleProcess.lockList.list);
    ListInit(&idleProcess.lockList.list);
    kprintf("Done\n");

    idleProcess.ss = 0x00;

    archsize_t *msp = (archsize_t *)(idleStack + 512);

    kprintf("Creating the \"magic stack\" for the idle process\n");
    // -- note there are no parameters; TODO: create a SYSCALL to self-terminate
    *(-- msp) = (archsize_t)0xff000000;                          // Force a page fault in the forbidden range
    *(-- msp) = (archsize_t)idleMain;                            // our entry point -- `lr` register
    *(-- msp) = (archsize_t)0;                                    // r13 or stack (thrown away)
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
    *(-- msp) = (archsize_t)0;                                    // cpsr

    idleProcess.stackPointer = (archsize_t)msp;
    idleProcess.status = PROC_RUN;
    idleProcess.pageTables = GetTTBR0();

    // -- These are legal without a lock because interrupts are still disabled
    procs[PID_IDLE] = &idleProcess;
    ListAddTail(&procIdlePtyList, &idleProcess.stsQueue);
    kprintf("Idle Process initialized\n");


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
    kprintf("Butler Process initialized\n");
}
