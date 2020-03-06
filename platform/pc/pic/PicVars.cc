//===================================================================================================================
//
//  PicVars.cc -- These are the variables for the x86 Pic
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
// -- This is the device description that is used to output data to the serial port during loader initialization
//    ----------------------------------------------------------------------------------------------------------
EXPORT KERNEL_DATA
PicDevice_t pic8259 = {
    .ipiReady = false,
    .PicInit = _PicInit,
    .PicRegisterHandler = _PicRegisterHandler,
    .PicMaskIrq = _PicMaskIrq,
    .PicUnmaskIrq = _PicUnmaskIrq,
    .PicEoi = _PicEoi,
    .PicBroadcastIpi = (void (*)(PicDevice_t *, int))EmptyFunction,
};
