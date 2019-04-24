//===================================================================================================================
//
//  EarlyInit.cc -- Handle the early initialization for the x86 architecture
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Feb-13  Initial   0.3.0   ADCL  Initial version
//
//===================================================================================================================


#include "loader.h"
#include "types.h"
#include "cpu.h"
#include "hw-disc.h"
#include "serial.h"
#include "printf.h"
#include "platform.h"


//
// -- Perform the early initialization.  The goal here is to not have to worry about loader/kernel code once
//    this is complete
//    ------------------------------------------------------------------------------------------------------
void __ldrtext EarlyInit(void)
{
    SerialOpen(&loaderSerial);       // initialize the serial port so we can output debug data

    if (CheckCpuid() != 0) {
        SetCpuid(true);
        CollectCpuid();
    }

    MmuEarlyInit();                 // Complete the MMU initialization for the loader
    PlatformEarlyInit();
}
