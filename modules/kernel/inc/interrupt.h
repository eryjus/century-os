//===================================================================================================================
//
//  interrupt.cc -- These are functions related to interrupts
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  These are function prototypes for interrupts management.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Oct-10  Initial   0.1.0   ADCL  Initial version
//  2018-Nov-11  Initial   0.2.0   ADCL  Rename this file to be more generic, better architecture abstraction
//
//===================================================================================================================


#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__


#include "types.h"
#include "arch-interrupt.h"


//
// -- Unregister an ISR (fails if nothing is registered)
//    --------------------------------------------------
void IsrUnregister(uint8_t interrupt);


//
// -- Register an ISR (fails if something is already registered)
//    ----------------------------------------------------------
void IsrRegister(uint8_t interrupt, isrFunc_t func);


//
// -- Dump the CPU state
//    ------------------
void IsrDumpState(isrRegs_t *regs);


//
// -- The system call handler
//    -----------------------
void SyscallHandler(isrRegs_t *regs);


#endif
