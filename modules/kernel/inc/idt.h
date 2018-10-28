//===================================================================================================================
//
// idt.cc -- These are functions related to interrupts
//
// These are function prototypes for interrupts management.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Oct-10  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#ifndef __IDT_H__
#define __IDT_H__


#include "types.h"


//
// -- Set up an IDT gate
//    ------------------
void IdtSetGate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);


//
// -- Build the IDT and populate its gates; initialize the handlers to NULL
//    ---------------------------------------------------------------------
void IdtBuild(void);


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
// -- These functions are the specific Interrupt service routines (before the handler)
//    --------------------------------------------------------------------------------
void IsrInt00(isrRegs_t *regs);
void IsrInt01(isrRegs_t *regs);
void IsrInt02(isrRegs_t *regs);
void IsrInt03(isrRegs_t *regs);
void IsrInt04(isrRegs_t *regs);
void IsrInt05(isrRegs_t *regs);
void IsrInt06(isrRegs_t *regs);
void IsrInt07(isrRegs_t *regs);
void IsrInt08(isrRegs_t *regs);
void IsrInt09(isrRegs_t *regs);
void IsrInt0a(isrRegs_t *regs);
void IsrInt0b(isrRegs_t *regs);
void IsrInt0c(isrRegs_t *regs);
void IsrInt0d(isrRegs_t *regs);
void IsrInt0e(isrRegs_t *regs);
void IsrInt0f(isrRegs_t *regs);
void IsrInt10(isrRegs_t *regs);
void IsrInt11(isrRegs_t *regs);
void IsrInt12(isrRegs_t *regs);
void IsrInt13(isrRegs_t *regs);
void IsrInt14(isrRegs_t *regs);
void IsrInt15(isrRegs_t *regs);
void IsrInt16(isrRegs_t *regs);
void IsrInt17(isrRegs_t *regs);
void IsrInt18(isrRegs_t *regs);
void IsrInt19(isrRegs_t *regs);
void IsrInt1a(isrRegs_t *regs);
void IsrInt1b(isrRegs_t *regs);
void IsrInt1c(isrRegs_t *regs);
void IsrInt1d(isrRegs_t *regs);
void IsrInt1e(isrRegs_t *regs);
void IsrInt1f(isrRegs_t *regs);


#endif
