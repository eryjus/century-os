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
#include "printf.h"
#include "heap.h"
#include "process.h"
#include "timer.h"
#include "ipc.h"
#include "pmm.h"
#include "serial.h"


//
// -- A couple of local prototypes
//    ----------------------------
extern "C" void kInit(void);
void PmmStart(Module_t *);


Process_t *A;
Process_t *B;
Process_t *C;
Process_t *D;
Process_t *E;
Process_t *F;
Process_t *G;



void StartB(void)
{
    while (1) {
        kprintf("B");
        ProcessSleep(1);        // sleep 1 second
    }
}

void StartC(void)
{
    while (1) {
        kprintf("C");
//        ProcessSleep(3);
        ProcessBlock(PROC_MSGW);
    }
}

void StartD(void)
{
    while (1) {
        kprintf("D");
        ProcessMilliSleep(250);     // sleep 250 ms
    }
}

void StartE(void)
{
    while (1) {
        kprintf("E");
        ProcessMicroSleep(1);       // 1 micro-second -- probably will not actually stop
    }
}

void StartF(void)
{
    while (1) {
        kprintf("F");
        ProcessMilliSleep(5);
    }
}

void StartG(void)
{
    while (1) {
        kprintf("G");
        ProcessMilliSleep(3);
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

    //
    // -- Phase 2: Required OS Structure Initialization
    //    ---------------------------------------------
    ProcessInit();
    TimerInit(&timerControl, 1000);
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


    A = currentProcess;
    B = ProcessCreate(StartB);
    C = ProcessCreate(StartC);
    D = ProcessCreate(StartD);
    E = ProcessCreate(StartE);
    F = ProcessCreate(StartF);
    G = ProcessCreate(StartG);


    while (1) {
        kprintf("\n");
        kprintf("A (pid = %x) timer = %p : %p\n", A->pid, (uint32_t)(A->timeUsed >> 32), (uint32_t)A->timeUsed);
        kprintf("B (pid = %x) timer = %p : %p\n", B->pid, (uint32_t)(B->timeUsed >> 32), (uint32_t)B->timeUsed);
        kprintf("C (pid = %x) timer = %p : %p\n", C->pid, (uint32_t)(C->timeUsed >> 32), (uint32_t)C->timeUsed);
        kprintf("D (pid = %x) timer = %p : %p\n", D->pid, (uint32_t)(D->timeUsed >> 32), (uint32_t)D->timeUsed);
        kprintf("E (pid = %x) timer = %p : %p\n", E->pid, (uint32_t)(E->timeUsed >> 32), (uint32_t)E->timeUsed);
        kprintf("F (pid = %x) timer = %p : %p\n", F->pid, (uint32_t)(F->timeUsed >> 32), (uint32_t)F->timeUsed);
        kprintf("G (pid = %x) timer = %p : %p\n", G->pid, (uint32_t)(G->timeUsed >> 32), (uint32_t)G->timeUsed);

        kprintf("Low 32-bit ticks is %x\n", (uint32_t)TimerCurrentCount(&timerControl));
        kprintf("Next wake time is %x\n", (uint32_t)nextWake);

        ListRemoveInit(&C->stsQueue);
        ProcessUnblock(C);

        kprintf("A");
        ProcessSleep(5);
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


//
// -- This is ths Idle process
//    ------------------------
void idleMain(void)
{
    while (1) {
        HaltCpu();
    }
}

