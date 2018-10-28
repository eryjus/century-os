//===================================================================================================================
// kernel/src/kInit.cc -- Initialize the kernel structures
//
// Initialize the kernel structures, preparing to formally start the OS.
//
// Initialization is going to be broken into several phases.  At a high level, the phases are:
// 1) Required initialization to put the processor into a known and common state
// 2) OS Structure Initialization
// 3) Service Interrupts and hardware discovery
// 4) Full interrupts enabled and user space initialization
// 5) Become the butler process
//
// The above is a starting point and will be expanded as we add support
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date     Tracker  Version  Pgmr  Description
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2012-09-15  Initial                 Initial version -- leveraged from Century: kmain
//  2013-09-01    #82                   Add a mutex for screen operations (2018-05-25: temporarily removed)
//  2013-09-03    #73                   Encapsulate Process Structure
//  2013-09-13   #101                   Resolve issues splint exposes
//  2018-05-25             0.1.0  ADCL  Copy this file from century32 to century-os
//  2018-07-01  Initial    0.1.0  ADCL  Refactor this function to be strictly the kernel (not the loader)
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "fb.h"
#include "hw-disc.h"
#include "idt.h"
#include "printf.h"
#include "heap.h"
#include "timer.h"


HardwareDiscovery_t *localHwDisc = (HardwareDiscovery_t *)0x00003000;


// extern uint32 schedulerEnabled;
// extern Mutex screenMutex;

extern "C" void kInit(void);

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
	TimerInit(250);
//	HeapInit();

//#ifndef USE_APIC
//	kprintf("Initializing PIT timer\n");
//	InitTimer(TIMER_FREQ);
//#else
//	kprintf("Initializing APIC timer\n");
//	InitAPIC(TIMER_FREQ);
//#endif

//	PciScanBus();

	//	InitProcess();
//	InitTTY();
//	TTY15Init();

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

	EnableInterrupts();

	kprintf("Reached the end of initialization\n");

	//
	// -- Phase 5: Assume the butler process role
	//	  ---------------------------------------

	while (1) {
		__asm("hlt");
	}
}
