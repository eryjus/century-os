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
//  2012-Jul-02                          Initial version
//  2012-Sep-16                          Leveraged from Century
//  2012-Sep-23                          Removed DUMP() define
//  2013-Sep-12   #101                   Resolve issues splint exposes
//  2018-Jun-01  Initial   0.1.0   ADCL  Copied this file from century32 to century-os
//
//===================================================================================================================


#include "types.h"
#include "printf.h"
#include "cpu.h"
#include "idt.h"


void IsrDumpState(isrRegs_t *regs)
{
    kprintf("EAX: %p  EBX: %p  ECX: %p\n", regs->eax, regs->ebx, regs->ecx);
    kprintf("EDX: %p  ESI: %p  EDI: %p\n", regs->edx, regs->esi, regs->edi);
    kprintf("EBP: %p  ESP: %p  SS: %x\n", regs->ebp, regs->esp, regs->ss);
    kprintf("EIP: %p  EFLAGS: %p\n", regs->eip, regs->eflags);
    kprintf("CS: %x  DS: %x  ES: %x  FS: %x  GS: %x\n",
            regs->cs, regs->ds, regs->es, regs->fs, regs->gs);
    kprintf("CR0: %p  CR2: %p  CR3: %p\n", regs->cr0, regs->cr2, regs->cr3);
    kprintf("Trap: %x  Error: %x\n\n", regs->intno, regs->errcode);

    Halt();
}