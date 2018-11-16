 //===================================================================================================================
//
//  SerialPutChar.cc -- Output a single character to the serial port
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2017-Nov-11  Initial   0.0.0   ADCL  Initial version -- well, documentated at the first time
//
//===================================================================================================================


#include "cpu.h"
#include "serial.h"


//
// -- Output a single character to the serial port
//    --------------------------------------------
void SerialPutChar(const char ch)
{
    while ((inb(serialPort + 5) & 0x20) == 0) {}

    outb(serialPort, ch);
}
