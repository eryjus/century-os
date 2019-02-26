//===================================================================================================================
//
//  IsrHandler.cc -- The common ISR handler routine
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  All ISRs are handled by a common service program.  This is it.  But it is currently a stub.
//
//  IRQ 0-63 are for the normal IRQs.  Then, there are 8 additional IRQs (64-71) for some additional interrupts.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Dec-06  Initial   0.2.0   ADCL  Initial version
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "types.h"
#include "hardware.h"
#include "printf.h"
#include "timer.h"
#include "interrupt.h"


//
// -- The ISR Handler Table
//    ---------------------
isrFunc_t isrHandlers[256] = {NULL_ISR};


//
// -- This is the common ISR Handler entry routine
//    --------------------------------------------
void IsrHandler(isrRegs_t *regs)
{
    int intno = 0;
    int i;
    uint32_t pending = 0;

    // -- Here we need to determine the intno for the ISR
    pending = PicGetIrq(&picControl);

    for (i = 11; i >= 0; i --) {
        if (pending & (1<<i)) {
            intno = i;
            goto process;
        }
    }

    kprintf("PANIC: Unable to determine interrupt number: %p\n", pending);
    Halt();
    return;

process:
    if (isrHandlers[intno] != NULL) {
        isrFunc_t handler = isrHandlers[intno];
        handler(regs);
    }
}
