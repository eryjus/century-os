//===================================================================================================================
//
//  PlatformEarlyInit.cc -- Handle the early initialization for the bcm2835 platform
//
//        Copyright (c)  2017-2019 -- Adam Clark
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
#include "hw-disc.h"
#include "platform.h"


//
// -- Handle the early initialization for the pc platform
//    ---------------------------------------------------
void __ldrtext PlatformEarlyInit(void)
{
    HwDiscovery();

    // -- at some point, this will come from the DTB
    cpus.cpusDiscovered = 4;
}



