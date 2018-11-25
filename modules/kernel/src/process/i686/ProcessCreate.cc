//===================================================================================================================
//
//  ProcessCreate.h -- Create a new process
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  Create a new process and add it into the ready queue.  The key to this function is to properly create the stack
//  such that it appears like a function call to the process entry point.  The stack needs to be prepared to mimick
//  a task switch as prepared by `ProcessSwitch()`.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2012-Sep-23                          Initial Version
//  2013-Aug-20    #46                   Need to initialize the new TTY session (removed)
//  2013-Aug-22    #60                   Create the ability to find a process quickly by PID
//  2013-Aug-25    #67                   Add a list of locks held by a process
//  2013-Aug-27    #36                   Add messages to the process structure
//  2013-Sep-01    #83                   Add a spinlock to the PID list array
//  2013-Sep-03    #73                   Encapsulate Process Structure
//  2013-Sep-12   #101                   Resolve issues splint exposes
//  2018-Oct-28  Initial   0.1.0   ADCL  Copied from Century32
//
//===================================================================================================================


#include "types.h"
#include "spinlock.h"
#include "heap.h"
#include "cpu.h"
#include "process.h"


//
// -- These are the initial flags for the running process (interrupts enabled)
//    ------------------------------------------------------------------------
#define INIT_STACK		4096


//
// -- Create a new process and build out its stack
//    --------------------------------------------
PID_t ProcessCreate(const char *cmd, ptrsize_t entryPoint, uint32_t nParms, ...)
{
    PID_t pid;
    char *stack = (char *)HeapAlloc(INIT_STACK + sizeof(Process_t), true);
    Process_t *proc = (Process_t *)(stack + INIT_STACK);
    const char *StackSig = "This is the process magic stack\0";
  	regval_t *msp;
	regval_t *arg;
    int i;

    if (!stack) {
        Halt();
    }

    SPIN_BLOCK(pidTableLock) {
        pid = ProcessNewPID();
        procs[pid] = proc;
        SpinlockUnlock(&pidTableLock);
    }

    kMemSetB(procs[pid], 0, sizeof(Process_t));

    proc->ssAddr = (ptrsize_t)stack;
	for (i = 0; i < MAX_CMD_LEN - 1 && cmd[i]; i ++) proc->command[i] = cmd[i];
	proc->command[i] = 0;
	proc->status = PROC_INIT;
	proc->priority = PTY_NORM;
	proc->quantumLeft = 0;
	proc->totalQuantum = 0;
	proc->ss = 0x10;
	proc->ssLength = INIT_STACK;

	stack = (char *)proc->ssAddr;
	for (i = 0; i < 32; i ++) {
		stack[INIT_STACK + i - 32] = stack[i] = StackSig[i];
	}

	msp = (regval_t *)(proc->ssAddr + INIT_STACK - 32);
	arg = (&nParms) + nParms + 1;

	for ( ; nParms > 0; nParms --) *(-- msp) = *(-- arg);	    // parms from right to left
	*(-- msp) = (ptrsize_t)ProcessEnd;	                        // the function to self-destroy -- simulated call
	*(-- msp) = (regval_t)INIT_FLAGS;			                // flags
	*(-- msp) = (regval_t)0x0f;									// User Code Segment
	*(-- msp) = (ptrsize_t)entryPoint;			                // our entry point -- simulated context switch
	*(-- msp) = (regval_t)0;					                // error code
	*(-- msp) = (regval_t)0;					                // unused ack irq
	*(-- msp) = (regval_t)0;									// interrupt number
	*(-- msp) = (regval_t)0;					                // eax
	*(-- msp) = (regval_t)0;					                // ecx
	*(-- msp) = (regval_t)0;					                // edx
	*(-- msp) = (regval_t)0;					                // ebx
	*(-- msp) = (regval_t)0;					                // esp
	*(-- msp) = (regval_t)0;					                // ebp
	*(-- msp) = (regval_t)0;					                // esi
	*(-- msp) = (regval_t)0;					                // edi
	*(-- msp) = (regval_t)0;					                // cr0
	*(-- msp) = (regval_t)0;									// cr2
	*(-- msp) = GetCr3();	                                    // cr3

	*(-- msp) = (regval_t)0x10;				                    // ds
	*(-- msp) = (regval_t)0x10;				                    // es
	*(-- msp) = (regval_t)0x10;				                    // fs
	*(-- msp) = (regval_t)0x10;				                    // gs
	*(-- msp) = (regval_t)0x10;									// ss

	proc->esp = (regval_t)msp;
	proc->status = PROC_RUN;
    ProcessReady(pid);

	return pid;
}
