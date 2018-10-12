//===================================================================================================================
// kernel/src/x86-common/IsrInt1f.cc -- This is Intel Reserved Interrupt
//
// An unused Interrupt
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Oct-11  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "printf.h"
#include "idt.h"


//
// -- An Intel Interrupt
//    --------------
void IsrInt1f(isrRegs_t *regs)
{
    kprintf("\nRESERVED INT1F\n");
    IsrDumpState(regs);
}
