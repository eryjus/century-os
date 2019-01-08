//===================================================================================================================
//
//  SerialPutS.cc -- Output a string to the serial port
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
//
//===================================================================================================================


#include "screen01.h"


//
// -- Send a character string to a serial port
//    ----------------------------------------
void SerialPutS(const char *s)
{
    while (*s) SerialPutChar(*s ++);
}
