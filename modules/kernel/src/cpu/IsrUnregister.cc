//===================================================================================================================
//
//  IsrUnregister.cc -- Unregister an ISR Handler
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Jul-06  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "printf.h"
#include "interrupt.h"


//
// -- Remove an ISR handler from the handlers table
//    ---------------------------------------------
void IsrUnregister(uint8_t interrupt)
{
	regval_t flags = DisableInterrupts();

    if (isrHandlers[interrupt] == NULL_ISR) {
        kprintf("When unregistering interrupt %d, no handler is registered\n", interrupt);
    } else {
	    isrHandlers[interrupt] = NULL_ISR;
    }

	RestoreInterrupts(flags);
}
