//===================================================================================================================
//
//  kInit.cc -- Initialize the kernel structures
//
//        Copyright (c)  2017-2018 -- Adam Clark
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
//  2018-May-25             0.1.0  ADCL  Copy this file from century32 to century-os
//  2018-Jul-01  Initial    0.1.0  ADCL  Refactor this function to be strictly the kernel (not the loader)
//
//===================================================================================================================


#include "types.h"
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
#include "tss.h"


//
// -- This is the common hardware discovery structure passed in from the loader
//    -------------------------------------------------------------------------
HardwareDiscovery_t *localHwDisc = (HardwareDiscovery_t *)0x00003000;


//
// -- A couple of local prototypes
//    ----------------------------
extern "C" void kInit(void);
void PmmStart(Module_t *);


//
// -- For debugging to the serial port, this is the hardware port number
//    ------------------------------------------------------------------
uint16_t serialPort = 0x3f8;


//
// -- This is the main entry point for the kernel, starting with initialization
//    -------------------------------------------------------------------------
void kInit(void)
{
	//
	// -- Phase 1: Required by the processor to setup the proper state
	//             Greet the user from the kernel.
	//    ------------------------------------------------------------
	IdtBuild();
	CpuTssInit();
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
	for (int i = 0; i < GetModCount(); i ++) {
		char *s = GetAvailModuleIdent(i);
		if (s[0] == 'p' && s[1] == 'm' && s[2] == 'm' && s[3] == 0) {
			PmmStart(&localHwDisc->mods[i]);
			break;
		}
	}
	TimerInit(250);
	EnableInterrupts();
	ProcessEnabled = true;
	HeapInit();

	// -- let the Pmm know we are putting it in-charge
	Message_t pmm;
	kMemSetB(&pmm, 0, sizeof(Message_t));

	pmm.msg = PMM_INIT;
	pmm.dataPayload = (void *)GetPmmBitmap();
	pmm.payloadSize = GetPmmFrameCount() << 12;			// convert this to bytes
	MessageSend(PID_PMM, &pmm);


	//
	// -- Phase 3: Service Interrupts only enabled, not ready for all interrupts
	//             Includes hardware discovery and initialization
	//    ----------------------------------------------------------------------

//	kprintf("\nEnabling Interrupts & Driver Initialization starting...\n");

//	EnableInterrupts();
//	schedulerEnabled = 1;
//	InitPS2();
//	InitKeyboard();

//	debuggerPID = CreateProcess("Kernel Debugger", (uint32)DbgProcess, 0);
//	kprintf("Debugger PID = %lu\n", debuggerPID);

//	wrk = globalProcess.next;
//	do {
//		kprintf("%ld %s\n", GetProcPID(GLOBAL2PROC(wrk)), GetProcCommand(GLOBAL2PROC(wrk)));

//		wrk = wrk->next;
//	} while (wrk != &globalProcess);
//	hang();

	//
	// -- Phase 4: Full interrupts enabled, user space prepared
	//             Includes loading and starting device drivers
	//	  -----------------------------------------------------

//	kprintf("\nStarting User Space\n");
//	readyForInterrupts = 1;

//	SetTTY(TTY_DEBUGGER);

//	if (tty15Mode != TTY15_NONE) SetTTY(TTYF);

	//
	// -- Phase 5: Become the Butler Process
	//    ----------------------------------

//	SetProcPriority(currentProcess, PTY_IDLE);
//	BREAKPOINT;


	EnterSystemMode();
	kprintf("Reached the end of initialization\n");

	//
	// -- Phase 5: Assume the butler process role
	//	  ---------------------------------------

	while (1) {
		HaltCpu();
	}
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
