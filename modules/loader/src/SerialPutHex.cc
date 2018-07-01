//===================================================================================================================
//
//  loader/src/SeriapPutHex.cc -- Output a Hex Number to the Serial Port
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date     Tracker  Version  Pgmr  Description
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2017-06-07  Initial   0.0.0   ADCL  Initial version
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
    extern uint16_t serialPort;
    char hex[] = "0123456789abcdef";

    SerialPutS("0x");
    for (int i = 28; i >= 0; i -= 4) {
        while ((inb(serialPort + 5) & 0x20) == 0) {}
        outb(serialPort, hex[(val >> i) & 0x0f]);
    }
}