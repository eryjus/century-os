//===================================================================================================================
//
//  GpioSelectAlt.cc -- Select the alternate function for a GPIO pin
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


#include "hardware.h"


//
// -- Select the alternate function for a Gpio pin
//    --------------------------------------------
void _GpioSelectAlt(GpioDevice_t *dev, GpioPin_t pin, GpioAlt_t alt)
{
    if (!dev) return;
    if (pin < 0 || pin > 53) return;
    if (alt < 0 || alt > 5) return;

    int bank = pin / 10;
    int shift = (pin % 10) << 3;

    archsize_t sel = MmioRead(dev->base + GPIO_FSEL0 + (bank * 4));
    sel &= ~(7 << shift);
    sel |= (alt << shift);
    MmioWrite(dev->base + GPIO_FSEL0 + (bank * 4), sel);
}
