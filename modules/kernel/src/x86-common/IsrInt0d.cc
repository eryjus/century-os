//===================================================================================================================
// kernel/src/x86-common/IsrInt0d.cc -- GPF handler
//
// The most basic GPF exception handler
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Oct-10  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "printf.h"
#include "idt.h"


//
// -- This is the ISR Handler routine
//    -------------------------------
void IsrInt0d(isrRegs_t *regs)
{
    kprintf("\nGeneral Protection Fault\n");
    IsrDumpState(regs);
}
