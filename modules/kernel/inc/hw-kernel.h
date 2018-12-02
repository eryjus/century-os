//===================================================================================================================
//
//  hw-kernel.h -- This is the include file for any hardware-specific lcoations
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-13  Initial   0.2.0   ADCL  Initial version
//
//===================================================================================================================


#ifndef __HW_H__
#define __HW_H__


//
// -- This is the offset to add to the MMIO addresses to get them into kernel address space
//    -------------------------------------------------------------------------------------
#define KERNEL_OFFSET       (0xf2000000 - 0x3f000000)


#include "arch-hw-prevalent.h"


#endif
