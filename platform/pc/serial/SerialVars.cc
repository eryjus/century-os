//===================================================================================================================
//
//  SerialVars.cc -- These are the variables for the Serial Port for x86
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


#include "loader.h"
#include "serial.h"


//
// -- This is the device description that is used to output data to the serial port during loader initialization
//    ----------------------------------------------------------------------------------------------------------
__ldrdata SerialDevice_t loaderSerial = {
    .base = COM1,
    .SerialOpen = _SerialOpen,                   // -- already in the __ldrtext section
    .SerialHasRoom = (bool (*)(SerialDevice_t *))PHYS_OF(_SerialHasRoom),    // -- in the kernel address space
    .SerialPutChar = (void (*)(SerialDevice_t *, uint8_t))PHYS_OF(_SerialPutChar), // -- in the kernel address space
};


//
// -- This is the device descriptionthat will be used once we can use the full kernel address space.  Note the
//    missing `SerialOpen()` function which is not located in the kernel section
//    --------------------------------------------------------------------------------------------------------
__krndata SerialDevice_t kernelSerial = {
    .base = COM1,
    .SerialHasRoom = _SerialHasRoom,
    .SerialPutChar = _SerialPutChar,
};



