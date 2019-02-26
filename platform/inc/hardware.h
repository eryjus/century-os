//===================================================================================================================
//
//  hardware.h -- These are the hardware abstractions for interacting with any of the platform devices
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  This header and the types and constants herein are used to provide a layer of abstraction between the kernel
//  code and the platform implementation.  The key to success here is that all of the hardware needs to have a
//  common interface to the kernel.  For example, all the serial functions need to look the same across all
//  platforms.  This means that on BCM2835 where it is accessed by MMIO, and x86 where it is accessed by I/O port,
//  the function prototypes need to be the same.  Complicate that by the fact that I will likely call each of these
//  (at least for the serial port) from both the loader and from the kernel and there is some thought that needs to
//  go into this effort.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Feb-23  Initial   0.3.0   ADCL  Initial version
//
//===================================================================================================================


#ifndef __HARDWARE_H__
#define __HARDWARE_H__


#include "types.h"


//
// == First take care of some of the more common macros that will be leveraged across all platforms
//    =============================================================================================


//
// -- Write to a Memory Mapped I/O Register
//    -------------------------------------
#define MmioWrite(regLocation,data) (*(volatile archsize_t *)(regLocation) = (data))


//
// -- Read from a Memory Mapped I/O Register
//    --------------------------------------
#define MmioRead(regLocation) (*(volatile archsize_t *)(regLocation))


//
// -- Include any platform-dependent I/O functions that might be required
//    -------------------------------------------------------------------
#if __has_include("platform-io.h")
#   include "platform-io.h"
#endif

#if __has_include("platform-gpio.h")
#   include "platform-gpio.h"
#endif

#if __has_include("platform-mailbox.h")
#   include "platform-mailbox.h"
#endif


//
// -- Include the different architectures' hardware definitions
//    ---------------------------------------------------------




#endif
