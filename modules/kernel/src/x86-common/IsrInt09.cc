//===================================================================================================================
// kernel/src/x86-common/IsrInt09.cc -- Coprocessor Segment Overrun
//
// The most basic Coprocessor Segment Overrid handler
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
void IsrInt09(isrRegs_t *regs)
{
    kprintf("\nCoprocessor Segment Overrun\n");
    IsrDumpState(regs);
}