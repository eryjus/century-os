//===================================================================================================================
//
//  hw-disc.cc -- hardware discovery structure implementation
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2017-Jun-07  Initial   0.1.0   ADCL  Initial version
//  2019-Feb-14  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "hw-disc.h"


//
// -- This is the local version of what we have found for hardware; these will be located in the loader addr space.
//    -------------------------------------------------------------------------------------------------------------
__ldrdata HardwareDiscovery_t _localHwDisc;
__ldrdata HardwareDiscovery_t *localHwDisc = &_localHwDisc;

