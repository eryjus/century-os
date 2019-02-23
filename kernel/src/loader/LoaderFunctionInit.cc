//===================================================================================================================
//
//  LoaderFunctionsInit.cc -- Set up the physical address for any functions called by the loader.
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Feb-12  Initial   0.3.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "serial.h"
#include "loader.h"


//
// -- Perform this function initialization
//    ------------------------------------
void __ldrtext LoaderFunctionInit(void)
{
    LoaderSerialPutChar = (SerialPutChar_t)PHYS_OF(SerialPutChar);
    lMemSetB = (kMemSetB_t)PHYS_OF(kMemSetB);
}

