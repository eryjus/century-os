//===================================================================================================================
//
//  EarlyInit.cc -- Handle the early initialization for the arm architecture
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
#include "printf.h"
#include "serial.h"



//
// -- Perform the early initialization.  The goal here is to not have to worry about loader/kernel code once
//    this is complete
//    ------------------------------------------------------------------------------------------------------
void __ldrtext EarlyInit(void)
{
    SerialOpen(&loaderSerial);                   // initialize the serial port so we can output debug data

    MmuEarlyInit();                 // Complete the MMU initialization for the loader

    //
    // -- prepare the FPU for accepting commands
    //    --------------------------------------
    archsize_t cpacr = READ_CPACR();
    cpacr |= (0b11<<20);
    cpacr |= (0b11<<22);
    WRITE_CPACR(cpacr);

    //
    // -- and enable the fpu
    //    ------------------
    WRITE_FPEXC(1<<30);
}
