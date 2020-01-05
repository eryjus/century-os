//===================================================================================================================
//
//  SerialVars.cc -- These are the variables for the Serial Port for x86
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Feb-23  Initial   0.3.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "serial.h"


//
// -- This is the device description that will be used for outputting data to the debugging serial port
//    -------------------------------------------------------------------------------------------------
EXPORT KERNEL_DATA
SerialDevice_t debugSerial = {
    .base = COM1,
    .lock = {0},
    .SerialOpen = _SerialOpen,
    .SerialHasRoom = _SerialHasRoom,
    .SerialPutChar = _SerialPutChar,
};



