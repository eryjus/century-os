//===================================================================================================================
// kernel/src/IsrRegister.cc -- Register an ISR Handler to the table
//
// Also checks for an already registered handler and will not replace it if one already exists.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date     Tracker  Version  Pgmr  Description
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2012-07-21                          Initial version
//  2012-09-16                          Leveraged from Century
//  2018-07-06  Initial   0.1.0   ADCL  Copied this file from century32 to century-os
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "printf.h"
#include "idt.h"


//
// -- Register an ISR handler to the ISR Handler table
//    ------------------------------------------------
void IsrRegister(uint8_t interrupt, isrFunc_t func)
{
	regval_t flags = DisableInterrupts();

    if (isrHandlers[interrupt] != NULL_ISR) {
        kprintf("When registering interrupt %d, a handler is already registered; user IsrUnregister()\n", interrupt);
    } else {
	    isrHandlers[interrupt] = func;
    }

	RestoreInterrupts(flags);
}