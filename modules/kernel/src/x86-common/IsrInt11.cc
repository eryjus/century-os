//===================================================================================================================
// kernel/src/x86-common/IsrInt11.cc -- Alignment Check
//
// The most basic Alignment Check handler
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
void IsrInt11(isrRegs_t *regs)
{
    kprintf("\nAlignment Check\n");
    IsrDumpState(regs);
}