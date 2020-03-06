//===================================================================================================================
//
//  arch-interrupt.cc -- These are functions related to interrupts for the i686 architecture
//
//        Copyright (c)  2017-2020 -- Adam Clark
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
//  2019-Feb-09  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#pragma once


#ifndef __INTERRUPT_H__
#   error "Do not include 'arch-interrupt.h' directly.  Include 'interrupt.h' and this file will be included"
#endif


#include "types.h"


//
// -- Set up an IDT gate
//    ------------------
EXTERN_C EXPORT KERNEL
void ArchIdtSetGate(uint8_t num, archsize_t base, archsize_t sel, uint8_t flags);


//
// -- These functions are the specific Interrupt service routines (before the handler)
//    --------------------------------------------------------------------------------
EXTERN_C EXPORT KERNEL
void IsrInt00(isrRegs_t *regs);

EXTERN_C EXPORT KERNEL
void IsrInt01(isrRegs_t *regs);

EXTERN_C EXPORT KERNEL
void IsrInt02(isrRegs_t *regs);

EXTERN_C EXPORT KERNEL
void IsrInt03(isrRegs_t *regs);

EXTERN_C EXPORT KERNEL
void IsrInt04(isrRegs_t *regs);

EXTERN_C EXPORT KERNEL
void IsrInt05(isrRegs_t *regs);

EXTERN_C EXPORT KERNEL
void IsrInt06(isrRegs_t *regs);

EXTERN_C EXPORT KERNEL
void IsrInt07(isrRegs_t *regs);

EXTERN_C EXPORT KERNEL
void IsrInt08(isrRegs_t *regs);

EXTERN_C EXPORT KERNEL
void IsrInt09(isrRegs_t *regs);

EXTERN_C EXPORT KERNEL
void IsrInt0a(isrRegs_t *regs);

EXTERN_C EXPORT KERNEL
void IsrInt0b(isrRegs_t *regs);

EXTERN_C EXPORT KERNEL
void IsrInt0c(isrRegs_t *regs);

EXTERN_C EXPORT KERNEL
void IsrInt0d(isrRegs_t *regs);

EXTERN_C EXPORT KERNEL
void IsrInt0e(isrRegs_t *regs);

EXTERN_C EXPORT KERNEL
void IsrInt0f(isrRegs_t *regs);

EXTERN_C EXPORT KERNEL
void IsrInt10(isrRegs_t *regs);

EXTERN_C EXPORT KERNEL
void IsrInt11(isrRegs_t *regs);

EXTERN_C EXPORT KERNEL
void IsrInt12(isrRegs_t *regs);

EXTERN_C EXPORT KERNEL
void IsrInt13(isrRegs_t *regs);

EXTERN_C EXPORT KERNEL
void IsrInt14(isrRegs_t *regs);

EXTERN_C EXPORT KERNEL
void IsrInt15(isrRegs_t *regs);

EXTERN_C EXPORT KERNEL
void IsrInt16(isrRegs_t *regs);

EXTERN_C EXPORT KERNEL
void IsrInt17(isrRegs_t *regs);

EXTERN_C EXPORT KERNEL
void IsrInt18(isrRegs_t *regs);

EXTERN_C EXPORT KERNEL
void IsrInt19(isrRegs_t *regs);

EXTERN_C EXPORT KERNEL
void IsrInt1a(isrRegs_t *regs);

EXTERN_C EXPORT KERNEL
void IsrInt1b(isrRegs_t *regs);

EXTERN_C EXPORT KERNEL
void IsrInt1c(isrRegs_t *regs);

EXTERN_C EXPORT KERNEL
void IsrInt1d(isrRegs_t *regs);

EXTERN_C EXPORT KERNEL
void IsrInt1e(isrRegs_t *regs);

EXTERN_C EXPORT KERNEL
void IsrInt1f(isrRegs_t *regs);

EXTERN_C EXPORT KERNEL
void ArchIntNone(isrRegs_t *regs);

//
// -- A Local prototype to prevent the compiler from name mangling
//    ------------------------------------------------------------
EXTERN_C EXPORT KERNEL
void IsrHandler(isrRegs_t regs);


//
// -- These are some macros to assist in the system calls handling
//    ------------------------------------------------------------
#define SYSCALL_FUNC_NO(regs)           ((regs)->eax)
#define SYSCALL_RETURN(regs)            ((regs)->eax)
#define SYSCALL_RCVMSG_PARM1(regs)      ((regs)->edi)
#define SYSCALL_SNDMSG_PARM1(regs)      ((regs)->edx)
#define SYSCALL_SNDMSG_PARM2(regs)      ((regs)->edi)

