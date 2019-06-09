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
// -- This is the data that will be used to manage the pic
//    ----------------------------------------------------
__krndata Bcm2835Pic_t bcm2835Data = {
    .picLoc = PIC,
    .timerLoc = TIMER,
};

//
// -- This is the device description that is used to output data to the serial port during loader initialization
//    ----------------------------------------------------------------------------------------------------------
__krndata PicDevice_t picBcm2835 = {
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
PicDevice_t *picControl = &picBcm2835;

