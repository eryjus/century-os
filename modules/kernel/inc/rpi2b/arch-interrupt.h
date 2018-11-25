//===================================================================================================================
//
//  arch-interrupt.cc -- These are functions related to interrupts for the rpi2b architecture
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  These are function prototypes for interrupts management
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-11  Initial   0.2.0   ADCL  Initial version
//
//===================================================================================================================


#ifndef __INTERRUPT_H__
#   error "Do not include 'arch-interrupt.h' directly.  Include 'interrupt.h' and this file will be included"
#endif

//
// -- Build the IDT and populate its gates; initialize the handlers to NULL
//    ---------------------------------------------------------------------
inline void IdtBuild(void) {}


