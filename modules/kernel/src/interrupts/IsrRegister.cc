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
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "printf.h"
#include "interrupt.h"


//
// -- Register an ISR handler to the ISR Handler table
//    ------------------------------------------------
isrFunc_t IsrRegister(uint8_t interrupt, isrFunc_t func)
{
    kprintf("Request to map vector %x to function at %p\n", interrupt, func);
    archsize_t flags = DisableInterrupts();
    isrFunc_t rv = isrHandlers[interrupt];

    isrHandlers[interrupt] = func;

    RestoreInterrupts(flags);
    return rv;
}
