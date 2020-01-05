//===================================================================================================================
//
//  PicEoi.cc -- Issue an end of interrupt to the interrupt controller
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Feb-24  Initial   0.3.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "hardware.h"
#include "pic.h"


//
// -- Issue an EOI for the timer
//    --------------------------
EXTERN_C EXPORT KERNEL
void _PicEoi(PicDevice_t *dev, Irq_t irq)
{
    if (!dev) return;

    if (irq >= 8) outb(PIC2 + PIC_SLAVE_COMMAND, 0x20);
    outb(PIC1 + PIC_MASTER_COMMAND, 0x20);
}

