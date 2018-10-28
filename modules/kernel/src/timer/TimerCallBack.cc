//===================================================================================================================
//
// TimerCallBack.cc -- Handle the interrupt when the timer fires.
//
// This IRQ is used as the scheduler for preemptive multitasking.  Since this is used as IRQ0, there is an EOI that
// needs to be issued to the interrupt controller.  This can get messy, so I want to walk it through a bit here.
//
// First, recall that there is a context for the running process.  This process was interrupted by the CPU and
// has all of that interrupt material on the user stack.  The interrupt also disabled interrupts after storing the
// flags register on the stack.  At the same time the PIC/APIC will be waiting for confirmation that the IRQ has
// been serviced.
//
// Then, in this context, the EOI is issues, allowing additional IRQs to be processed.  But interrupts are still
// disabled, so we are still in contol of the CPU.  `ProcessReschedule()` is called to determine the next process to
// give to the CPU.  Finally, `ProcessSwitch()` is called to actually perform the context switch to a new process.
// This context switch will also save all the necessary registers and flags on the stack (duplicating them if they
// are already there).
//
// At this point, interrupts are still disabled.  To complicate matters, the new process may or may not have been
// interrupted, but rather voluntarily gave up the CPU because it was blocked.  The point is the path out of any
// particular context switch (an unblocked process) will not be the same as the path in (an interrupt).  But,
// that does not cause a problem since the stack was changed as well and the `ret` opcodes will hand control back
// to the proper functions.  Eventually, the flags will be popped off the stack (at least once) and will then
// restore the interrupts for that context.  This will then allow additional IRQs to interrupt the CPU.
//
// It took me a while to get my head around all this, as I naturally want everything to be symmetric and this is
// not.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2012-Sep-16                          Leveraged from Royalty
//  2012-Sep-23                          set new SpuriousIRQ handler
//  2013-Sep-03    #73                   Encapsulate Process Structure
//  2013-Sep-12   #101                   Resolve issues splint exposes
//  2018-Oct-28  Initial   0.1.0   ADCL  Copied this function from Century32 to Centrury-OS
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "process.h"
#include "idt.h"
#include "timer.h"


//
// -- This is the current timer controls functions required to issue the proper EOI
//    -----------------------------------------------------------------------------
TimerFunctions_t *timerControl;


//
// -- Initialize the PIT frequency
//    ----------------------------
void TimerCallBack(UNUSED(isrRegs_t *reg))
{
    kprintf(".");
	if (!ProcessEnabled) {
        timerControl->eoi(0);
        return;
    }

	procs[currentPID]->totalQuantum ++;
	if (-- procs[currentPID]->quantumLeft <= 0) {
        timerControl->eoi(0);
        ProcessReschedule();
    }
}