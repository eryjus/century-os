//===================================================================================================================
//
//  SerialInit.cc -- Initialize a serial port for debugging output
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
// -- Initialize the UART Serial Port
//    -------------------------------
void SerialInit(void)
{
    // -- must start by enabling the mini-UART; no register access will work until...
    MmioWrite(AUX_ENABLES, 1);

    // -- Disable all interrupts
    MmioWrite(AUX_MU_IER_REG, 0);

    // -- Reset the control register
    MmioWrite(AUX_MU_CNTL_REG, 0);

    // -- Program the Line Control Register -- 8 bits, please
    MmioWrite(AUX_MU_LCR_REG, 3);

    // -- Program the Modem Control Register -- reset
    MmioWrite(AUX_MU_MCR_REG, 0);

    // -- Disable all interrupts -- again
    MmioWrite(AUX_MU_IER_REG, 0);

    // -- Clear all interrupts
    MmioWrite(AUX_MU_IIR_REG, 0xc6);

    // -- Set the BAUD to 115200 -- ((250,000,000/115200)/8)-1 = 270
    MmioWrite(AUX_MU_BAUD_REG, 270);

    // -- Select alternate function 5 to work on GPIO pin 14
    uint32_t sel = MmioRead(GPIO_FSEL1);
    sel &= ~(7<<12);
    sel |= (0b010<<12);
    sel &= ~(7<<15);
    sel |= (0b010<<15);
    MmioWrite(GPIO_FSEL1, sel);

    // -- Enable GPIO pins 14/15 only
    MmioWrite(GPIO_GPPUD, 0x00000000);
    BusyWait(150);
    MmioWrite(GPIO_GPPUDCLK1, (1<<14)|(1<<15));
    BusyWait(150);
    MmioWrite(GPIO_GPPUDCLK1, 0x00000000);              // LEARN: Why does this make sense?

    // -- Enable TX/RX
    MmioWrite(AUX_MU_CNTL_REG, 3);

    // -- clear the input buffer
    while ((MmioRead(AUX_MU_LSR_REG) & (1<<0)) != 0) MmioRead(AUX_MU_IO_REG);

    SerialPutS("Serial port initialized!\n");
}
