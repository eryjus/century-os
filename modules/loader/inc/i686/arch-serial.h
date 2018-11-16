//===================================================================================================================
//
//  arch-serial-loader.h -- This is the architecture-specific mmu functions for the i686 architecture
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


#ifndef __SERIAL_H__
#   error "Do not include 'arch-serial-loader.h' directly.  Instead, include 'serial.h'"
#endif


#include "types.h"


//
// -- This is the serial port we will use for debugging info
//    ------------------------------------------------------
extern uint16_t serialPort;

