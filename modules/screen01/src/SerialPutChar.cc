//===================================================================================================================
//
//  SerialPutChar.cc -- Write a single character to the UART Serial Port
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-13  Initial   0.2.0   ADCL  Initial version -- leveraged out of century's `uart-dev.c`
//
//===================================================================================================================


#include "screen01.h"


//
// -- Write a single character to the UART
//    ------------------------------------
void SerialPutChar(char byte)
{
    if (byte == '\n') SerialPutChar('\r');
    while ((MmioRead(AUX_MU_LSR_REG) & (1<<5)) == 0) { }
    MmioWrite(AUX_MU_IO_REG, byte);
}
