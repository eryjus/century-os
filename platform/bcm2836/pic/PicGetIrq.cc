//===================================================================================================================
//
//  PicGetIrq.cc -- Get the current IRQ from the PIC
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


#include "printf.h"
#include "timer.h"
#include "pic.h"


archsize_t _PicGetIrq(PicDevice_t *dev)
{
    if (!dev) return -1;

    int core = 0;

    return MmioRead(dev->base2 + TIMER_IRQ_SOURCE + (core * 4));
}
