//===================================================================================================================
//
//  IsrRegister.cc -- Register an ISR Handler to the table
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  Also checks for an already registered handler and will not replace it if one already exists.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2012-Jul-21                          Initial version
//  2012-Sep-16                          Leveraged from Century
//  2018-Jul-06  Initial   0.1.0   ADCL  Copied this file from century32 to century-os
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "printf.h"
#include "interrupt.h"


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
