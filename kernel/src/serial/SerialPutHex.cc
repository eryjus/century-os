//===================================================================================================================
//
//  SerialPutHex.cc -- Output a Hex Number to the Serial Port
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  Note that this function has been deliberately rewritten not to use strings due to the issues with linking
//  strings at the loader sections.
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
void __ldrtext SerialPutHex(uint32_t val)
{
    SerialPutChar('0');
    SerialPutChar('x');
    for (int i = 28; i >= 0; i -= 4) {
        char c = ((val >> i) & 0x0f);

        if (c > 9) SerialPutChar(c - 10 + 'a');
        else SerialPutChar(c + '0');
    }
}
