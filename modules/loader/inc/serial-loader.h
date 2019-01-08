//===================================================================================================================
//
//  serial-loader.h -- Serial debugging functions
//
//        Copyright (c)  2017-2019 -- Adam Clark
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
//
//===================================================================================================================


#ifndef __SERIAL_H__
#define __SERIAL_H__


#include "types.h"
#include "arch-serial.h"


//
// -- Initialize the serial port for writing debug data
//    -------------------------------------------------
void SerialInit(void);


//
// -- Send a string to the serial port
//    --------------------------------
void SerialPutS(const char *s);


//
// -- Send a single character to the serial port
//    ------------------------------------------
void SerialPutChar(const char ch);


//
// -- Send a hexidecimal number to the serial port
//    --------------------------------------------
void SerialPutHex(uint32_t val);


#endif
