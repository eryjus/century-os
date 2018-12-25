//===================================================================================================================
//
//  SerialPutChar.cc -- Write a single character to the UART Serial Port
//
//        Copyright (c)  2017-2018 -- Adam Clark
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


#include "cpu.h"
#include "serial-loader.h"


//
// -- Write a single character to the UART
//    ------------------------------------
void SerialPutChar(char byte)
{
    if (byte == '\n') SerialPutChar('\r');
    while ((MmioRead(UART_BASE + UART_FR) & UARTFR_TXFF) != 0) { }

    MmioWrite(UART_BASE + UART_DR, byte);
}
