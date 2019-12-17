//===================================================================================================================
//
//  HwDiscovery.cc -- This source contains the i686 implementation of the hardware discovery.
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2017-Jun-09  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "printf.h"
#include "hw-disc.h"


//
// -- Perform the hardware discovery
//    ------------------------------
void __ldrtext HwDiscovery(void)
{
    kMemSetB(localHwDisc, 0, sizeof(HardwareDiscovery_t));
    Mb1Parse();
    Mb2Parse();
//    ArchDiscovery();
}
