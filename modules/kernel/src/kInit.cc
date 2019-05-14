//===================================================================================================================
//
//  kInit.cc -- Initialize the kernel structures
//
//        Copyright (c)  2017-2019 -- Adam Clark
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
#include "atomic.h"
#include "semaphore.h"


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
    struct sembuf inc = {0, 2, IPC_NOWAIT};
    struct sembuf zero = {0, 0, 0};

    kprintf("Starting A\n");

    while (1) {
        while (SemaphoreOperations(semid, &inc, 1) < 0) { kprintf("a"); }
        kprintf("A");
        SemaphoreOperations(semid, &zero, 1);
    }
}

void StartB(void)
{
    struct sembuf dec = {0, -1, IPC_NOWAIT};

    kprintf("Starting B\n");

    while (1) {
        while (SemaphoreOperations(semid, &dec, 1) < 0) { kprintf("b"); }
        kprintf("B");
    }
}

//
// -- This is the main entry point for the kernel, starting with initialization
//    -------------------------------------------------------------------------
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

#ifdef RSDP_t
#   define RSDT        ((RSDP_t *)GetRsdp())->rsdtAddress
#else
#   define RSDT        0
#endif
    FrameBufferPutS("The RSDT is located at ");  FrameBufferPutHex(RSDT); FrameBufferDrawChar('\n');
#undef RSDT


    //
    // -- Phase 2: Required OS Structure Initialization
    //    ---------------------------------------------
    ProcessInit();
    SemaphoreInit();
    TimerInit(timerControl, 1000);
    kprintf("Enabling interrupts now\n");
    EnableInterrupts();


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

    while (1) {
        kprintf(".");
        HaltCpu();
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

