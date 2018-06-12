//===================================================================================================================
//
//  loader/src/i686/HwDiscovery.cc -- This source contains the i686 implementation of the hardware discovery.
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date     Tracker  Version  Pgmr  Description
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2017-06-09  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "bda.h"
#include "pmm.h"
#include "hw-disc.h"


//
// -- Perform the hardware discovery
//    ------------------------------
void HwDiscovery(void)
{
    Mb1Parse();
    Mb2Parse();
    BdaRead();
    PmmInit();
}