//===================================================================================================================
//
//  platform-init.h -- Some platform-specific initialization routines
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Apr-10  Initial  v0.6.1b  ADCL  Initial version
//
//===================================================================================================================


#pragma once


#ifndef __HARDWARE_H__
#   error "Use #include \"hardware.h\" and it will pick up this file; do not #include this file directly."
#endif



#include "types.h"


//
// -- This is the architecture-specific discovery
//    -------------------------------------------
#define PlatformDiscovery()     SetEbda(0x80000);
/*#define PlatformDiscovery() do {                                    \*/
/*        uint32_t *ebdaLoc = (uint32_t *)0x40e;                      \*/
/*        if (*ebdaLoc != 0) SetEbda(*ebdaLoc);                       \*/
/*        else SetEbda(0x80000);                                      \*/
/*    } while (0)                                                      */




//
// -- all low memory is available on rpi2b
//    ------------------------------------
EXTERN_C EXPORT LOADER
bool LowMemCheck(frame_t frame);

