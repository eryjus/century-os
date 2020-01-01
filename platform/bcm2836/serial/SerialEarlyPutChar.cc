//===================================================================================================================
//
//  SerialEarlyPutChar.cc -- Write a single character to the UART Serial Port -- for being called early in boot
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Apr-02  Initial   0.4.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "loader.h"
#include "hardware.h"
#include "serial.h"


__CENTURY_FUNC__ void SerialEarlyPutChar(uint8_t ch);

//
// -- Write a single character to the UART
//    ------------------------------------
void __ldrtext SerialEarlyPutChar(uint8_t ch)
{
    if (ch == '\n') SerialEarlyPutChar('\r');

    while ((MmioRead(LDR_SERIAL_BASE + AUX_MU_LSR_REG) & (1<<5)) == 0) { }

    MmioWrite(LDR_SERIAL_BASE + AUX_MU_IO_REG, ch);
}
