//===================================================================================================================
//
//  SerialVars.cc -- These are the variables for the Serial Port for BCM2835
//
//        Copyright (c)  2017-2019 -- Adam Clark
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
    .SerialHasRoom = _SerialHasRoom,
    .SerialPutChar = _SerialPutChar,
    .platformData = &kernelGpio,
};



EXPORT KERNEL_DATA
SerialDevice_t testSerial = {
    .base = LDR_SERIAL_BASE,
    .lock = {0},
    .SerialOpen = _SerialOpen,
    .SerialHasRoom = _SerialHasRoom,
    .SerialPutChar = _SerialPutChar,
    .platformData = &loaderGpio,
};



