//===================================================================================================================
//
//  SerialPutChar.cc -- Write a single character to the UART Serial Port
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-13  Initial   0.2.0   ADCL  Initial version -- leveraged out of century's `uart-dev.c`
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "types.h"
#include "hardware.h"
#include "spinlock.h"
#include "serial.h"


EXTERN_C EXPORT LOADER
void ___SerialPutChar(int ch)
{
    debugSerial.SerialPutChar(&debugSerial, (uint8_t)ch);
}

//
// -- Write a single character to the UART
//    ------------------------------------
EXTERN_C EXPORT KERNEL
void _SerialPutChar(SerialDevice_t *dev, uint8_t ch)
{
    if (!dev) return;
    if (ch == '\n') dev->SerialPutChar(dev, '\r');

    archsize_t flags = SPINLOCK_BLOCK_NO_INT(dev->lock) {
        while ((MmioRead(dev->base + AUX_MU_LSR_REG) & (1<<5)) == 0) { }

        MmioWrite(dev->base + AUX_MU_IO_REG, ch);
        SPINLOCK_RLS_RESTORE_INT(dev->lock, flags);
    }
}
