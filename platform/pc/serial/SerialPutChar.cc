//===================================================================================================================
//
//  SerialPutChar.cc -- Output a single character to the serial port
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2017-Nov-11  Initial   0.0.0   ADCL  Initial version -- well, documentated at the first time
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "hardware.h"
#include "spinlock.h"
#include "serial.h"


//
// -- Output a single character to the serial port
//    --------------------------------------------
EXTERN_C EXPORT KERNEL
void _SerialPutChar(SerialDevice_t *dev, uint8_t ch)
{
    if (!dev) return;
    if (ch == '\n') dev->SerialPutChar(dev, '\r');

    archsize_t flags = SPINLOCK_BLOCK_NO_INT(dev->lock) {
        while (!dev->SerialHasRoom(dev)) {}

        outb(dev->base + SERIAL_DATA, ch);
        SPINLOCK_RLS_RESTORE_INT(dev->lock, flags);
    }
}
