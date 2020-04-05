//===================================================================================================================
//
//  SerialGetChar.cc -- Get a single character from the serial port
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Apr-02  Initial  v0.6.6a  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "hardware.h"
#include "spinlock.h"
#include "serial.h"


//
// -- Output a single character to the serial port
//    --------------------------------------------
EXTERN_C EXPORT KERNEL
uint8_t _SerialGetChar(SerialDevice_t *dev)
{
    if (!dev) return 0;
    uint8_t rv;

    archsize_t flags = SPINLOCK_BLOCK_NO_INT(dev->lock) {
        while (!dev->SerialHasChar(dev)) {}

        rv = inb(dev->base + SERIAL_DATA);
        SPINLOCK_RLS_RESTORE_INT(dev->lock, flags);
    }

    return rv;
}
