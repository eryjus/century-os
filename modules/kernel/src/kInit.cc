//===================================================================================================================
//
//  kInit.cc -- Initialize the kernel structures
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  Initialize the kernel structures, preparing to formally start the OS.
//
//  Initialization is going to be broken into several phases.  At a high level, the phases are:
//  1) Required initialization to put the processor into a known and common state
//  2) OS Structure Initialization
//  3) Service Interrupts and hardware discovery
//  4) Full interrupts enabled and user space initialization
//  5) Become the butler process
//
//  The above is a starting point and will be expanded as we add support
//
//  -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2012-Sep-15  Initial                 Initial version -- leveraged from Century: kmain
//  2013-Sep-01    #82                   Add a mutex for screen operations (2018-05-25: temporarily removed)
//  2013-Sep-03    #73                   Encapsulate Process Structure
//  2013-Sep-13   #101                   Resolve issues splint exposes
//  2018-May-25            0.1.0   ADCL  Copy this file from century32 to century-os
//  2018-Jul-01  Initial   0.1.0   ADCL  Refactor this function to be strictly the kernel (not the loader)
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "types.h"
#include "lists.h"
#include "cpu.h"
#include "fb.h"
#include "hw-disc.h"
#include "interrupt.h"
#include "hardware.h"
#include "printf.h"
#include "heap.h"
#include "process.h"
#include "timer.h"
#include "pmm.h"
#include "msgq.h"
#include "serial.h"
#include "debugger.h"
#include "msgq.h"
#include "butler.h"


//
// -- the flag which will indicate it is time to clean up
//    ---------------------------------------------------
extern volatile bool startCleanup;


//
// -- A couple of local prototypes
//    ----------------------------
extern "C" void kInit(void);
void PmmStart(Module_t *);


EXPORT KERNEL_DATA
Process_t *A;

EXPORT KERNEL_DATA
Process_t *B;

EXPORT KERNEL_DATA
Process_t *debugger;

int semid;

EXPORT KERNEL_DATA
MessageQueue_t *q1;

EXPORT KERNEL_DATA
MessageQueue_t *q2;


EXTERN_C EXPORT KERNEL
void StartA(void)
{
    while (true) {
        long t = 0;
        MessageQueueSend(q1, t, 0, 0);
        MessageQueueReceive(q2, &t, 0, 0, true);
        ProcessSleep(1);
    }
}

EXTERN_C EXPORT KERNEL
void StartB(void)
{
    while (true) {
        long t = 0;
        MessageQueueReceive(q1, &t, 0, 0, true);
        ProcessSleep(1);
        MessageQueueSend(q2, t, 0, 0);
    }
}


EXPORT KERNEL_DATA
Spinlock_t testLock = {0};

EXPORT KERNEL_DATA
volatile int testval = 0;

EXPORT KERNEL_DATA
AtomicInt_t atomVal = {0};

EXPORT KERNEL_DATA
AtomicInt_t done = {0};

EXPORT KERNEL_DATA
AtomicInt_t instance = {0};


EXTERN_C EXPORT KERNEL
void AtomicsTest(void)
{
    int odd = AtomicInc(&instance) % 2;

    while (cpus.cpusRunning != 4) {}

    for (int i = 0; i < 1000000; i ++) {
        if (odd) {
            AtomicInc(&atomVal);
        } else {
            AtomicDec(&atomVal);
        }
    }

    for (int i = 0; i < 1000000; i ++) {
        archsize_t flags =  SPINLOCK_BLOCK_NO_INT(testLock) {
            if (odd) {
                testval ++;
            } else {
                testval --;
            }
        } SPINLOCK_RLS_RESTORE_INT(testLock, flags);
    }

    AtomicInc(&done);
}


//
// -- This is the main entry point for the kernel, starting with initialization
//    -------------------------------------------------------------------------
EXTERN_C EXPORT KERNEL NORETURN
void kInit(void)
{
    //
    // -- Phase 1: Required by the processor to setup the proper state
    //             Greet the user from the kernel.
    //    ------------------------------------------------------------
    kprintf("\x1b[0;1;31;40mWelcome to CenturyOS\x1b[0m -- a hobby operating system\n");
    kprintf("    (initializing...)\n");

    SetBgColor(FrameBufferParseRGB("#404040"));
    SetFgColor(0xffff);
    FrameBufferClear();
    FrameBufferPutS("Welcome to CenturyOS -- a hobby operating system\n");
    FrameBufferPutS("    (initializing...)\n");
    FrameBufferPutS("The RSDP is located at "); FrameBufferPutHex(GetRsdp()); FrameBufferDrawChar('\n');


    //
    // -- Phase 2: Required OS Structure Initialization
    //    ---------------------------------------------
    ProcessInit();
    TimerInit(timerControl, 1000);

    kprintf("Reporting interesting Process_t offsets:\n");
    kprintf("  Top of Stack: %x\n", offsetof(Process_t, topOfStack));
    kprintf("  Virtual Address Space: %x\n", offsetof(Process_t, virtAddrSpace));
    kprintf("  Process Status: %x\n", offsetof(Process_t, status));
    kprintf("  Process Priority: %x\n", offsetof(Process_t, priority));
    kprintf("  Process Quantum Left: %x\n", offsetof(Process_t, quantumLeft));
    kprintf("Reporting interesting Scheduler_t offsets:\n");
    kprintf("  Next PID to assign: %x\n", offsetof(Scheduler_t, nextPID));
    kprintf("  Next wake timer tick: %x\n", offsetof(Scheduler_t, nextWake));
    kprintf("  Process Change Pending flag: %x\n", offsetof(Scheduler_t, processChangePending));
    kprintf("  Process Lock Count: %x\n", offsetof(Scheduler_t, schedulerLockCount));
    kprintf("  Postpone Count: %x\n", offsetof(Scheduler_t, postponeCount));
    kprintf("Reporting interesting perCPU offsets:\n");
    kprintf("  Current Process: %x\n", offsetof(ArchCpu_t, process));

    MessageQueueInit();

    kprintf("Enabling interrupts now\n");
    EnableInterrupts();
//BOCHS_TOGGLE_INSTR;
    CoresStart();
    picControl->ipiReady = true;


    //
    // -- Phase 3: Service Interrupts only enabled, not ready for all interrupts
    //             Includes hardware initialization
    //    ----------------------------------------------------------------------


    //
    // -- Phase 4: Full interrupts enabled, user space prepared
    //             Includes loading and starting device drivers
    //      -----------------------------------------------------
    q1 = MessageQueueCreate();
    q2 = MessageQueueCreate();
    kprintf("Starting drivers and other kernel processes\n");
    ProcessCreate("Process A", StartA);
    ProcessCreate("Process B", StartB);
    debugger = ProcessCreate("Kernel Debugger", DebugStart);



    //
    // -- Phase 5: Assume the butler process role
    //      ---------------------------------------
    startCleanup = true;
    Butler();
}

