//===================================================================================================================
//
//  IsrHandler.cc -- The common ISR handler routine
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  All ISRs are handled by a common service program.  This is it.  But it is currently a stub.
//
//  IRQ 0-63 are for the normal IRQs.  Then, there are 8 additional IRQs (64-71) for some additional interrupts.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Dec-06  Initial   0.2.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "printf.h"
#include "interrupt.h"


//
// -- The ISR Handler Table
//    ---------------------
isrFunc_t isrHandlers[256] = {NULL_ISR};


//
// -- This is the common ISR Handler entry routine
//    --------------------------------------------
void IsrHandler(isrRegs_t *regs)
{
    int intno = 0;
    int i;

    // -- Here we need to determine the intno for the ISR
    uint32_t pending = MmioRead(INT_IRQPEND0);
    kprintf("ISR Pending: %p\n", pending);

    if (pending & INTPND0IRQ62) {
        intno = 62;
    } else if (pending & INTPND0IRQ57) {
        intno = 57;
    } else if (pending & INTPND0IRQ56) {
        intno = 56;
    } else if (pending & INTPND0IRQ55) {
        intno = 55;
    } else if (pending & INTPND0IRQ54) {
        intno = 54;
    } else if (pending & INTPND0IRQ53) {
        intno = 53;
    } else if (pending & INTPND0IRQ19) {
        intno = 19;
    } else if (pending & INTPND0IRQ18) {
        intno = 18;
    } else if (pending & INTPND0IRQ10) {
        intno = 10;
    } else if (pending & INTPND0IRQ9) {
        intno = 9;
    } else if (pending & INTPND0IRQ7) {
        intno = 7;
    } else if (pending & INTPND0IRQREG2) {
        pending = MmioRead(INT_IRQPEND2);

        for (i = 0; i < 32; i ++) {
            if (pending & (1 << i)) {
                intno = 32 + i;
                goto process;
            }
        }

        return;
    } else if (pending & INTPND0IRQREG1) {
        pending = MmioRead(INT_IRQPEND2);

        for (i = 0; i < 32; i ++) {
            if (pending & (1 << i)) {
                intno = i;
                goto process;
            }
        }

        return;
    } else if (pending & INTPND0IRQILL0) {
        intno = 71;
    } else if (pending & INTPND0IRQILL1) {
        intno = 70;
    } else if (pending & INTPND0IRQGPUH1) {
        intno = 69;
    } else if (pending & INTPND0IRQGPUH0) {
        intno = 68;
    } else if (pending & INTPND0IRQDOORB1) {
        intno = 67;
    } else if (pending & INTPND0IRQDOORB0) {
        intno = 66;
    } else if (pending & INTPND0IRQMAIL) {
        intno = 65;
    } else if (pending & INTPND0IRQTIMER) {
        intno = 64;
    } else {
        kprintf("ERROR: Unable to determine IRQ number: %p\n", pending);
        return;
    }

process:
    kprintf("Determined IRQ to be %x\n", intno);
    if (isrHandlers[intno] != NULL) {
        isrFunc_t handler = isrHandlers[intno];
        kprintf("Executing interrupt %x at %p\n", intno, handler);
        handler(regs);
    } else {
        // TODO: this needs to get fixed!
        if (intno == 57) {
            // -- clear any pending interrupt
            MmioWrite(UART_BASE + UART_ICR, 0xffffffff);
        } else kprintf("Unhandled Interrupt #%x\n", intno);
    }
}
