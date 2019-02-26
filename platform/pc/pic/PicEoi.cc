//===================================================================================================================
//
//  PicEoi.cc -- Issue an end of interrupt to the interrupt controller
//
//        Copyright (c)  2017-2019 -- Adam Clark
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
#include "printf.h"
#include "hardware.h"
#include "pic.h"


//
// -- Issue an EOI for the timer
//    --------------------------
void _PicEoi(PicDevice_t *dev, int irq)
{
    if (!dev) return;

    if (irq >= 8) outb(dev->base2 + PIC_SLAVE_COMMAND, 0x20);
    outb(dev->base1 + PIC_MASTER_COMMAND, 0x20);
}

