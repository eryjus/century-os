//===================================================================================================================
// kernel/src/x86-common/IsrInt13.cc -- SIMD Floating Point Exception
//
// The most basic SIMD Floating Point Exception handler
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
void IsrInt13(isrRegs_t *regs)
{
    kprintf("\nSIMD Floating Point Exception\n");
    IsrDumpState(regs);
}