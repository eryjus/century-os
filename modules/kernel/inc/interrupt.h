//===================================================================================================================
//
//  interrupt.cc -- These are functions related to interrupts
//
//        Copyright (c)  2017-2020 -- Adam Clark
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
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#pragma once


#define __INTERRUPT_H__


#include "types.h"
#include "arch-interrupt.h"


//
// -- Unregister an ISR (fails if nothing is registered)
//    --------------------------------------------------
EXTERN_C EXPORT KERNEL
void IsrUnregister(uint8_t interrupt);


//
// -- Register an ISR (fails if something is already registered)
//    ----------------------------------------------------------
EXTERN_C EXPORT KERNEL
isrFunc_t IsrRegister(uint8_t interrupt, isrFunc_t func);


//
// -- Dump the CPU state
//    ------------------
EXTERN_C EXPORT NORETURN KERNEL
void IsrDumpState(isrRegs_t *regs);


//
// -- The system call handler
//    -----------------------
EXTERN_C EXPORT SYSCALL
void SyscallHandler(isrRegs_t *regs);


//
// -- An interrupt to handle TLB Flushes
//    ----------------------------------
EXTERN_C EXPORT KERNEL
void IpiHandleTlbFlush(UNUSED(isrRegs_t *regs));


//
// -- An interrupt to handle debugger engagement
//    ------------------------------------------
EXTERN_C EXPORT KERNEL
void IpiHandleDebugger(UNUSED(isrRegs_t *regs));


