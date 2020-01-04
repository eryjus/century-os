//===================================================================================================================
//
//  serial.h -- Serial debugging functions
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  These functions are used to send debugging information to the serial port.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Jun-27  Initial   0.1.0   ADCL  Initial version
//  2018-Nov-13  Initial   0.2.0   ADCL  Duplicate this file from libk, eliminating the libk version.
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#pragma once
#define __SERIAL_H__


#include "types.h"
#include "spinlock.h"


//
// -- go get any platform-specific definitions required for the serial port interface
//    -------------------------------------------------------------------------------
#if __has_include("platform-serial.h")
#   include "platform-serial.h"
#endif


//
// -- Now, carefully define a common interface for the serial port that can be used by both the loader and the
//    kernel.  There are 3 places we will want to be able to output to the serial port:
//    1) in the early loader before the kernel is mapped
//    2) in the loader after the kernel is mapped and can be called by far calls
//    3) in the kernel proper
//
//    As such, not all the functions will be implemented with each location
//    --------------------------------------------------------------------------------------------------------
typedef struct SerialDevice_t {
    SerialBase_t base;
    Spinlock_t lock;

    void (*SerialOpen)(struct SerialDevice_t *);
    bool (*SerialHasChar)(struct SerialDevice_t *);
    bool (*SerialHasRoom)(struct SerialDevice_t *);
    uint8_t (*SerialGetChar)(struct SerialDevice_t *);
    void (*SerialPutChar)(struct SerialDevice_t *, uint8_t);
    void (*SerialClose)(struct SerialDevice_t *);

    void *platformData;                         // This will be used by the platform for additional info
} SerialDevice_t;


//
// -- Here, declare the different configurations of the serial port we will use
//    -------------------------------------------------------------------------
EXTERN KERNEL_DATA
SerialDevice_t debugSerial;


//
// -- These are the common interface functions we will use to interact with the serial port.  These functions are
//    not safe in that they will not check for nulls before calling the function.  Therefore, caller beware!
//    -----------------------------------------------------------------------------------------------------------
EXTERN_C EXPORT KERNEL
inline void SerialOpen(SerialDevice_t *dev) { dev->SerialOpen(dev); }

EXTERN_C EXPORT KERNEL
inline void SerialClose(SerialDevice_t *dev) { dev->SerialClose(dev); }

EXTERN_C EXPORT KERNEL
inline bool SerialHasRoom(SerialDevice_t *dev) { return dev->SerialHasRoom(dev); }

EXTERN_C EXPORT KERNEL
inline bool SerialHasChar(SerialDevice_t *dev) { return dev->SerialHasChar(dev); }

EXTERN_C EXPORT KERNEL
inline uint8_t SerialGetChar(SerialDevice_t *dev) { return dev->SerialGetChar(dev); }

EXTERN_C EXPORT KERNEL
inline void SerialPutChar(SerialDevice_t *dev, uint8_t ch) { dev->SerialPutChar(dev, ch); }


//
// -- Here are the function prototypes that the operation functions need to conform to
//    --------------------------------------------------------------------------------
EXTERN_C EXPORT KERNEL
void _SerialOpen(SerialDevice_t *dev);

EXTERN_C EXPORT KERNEL
void _SerialClose(SerialDevice_t *dev);

EXTERN_C EXPORT KERNEL
bool _SerialHasRoom(SerialDevice_t *dev);

EXTERN_C EXPORT KERNEL
bool _SerialHasChar(SerialDevice_t *dev);

EXTERN_C EXPORT KERNEL
uint8_t _SerialGetChar(SerialDevice_t *dev);

EXTERN_C EXPORT KERNEL
void _SerialPutChar(SerialDevice_t *dev, uint8_t ch);

