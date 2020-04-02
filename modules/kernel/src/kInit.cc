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
#include "serial.h"


//
// -- A couple of local prototypes
//    ----------------------------
extern "C" void kInit(void);
void PmmStart(Module_t *);


Process_t *A;
Process_t *B;

int semid;


void StartA(void)
{
    while (true) {
        kprintf("A(%d);", thisCpu->cpuNum);
        ProcessMilliSleep(500);
    }
}

void StartB(void)
{
    while (true) {
        kprintf("B(%d);", thisCpu->cpuNum);
        ProcessMilliSleep(250);
    }
}


Spinlock_t testLock = {0};
volatile int testval = 0;
AtomicInt_t atomVal = {0};
AtomicInt_t done = {0};
AtomicInt_t instance = {0};

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
    kprintf("Welcome to CenturyOS -- a hobby operating system\n");
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
//    ProcessCheckQueue();
    TimerInit(timerControl, 1000);
    kprintf("Reporting interesting Process_t offsets:\n");
    kprintf("  Top of Stack: %x\n", offsetof(Process_t, topOfStack));
    kprintf("  Virtual Address Space: %x\n", offsetof(Process_t, virtAddrSpace));
    kprintf("  Process Status: %x\n", offsetof(Process_t, status));
    kprintf("Reporting interesting Scheduler_t offsets:\n");
    kprintf("  Next PID to assign: %x\n", offsetof(Scheduler_t, nextPID));
    kprintf("  Next wake timer tick: %x\n", offsetof(Scheduler_t, nextWake));
    kprintf("  Process Change Pending flag: %x\n", offsetof(Scheduler_t, processChangePending));
    kprintf("  Postpone Count: %x\n", offsetof(Scheduler_t, postponeCount));

    kprintf("Enabling interrupts now\n");
    EnableInterrupts();
    CoresStart();
    picControl->ipiReady = true;
    kprintf("Starting processes\n");
    BOCHS_TOGGLE_INSTR;
//    AtomicsTest();  while (AtomicRead(&done) != 4) {}
//    kprintf("The resulting int value is %d\n", testval);
//    kprintf("The resulting Atomic val is %d\n", AtomicRead(&atomVal));

    A = ProcessCreate(StartA);
    B = ProcessCreate(StartB);


    //
    // -- Phase 3: Service Interrupts only enabled, not ready for all interrupts
    //             Includes hardware discovery and initialization
    //    ----------------------------------------------------------------------

//    kprintf("\nEnabling Interrupts & Driver Initialization starting...\n");

//    EnableInterrupts();
//    schedulerEnabled = 1;
//    InitPS2();
//    InitKeyboard();

//    debuggerPID = CreateProcess("Kernel Debugger", (uint32)DbgProcess, 0);
//    kprintf("Debugger PID = %lu\n", debuggerPID);

//    wrk = globalProcess.next;
//    do {
//        kprintf("%ld %s\n", GetProcPID(GLOBAL2PROC(wrk)), GetProcCommand(GLOBAL2PROC(wrk)));

//        wrk = wrk->next;
//    } while (wrk != &globalProcess);
//    hang();

    //
    // -- Phase 4: Full interrupts enabled, user space prepared
    //             Includes loading and starting device drivers
    //      -----------------------------------------------------

//    kprintf("\nStarting User Space\n");
//    readyForInterrupts = 1;

//    SetTTY(TTY_DEBUGGER);

//    if (tty15Mode != TTY15_NONE) SetTTY(TTYF);

    //
    // -- Phase 5: Become the Butler Process
    //    ----------------------------------

//    SetProcPriority(currentProcess, PTY_IDLE);
//    BREAKPOINT;


#if 0
    semid = SemaphoreGet(IPC_PRIVATE, 1, 0);
    if (semid < 0) {
        kprintf("SemaphoreGet() returned -%x\n", -semid);
        HaltCpu();
    } else kprintf("SemaphoreGet() offered semid %x\n", semid);

    A = ProcessCreate(StartA);
    B = ProcessCreate(StartB);

    scheduler.currentProcess->priority = PTY_LOW;
#endif

    while (true) {
        kprintf(".(%d);", thisCpu->cpuNum);
        ProcessSleep(2);
    }


#if 0
    //
    // -- Phase 5: Assume the butler process role
    //      ---------------------------------------
    while (1) {
        //
        // -- Here we will look for pmm frames to clear
        //    -----------------------------------------
        PmmScrubBlock();

        //
        // -- there is nothing to do, so halt the cpu
        //    ---------------------------------------
        HaltCpu();
    }
#endif
}

