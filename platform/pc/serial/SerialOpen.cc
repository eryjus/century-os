//===================================================================================================================
//
//  SerialOpen.cc -- Initialize and open a serial port for debugging output
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


#include "hardware.h"
#include "serial.h"


//
// -- Initialize the serial port for debugging output
//    -----------------------------------------------
void __ldrtext _SerialOpen(SerialDevice_t *dev)
{
    if (!dev) return;

    SerialBase_t base = dev->base;

    outb(base + SERIAL_INTERRUPT_ENABLE, 0x00); // Disable all interrupts
    outb(base + SERIAL_LINE_CONTROL, 0x80);     // Enable DLAB (set baud rate divisor)
    outb(base + SERIAL_DIVISOR_LSB, 0x03);      // Set divisor to 3 (lo byte) 38400 baud
    outb(base + SERIAL_DIVISOR_MSB, 0x00);      //                  (hi byte)
    outb(base + SERIAL_LINE_CONTROL, 0x03);     // 8 bits, no parity, one stop bit
    outb(base + SERIAL_FIFO_CONTROL, 0xC7);     // Enable FIFO, clear them, with 14-byte threshold
    outb(base + SERIAL_MODEM_CONTROL, 0x0B);    // IRQs enabled, RTS/DSR set
}
