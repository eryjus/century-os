//===================================================================================================================
//
//  SerialOpen.cc -- Initialize a serial port for debugging output
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
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//  2019-Feb-10  Initial   0.3.0   ADCL  Remove the call to BusyWait() and replace with a simple loop
//
//===================================================================================================================


#include "hardware.h"
#include "serial.h"


//
// -- Initialize the UART Serial Port
//    -------------------------------
EXTERN_C EXPORT KERNEL
void _SerialOpen(SerialDevice_t *dev)
{
    if (!dev) return;

    SerialBase_t base = dev->base;

    // -- must start by enabling the mini-UART; no register access will work until...
    MmioWrite(base + AUX_ENABLES, 1);

    // -- Disable all interrupts
    MmioWrite(base + AUX_MU_IER_REG, 0);

    // -- Reset the control register
    MmioWrite(base + AUX_MU_CNTL_REG, 0);

    // -- Program the Line Control Register -- 8 bits, please
    MmioWrite(base + AUX_MU_LCR_REG, 3); //

    // -- Program the Modem Control Register -- reset
    MmioWrite(base + AUX_MU_MCR_REG, 0);

    // -- Disable all interrupts -- again
    MmioWrite(base + AUX_MU_IER_REG, 0);

    // -- Clear all interrupts
    MmioWrite(base + AUX_MU_IIR_REG, 0xc6);

    // -- Set the BAUD to 115200 -- ((250,000,000/115200)/8)-1 = 270
    MmioWrite(base + AUX_MU_BAUD_REG, 270);

    GpioDevice_t *gpio = (GpioDevice_t *)dev->platformData;
    GpioSelectAlt(gpio, GPIO14, ALT5);
    GpioSelectAlt(gpio, GPIO15, ALT5);
    GpioEnablePin(gpio, GPIO14);
    GpioEnablePin(gpio, GPIO15);

    // -- Enable TX/RX
    MmioWrite(base + AUX_MU_CNTL_REG, 3);

    // -- clear the input buffer
    while ((MmioRead(base + AUX_MU_LSR_REG) & (1<<0)) != 0) MmioRead(base + AUX_MU_IO_REG);
}

