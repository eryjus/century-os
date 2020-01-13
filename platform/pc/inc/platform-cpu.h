//===================================================================================================================
//
//  platform-cpu.h -- These are the structures and functions for the final CPU initialization
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Jan-05  Initial  v0.5.0e  ADCL  Initial version
//
//===================================================================================================================


#pragma once


#ifndef __PLATFORM_H__
#   error "Use #include \"platform.h\" and it will pick up this file; do not #include this file directly."
#endif


#include "types.h"


//
// -- Initialize the GDT to its final location
//    ----------------------------------------
EXTERN_C EXPORT LOADER
void InitGdt(void);


