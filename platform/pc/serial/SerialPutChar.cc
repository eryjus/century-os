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


#include "cpu.h"
#include "serial.h"


//
// -- For debugging to the serial port, this is the hardware port number
//    ------------------------------------------------------------------
devaddr_t serialPort = 0x3f8;


//
// -- Output a single character to the serial port
//    --------------------------------------------
void SerialPutChar(const char ch)
{
    if (ch == '\n') SerialPutChar('\r');
    while ((inb(serialPort + 5) & 0x20) == 0) {}

    outb(serialPort, ch);
}
