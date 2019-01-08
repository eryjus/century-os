//===================================================================================================================
//
//  modules.h -- The loader functions for handling the modules loaded by Multiboot
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
//
//===================================================================================================================


#ifndef __MODULES_H__
#define __MODULES_H__


//
// -- Initialize the modules loaded by Multiboot
//    ------------------------------------------
ptrsize_t ModuleInit(void);


#endif
