//===================================================================================================================
//
//  platform.h -- These are the common functions for interacting with the platform
//
//        Copyright (c)  2017-2020 -- Adam Clark
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


#pragma once
#define __PLATFORM_H__


#include "types.h"


//
// -- This is the early platform initialization function
//    --------------------------------------------------
EXTERN_C EXPORT LOADER
void PlatformEarlyInit(void);


//
// -- Complete the platform initialization
//    ------------------------------------
EXTERN_C EXPORT LOADER
void PlatformInit(void);


//
// -- Complete the platform-specific initialization for the AP
//    --------------------------------------------------------
EXTERN_C EXPORT KERNEL
void PlatformApInit(void);

