//===================================================================================================================
//
//  IsrDumpState.cc -- For exceptions, dump the current state of the processor from the registers
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-30  Initial   0.2.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "printf.h"
#include "cpu.h"
#include "interrupt.h"


void IsrDumpState(isrRegs_t *regs)
{
    kprintf("At address: %p\n", regs);
    kprintf(" R0: %p   R1: %p   R2: %p\n", regs->r0, regs->r1, regs->r2);
    kprintf(" R3: %p   R4: %p   R5: %p\n", regs->r3, regs->r4, regs->r5);
    kprintf(" R6: %p   R7: %p   R8: %p\n", regs->r6, regs->r7, regs->r8);
    kprintf(" R9: %p  R10: %p  R11: %p\n", regs->r9, regs->r10, regs->r11);
    kprintf("R12: %p   SP: %p   LR_ret: %p\n", regs->r12, regs->sp_svc, regs->lr_ret);
    kprintf("SPSR_ret: %p     type: %x\n", regs->spsr_ret, regs->type);
    kprintf("\nAdditional Data Points:\n");
    kprintf("User LR: %p  User SP: %p\n", regs->lr_usr, regs->sp_usr);
    kprintf("Svc LR: %p\n", regs->lr_svc);
}