//===================================================================================================================
//
//  SerialHasChar.cc -- Determine if there is data in the buffer to be read
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Apr-02  Initial  v0.6.0a  ADCL  Initial Version
//
//===================================================================================================================


#include "types.h"
#include "hardware.h"
#include "serial.h"


//
// -- does the serial port have room for a character to be added?
//    -----------------------------------------------------------
EXTERN_C EXPORT KERNEL
bool _SerialHasChar(SerialDevice_t *dev)
{
    if (!dev) return false;

    archsize_t val = MmioRead(dev->base + AUX_MU_LSR_REG);
    val &= 1;

    return (val != 0);
}

