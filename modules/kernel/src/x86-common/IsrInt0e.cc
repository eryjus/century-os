//===================================================================================================================
// kernel/src/x86-common/IsrInt0e.cc -- Page Fault handler
//
// The most basic Page Fault exception handler
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
void IsrInt0e(isrRegs_t *regs)
{
    kprintf("\nPage Fault\n");
    IsrDumpState(regs);
}