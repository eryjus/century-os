//===================================================================================================================
//
//  PlatformEarlyInit.cc -- Handle the early initialization for the bcm2835 platform
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  This function is called after `MmuEarlyInit()`, so we expect to have access to kernel virtual memory addresses.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Apr-05  Initial   0.4.1   ADCL  Initial version
//
//===================================================================================================================


#include "hardware.h"
#include "cpu.h"
#include "serial.h"
#include "printf.h"
#include "hw-disc.h"
#include "platform.h"

EXTERN EXPORT KERNEL_DATA
SerialDevice_t testSerial;


//
// -- Handle the early initialization for the pc platform
//    ---------------------------------------------------
EXTERN_C EXPORT LOADER
void PlatformEarlyInit(void)
{
    SerialOpen(&debugSerial);       // initialize the serial port so we can output debug data
    HwDiscovery();

    // -- at some point, this will come from the DTB
    cpus.cpusDiscovered = 4;
    FpuInit();
}



