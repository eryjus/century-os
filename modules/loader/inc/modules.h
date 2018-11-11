//===================================================================================================================
//
//  modules.h -- The loader functions for handling the modules loaded by Multiboot
//
//        Copyright (c)  2017-2018 -- Adam Clark
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
// -- This is the entry point for the kernel
//    --------------------------------------
typedef void (*kernEntry_t)(void);


//
// -- Initialize the modules loaded by Multiboot
//    ------------------------------------------
kernEntry_t ModuleInit(void);


#endif
