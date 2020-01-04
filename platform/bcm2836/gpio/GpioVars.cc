//===================================================================================================================
//
//  GpioVars.cc -- These are the variables for the BCM2835 GPIO block
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


#include "mmu.h"
#include "hardware.h"


//
// -- This is the device structure that will be used for the loader to access the gpio
//    --------------------------------------------------------------------------------
__ldrdata GpioDevice_t loaderGpio = {
    .base = LDR_GPIO_BASE,
    .GpioSelectAlt = _GpioSelectAlt,
    .GpioEnablePin = _GpioEnablePin,
};


//
// -- This is the device structure that will be used for the kernel to access the gpio
//    --------------------------------------------------------------------------------
__krndata GpioDevice_t kernelGpio = {
    .base = KRN_GPIO_BASE,
    .GpioSelectAlt = _GpioSelectAlt,
    .GpioEnablePin = _GpioEnablePin,
};

