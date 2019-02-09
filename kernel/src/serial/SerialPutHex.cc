//===================================================================================================================
//
//  SerialPutHex.cc -- Output a Hex Number to the Serial Port
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2017-Jun-07  Initial   0.0.0   ADCL  Initial version
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "types.h"
#include "serial.h"
#include "cpu.h"


//
// -- Print a hex number to the serial port
//    -------------------------------------
void SerialPutHex(uint32_t val)
{
    char hex[] = "0123456789abcdef";

    SerialPutS("0x");
    for (int i = 28; i >= 0; i -= 4) {
        SerialPutChar(hex[(val >> i) & 0x0f]);
    }
}
