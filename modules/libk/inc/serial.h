//===================================================================================================================
//
// loader/inc/serial.h -- Serial debugging functions
//
// These functions are used to send debugging information to the serial port.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date     Tracker  Version  Pgmr  Description
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-06-27  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#ifndef __SERIAL_H__
#define __SERIAL_H__


#include "types.h"


//
// -- This is the serial port we will use for debugging info
//    ------------------------------------------------------
extern uint16_t serialPort;


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
