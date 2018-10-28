//===================================================================================================================
// kernel/src/x86-common/IsrInt0c.cc -- Stack Segment Fault
//
// The most basic Stack Segment Fault handler
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
void IsrInt0c(isrRegs_t *regs)
{
    kprintf("\nStack Segment Fault\n");
    IsrDumpState(regs);
}