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



#define PlatformDiscovery()


//
// -- all low memory is available on rpi2b
//    ------------------------------------
#define LowMemCheck(frame)      (frame?true:false)


