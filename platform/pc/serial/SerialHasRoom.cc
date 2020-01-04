//===================================================================================================================
//
//  SerialHasRoom.cc -- Determine if there is room in the buffer for the character
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Feb-23  Initial   0.3.0   ADCL  InitialVersion
//
//===================================================================================================================


#include "hardware.h"
#include "serial.h"


//
// -- does the serial port have room for a character to be added?
//    -----------------------------------------------------------
EXTERN_C EXPORT KERNEL
bool _SerialHasRoom(SerialDevice_t *dev)
{
    if (!dev) return false;

    return ((inb(dev->base + SERIAL_LINE_STATUS) & 0x20) != 0);
}

