//===================================================================================================================
//
//  GpioEnablePin.cc -- Enable a pin on the GPIO block
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Feb-24  Initial   0.3.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "hardware.h"


//
// -- Select the alternate function for a Gpio pin
//    --------------------------------------------
EXTERN_C EXPORT KERNEL
void _GpioEnablePin(GpioDevice_t *dev, GpioPin_t pin)
{
    if (!dev) return;
    if (pin < 0 || pin > 53) return;

    volatile int i;         // make sure the compiler does not try to optimize this away
    int bank = pin / 32;
    int shift = pin % 32;

    MmioWrite(dev->base + GPIO_GPPUD, 0x00000000);
    for (i = 0; i < 200; i ++) {}

    MmioWrite(dev->base + GPIO_GPPUDCLK1 + bank * 4, 1 << shift);
    for (i = 0; i < 200; i ++) {}

//    MmioWrite(dev->base + GPIO_GPPUD, 0x00000000);
//    MmioWrite(dev->base + GPIO_GPPUDCLK1 + bank * 4, 0x00000000);
//    for (i = 0; i < 200; i ++) {}
}
