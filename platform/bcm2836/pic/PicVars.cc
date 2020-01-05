//===================================================================================================================
//
//  PicVars.cc -- These are the variables for the bcm2835 Pic
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
#include "pic.h"


//
// -- This is the data that will be used to manage the pic
//    ----------------------------------------------------
EXPORT KERNEL_DATA
Bcm2835Pic_t bcm2835Data = {
    .picLoc = BCM2835_PIC,
    .timerLoc = BCM2835_TIMER,
};

//
// -- This is the device description that is used to output data to the serial port during loader initialization
//    ----------------------------------------------------------------------------------------------------------
EXPORT KERNEL_DATA
PicDevice_t picBcm2835 = {
    .device = { .deviceData = (DeviceData_t *)&bcm2835Data, },
    .PicInit = _PicInit,
    .PicMaskIrq = _PicMaskIrq,
    .PicUnmaskIrq = _PicUnmaskIrq,
    .PicDetermineIrq = _PicDetermineIrq,
    .PicBroadcastIpi = _PicBroadcastIpi,
};


//
// -- This is the pic we are going to use
//    -----------------------------------
EXPORT KERNEL_DATA
PicDevice_t *picControl = &picBcm2835;

