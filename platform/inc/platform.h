//===================================================================================================================
//
//  platform.h -- These are the common functions for interacting with the platform
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Apr-05  Initial   0.4.1   ADCL  Initial version
//
//===================================================================================================================


#ifndef __PLATFORM_H__
#define __PLATFORM_H__


#include "types.h"


//
// -- Update where kprintf outputs its data to
//    ----------------------------------------
__CENTURY_FUNC__ void UpdateKprintfPort(void);


//
// -- This is the early platform initialization function
//    --------------------------------------------------
__CENTURY_FUNC__ void PlatformEarlyInit(void);


//
// -- Complete the platform initialization
//    ------------------------------------
__CENTURY_FUNC__ void PlatformInit(void);


#endif