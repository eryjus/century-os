//===================================================================================================================
//
//  arch-interrupt.cc -- These are functions related to interrupts for the i686 architecture
//
//        Copyright (c)  2017-2019 -- Adam Clark
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
// -- Set up an IDT gate
//    ------------------
void IdtSetGate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);


//
// -- Build the IDT and populate its gates; initialize the handlers to NULL
//    ---------------------------------------------------------------------
void IdtBuild(void);


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


//
// -- A Local prototype to prevent the compiler from name mangling
//    ------------------------------------------------------------
extern "C" void IsrHandler(isrRegs_t regs);


//
// -- These are some macros to assist in the system calls handling
//    ------------------------------------------------------------
#define SYSCALL_FUNC_NO(regs)           ((regs)->eax)
#define SYSCALL_RETURN(regs)            ((regs)->eax)
#define SYSCALL_RCVMSG_PARM1(regs)      ((regs)->edi)
#define SYSCALL_SNDMSG_PARM1(regs)      ((regs)->edx)
#define SYSCALL_SNDMSG_PARM2(regs)      ((regs)->edi)

