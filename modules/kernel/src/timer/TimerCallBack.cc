//===================================================================================================================
//
//  TimerCallBack.cc -- Handle the interrupt when the timer fires.
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  This IRQ is used as the scheduler for preemptive multitasking.  Since this is used as IRQ0, there is an EOI that
//  needs to be issued to the interrupt controller.  This can get messy, so I want to walk it through a bit here.
//
//  First, recall that there is a context for the running process.  This process was interrupted by the CPU and
//  has all of that interrupt material on the user stack.  The interrupt also disabled interrupts after storing the
//  flags register on the stack.  At the same time the PIC/APIC will be waiting for confirmation that the IRQ has
//  been serviced.
//
//  Then, in this context, the EOI is issues, allowing additional IRQs to be processed.  But interrupts are still
//  disabled, so we are still in contol of the CPU.  `ProcessReschedule()` is called to determine the next process to
//  give to the CPU.  Finally, `ProcessSwitch()` is called to actually perform the context switch to a new process.
//  This context switch will also save all the necessary registers and flags on the stack (duplicating them if they
//  are already there).
//
//  At this point, interrupts are still disabled.  To complicate matters, the new process may or may not have been
//  interrupted, but rather voluntarily gave up the CPU because it was blocked.  The point is the path out of any
//  particular context switch (an unblocked process) will not be the same as the path in (an interrupt).  But,
//  that does not cause a problem since the stack was changed as well and the `ret` opcodes will hand control back
//  to the proper functions.  Eventually, the flags will be popped off the stack (at least once) and will then
//  restore the interrupts for that context.  This will then allow additional IRQs to interrupt the CPU.
//
//  It took me a while to get my head around all this, as I naturally want everything to be symmetric and this is
//  not.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2012-Sep-16                          Leveraged from Royalty
//  2012-Sep-23                          set new SpuriousIRQ handler
//  2013-Sep-03    #73                   Encapsulate Process Structure
//  2013-Sep-12   #101                   Resolve issues splint exposes
//  2018-Oct-28  Initial   0.1.0   ADCL  Copied this function from Century32 to Century-OS
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "process.h"
#include "interrupt.h"
#include "serial.h"
#include "hardware.h"
#include "timer.h"


//
// -- Handle a timer IRQ
//    ------------------
EXPORT KERNEL
void TimerCallBack(UNUSED(isrRegs_t *reg))
{
    uint64_t now = 0;

    TimerEoi(timerControl);     // take care of this while interrupts are disabled!

    ProcessLockAndPostpone();

#if DEBUG_ENABLED(TimerCallBack)
    kprintf("handling timer\n");
#endif

    if (timerControl->TimerPlatformTick && thisCpu->cpuNum == 0) TimerPlatformTick(timerControl);

    //
    // -- here we look for any sleeping tasks to wake
    //    -------------------------------------------
    now = TimerCurrentCount(timerControl);
    if (now >= scheduler.nextWake && IsListEmpty(&scheduler.listSleeping) == false) {
        uint64_t newWake = (uint64_t)-1;


        //
        // -- loop through and find the processes to wake up
        //    ----------------------------------------------
        ListHead_t::List_t *list = scheduler.listSleeping.list.next;
        while (list != &scheduler.listSleeping.list) {
            ListHead_t::List_t *next = list->next;      // must be saved before it is changed below
            Process_t *wrk = FIND_PARENT(list, Process_t, stsQueue);
            if (now >= wrk->wakeAtMicros) {
                wrk->wakeAtMicros = 0;
                ListRemoveInit(&wrk->stsQueue);
                ProcessDoUnblock(wrk);
            } else if (wrk->wakeAtMicros < newWake) newWake = wrk->wakeAtMicros;

            list = next;
        }

        scheduler.nextWake = newWake;
    }


    //
    // -- adjust the quantum and see if it is time to change tasks
    //    --------------------------------------------------------
    if (currentThread != NULL) {
        if (AtomicDec(&currentThread->quantumLeft) <= 0) {
#if DEBUG_ENABLED(TimerCallBack)
            kprintf("Preempt\n");
#endif
            scheduler.processChangePending = true;
        }
    }

    ProcessUnlockAndSchedule();
}
