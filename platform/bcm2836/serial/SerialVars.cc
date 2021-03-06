//===================================================================================================================
//
//  SerialVars.cc -- These are the variables for the Serial Port for BCM2835
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
#include "hardware.h"
#include "mmu.h"
#include "serial.h"


//
// -- This is the device description that will be used for outputting data to the debugging serial port
//    -------------------------------------------------------------------------------------------------
EXPORT KERNEL_DATA
SerialDevice_t debugSerial = {
    .base = KRN_SERIAL_BASE,
    .lock = {0},
    .SerialOpen = _SerialOpen,
    .SerialHasChar = _SerialHasChar,
    .SerialHasRoom = _SerialHasRoom,
    .SerialGetChar = _SerialGetChar,
    .SerialPutChar = _SerialPutChar,
    .platformData = &kernelGpio,
};

