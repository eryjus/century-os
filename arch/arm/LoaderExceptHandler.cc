//===================================================================================================================
//
//  LoaderExceptHandler.cc -- For exceptions, dump the current state of the processor from the registers (loader)
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-30  Initial   0.2.0   ADCL  Initial version
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "loader.h"
#include "types.h"
#include "serial.h"
#include "cpu.h"
#include "interrupt.h"


extern const char *causes[];


__CENTURY_FUNC__ void LoaderExceptHandler(isrRegs_t *regs);


void __ldrtext LoaderExceptHandler(isrRegs_t *regs)
{
    switch(regs->type) {
    case 0x1b: LoaderSerialPutS("UNDEFINED"); break;
    default: LoaderSerialPutS("SVC"); break;
    case 0x17: LoaderSerialPutS("Data Abort"); break;
    case 0x18: LoaderSerialPutS("Pre-Fetch Abort"); break;
    case 0x12: LoaderSerialPutS("IRQ"); break;
    case 0x11: LoaderSerialPutS("FIQ"); break;
    }

    LoaderSerialPutS(" Exception:\n");

    if (regs->type == 0x18) {
        LoaderSerialPutS("The faulting address is: "); LoaderSerialPutHex(READ_IFAR()); LoaderSerialPutChar('\n');
        LoaderSerialPutS("The faulting status is : "); LoaderSerialPutHex(READ_IFSR()); LoaderSerialPutChar('\n');
    }

    LoaderSerialPutS("Register structure at: "); LoaderSerialPutHex((uint32_t)regs); LoaderSerialPutChar('\n');

    LoaderSerialPutS(" R0: "); LoaderSerialPutHex(regs->r0);
            LoaderSerialPutS("  R1: "); LoaderSerialPutHex(regs->r1);
            LoaderSerialPutS("  R2: "); LoaderSerialPutHex(regs->r2); LoaderSerialPutChar('\n');

    LoaderSerialPutS(" R3: "); LoaderSerialPutHex(regs->r3);
            LoaderSerialPutS("  R4: "); LoaderSerialPutHex(regs->r4);
            LoaderSerialPutS("  R5: "); LoaderSerialPutHex(regs->r5); LoaderSerialPutChar('\n');

    LoaderSerialPutS(" R6: "); LoaderSerialPutHex(regs->r6);
            LoaderSerialPutS("  R7: "); LoaderSerialPutHex(regs->r7);
            LoaderSerialPutS("  R8: "); LoaderSerialPutHex(regs->r8); LoaderSerialPutChar('\n');

    LoaderSerialPutS(" R9: "); LoaderSerialPutHex(regs->r9);
            LoaderSerialPutS(" R10: "); LoaderSerialPutHex(regs->r10);
            LoaderSerialPutS(" R11: "); LoaderSerialPutHex(regs->r11); LoaderSerialPutChar('\n');

    LoaderSerialPutS("R12: "); LoaderSerialPutHex(regs->r12);
            LoaderSerialPutS("  SP: "); LoaderSerialPutHex(regs->sp_svc);
            LoaderSerialPutS("  LR_ret: "); LoaderSerialPutHex(regs->lr_ret); LoaderSerialPutChar('\n');



//    kprintf("SPSR_ret: %p     type: %x\n", regs->spsr_ret, regs->type);
//    kprintf("\nAdditional Data Points:\n");
//    kprintf("User LR: %p  User SP: %p\n", regs->lr_usr, regs->sp_usr);
//    kprintf("Svc LR: %p\n", regs->lr_svc);

    while (1) HaltCpu();
}