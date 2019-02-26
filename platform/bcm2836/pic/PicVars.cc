//===================================================================================================================
//
//  PicVars.cc -- These are the variables for the bcm2835 Pic
//
//        Copyright (c)  2017-2019 -- Adam Clark
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


#include "timer.h"
#include "cpu.h"
#include "pic.h"


//
// -- This is the device description that is used to output data to the serial port during loader initialization
//    ----------------------------------------------------------------------------------------------------------
__krndata PicDevice_t picControl = {
    .base1 = PIC,
    .base2 = TIMER,
    .PicInit = _PicInit,
    .PicEnableIrq = _PicEnableIrq,
    .PicDisableIrq = _PicDisableIrq,
    .PicGetIrq = _PicGetIrq,
};
