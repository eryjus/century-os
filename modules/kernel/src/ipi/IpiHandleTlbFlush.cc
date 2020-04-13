//===================================================================================================================
//
//  IpiHandleTlbFlush.cc -- Handle the actual TLB flush for an address
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Feb-24  Initial  v0.3.0h  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "mmu.h"
#include "pic.h"
#include "process.h"
#include "printf.h"

//
// -- Handle the actual TLB flush, waiting for the address to flush
//    -------------------------------------------------------------
EXTERN_C EXPORT KERNEL
void IpiHandleTlbFlush(isrRegs_t *regs)
{
    // -- wait for the address to be given to the CPU
    while (tlbFlush.addr == (archsize_t)-1) {}

#if DEBUG_ENABLED(IpiHandleTlbFlush)
    kprintf("Flushing TLB on CPU %d\n", thisCpu->cpuNum);
#endif

    InvalidatePage(tlbFlush.addr);
    AtomicDec(&tlbFlush.count);
    PicEoi(picControl, (Irq_t)0);
}


