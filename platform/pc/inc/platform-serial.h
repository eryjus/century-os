//===================================================================================================================
//
//  platform-serial.h -- Serial definitions and functions for the x86 serial port
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Feb-23  Initial   0.3.0   ADCL  Initial version
//
//===================================================================================================================


#ifndef __SERIAL_H__
#   error "Use #include \"serial.h\" and it will pick up this file; do not #include this file directly."
#endif


//
// -- on x86, this is the type we use to refer to the serial port
//    -----------------------------------------------------------
typedef uint16_t SerialBase_t;


//
// -- Some constants used to help manage the serial port
//    --------------------------------------------------
#define COM1        (0x3f8)

#define SERIAL_DATA                     0
#define SERIAL_DIVISOR_LSB              0
#define SERIAL_INTERRUPT_ENABLE         1
#define SERIAL_DIVISOR_MSB              1
#define SERIAL_FIFO_CONTROL             2
#define SERIAL_LINE_CONTROL             3
#define SERIAL_MODEM_CONTROL            4
#define SERIAL_LINE_STATUS              5
