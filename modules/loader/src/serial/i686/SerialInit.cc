//===================================================================================================================
//
//  SerialInit.cc -- Initialize a serial port for debugging output
//
//        Copyright (c)  2017-2018 -- Adam Clark
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


#include "bda.h"
#include "cpu.h"
#include "serial-loader.h"


//
// -- This is the serial port for debugging
//    -------------------------------------
uint16_t serialPort = 0;


//
// -- Initialize the serial port for debugging output
//    -----------------------------------------------
void SerialInit(void)
{
    serialPort = BdaGetCom1();

    outb(serialPort + 1, 0x00);    // Disable all interrupts
    outb(serialPort + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outb(serialPort + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    outb(serialPort + 1, 0x00);    //                  (hi byte)
    outb(serialPort + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(serialPort + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    outb(serialPort + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}
