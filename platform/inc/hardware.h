//===================================================================================================================
//
//  hardware.h -- These are the hardware abstractions for interacting with any of the platform devices
//
//        Copyright (c)  2017-2020 -- Adam Clark
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


#pragma once
#define __HARDWARE_H__


#include "types.h"
#include "lists.h"


//
// -- Write to a Memory Mapped I/O Register
//    -------------------------------------
EXPORT INLINE
void MmioWrite(archsize_t regLocation, uint32_t data) { (*((volatile uint32_t *)(regLocation)) = (data)); }


//
// -- Write to a 64-bit Memory Mapped I/O Register
//    --------------------------------------------
EXPORT INLINE
void MmioWrite64(archsize_t regLocation, uint64_t data) { (*((volatile uint64_t *)(regLocation)) = (data)); }


//
// -- Read from a Memory Mapped I/O Register
//    --------------------------------------
EXPORT INLINE
uint32_t MmioRead(archsize_t regLocation) { return (*((volatile uint32_t *)(regLocation))); }


//
// -- Read from a 64-bit Memory Mapped I/O Register
//    ---------------------------------------------
EXPORT INLINE
uint64_t MmioRead64(archsize_t regLocation) { return (*((volatile uint64_t *)(regLocation))); }


//
// -- This is the base of a device data structure; this structure will need to be included
//    as the first data member (not a pointer) of any device's specific data.
//    ------------------------------------------------------------------------------------
typedef void *DeviceData_t;


//
// -- This structure is the basis for any device that is managed in Century; this structure will also
//    need to be included as the first data member (not a pointer) of any device's structure.
//    -----------------------------------------------------------------------------------------------
typedef struct GenericDevice_t {
    struct GenericDevice_t *parent;
    ListHead_t::List_t siblings;
    ListHead_t children;
    char name[MAX_DEV_NAME];
    DeviceData_t deviceData;
} GenericDevice_t;


//
// -- Include any platform-dependent I/O functions that might be required
//    -------------------------------------------------------------------
#if __has_include("platform-acpi.h")
#   include "platform-acpi.h"
#endif

#if __has_include("platform-io.h")
#   include "platform-io.h"
#endif

#if __has_include("platform-gpio.h")
#   include "platform-gpio.h"
#endif

#if __has_include("platform-mailbox.h")
#   include "platform-mailbox.h"
#endif

#if __has_include("platform-init.h")
#   include "platform-init.h"
#endif


