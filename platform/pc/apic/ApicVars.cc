//===================================================================================================================
//
//  ApicVars.cc -- These are the variables for the x86 APIC
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Apr-19  Initial   0.4.1   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "pic.h"


//
// -- This is the structure for the data needed by this driver
//    --------------------------------------------------------
__krndata ApicDeviceData_t apicData = {0};


//
// -- This is the device description that is used to output data to the serial port during loader initialization
//    ----------------------------------------------------------------------------------------------------------
__krndata PicDevice_t apicDriver = {
    .device = {
        .name = {'a', 'p', 'i', 'c', '\0'},
        .deviceData = (DeviceData_t)&apicData,
    },
    .PicInit = _ApicInit,
    .PicRegisterHandler = _ApicRegisterHandler,
    .PicMaskIrq = _ApicMaskIrq,
    .PicUnmaskIrq = _ApicUnmaskIrq,
    .PicEoi = _ApicEoi,
};
