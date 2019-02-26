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
#include "serial.h"


//
// -- Perform the early initialization.  The goal here is to not have to worry about loader/kernel code once
//    this is complete
//    ------------------------------------------------------------------------------------------------------
void __ldrtext EarlyInit(void)
{
    LoaderSerialPutS(".... Opening the serial port again\n");
    SerialOpen(&loaderSerial);                   // initialize the serial port so we can output debug data

    LoaderSerialPutS(".... At this point, the serial port has been re-opened and we should still be able to write\n");

    MmuEarlyInit();                 // Complete the MMU initialization for the loader
}
