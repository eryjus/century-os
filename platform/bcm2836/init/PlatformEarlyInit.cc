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


#include "types.h"
#include "serial.h"
#include "hw-disc.h"
#include "cpu.h"
#include "printf.h"
#include "platform.h"


//
// -- Handle the early initialization for the pc platform
//    ---------------------------------------------------
EXTERN_C EXPORT LOADER
void PlatformEarlyInit(void)
{
    SerialOpen(&debugSerial);       // initialize the serial port so we can output debug data
    kprintf("Hello...\n");
    HwDiscovery();

    // -- at some point, this will come from the DTB
    cpus.cpusDiscovered = 4;
    cpus.cpusRunning = 1;
    if (cpus.cpusDiscovered > MAX_CPUS) cpus.cpusDiscovered = MAX_CPUS;

    CpuInit();
}



