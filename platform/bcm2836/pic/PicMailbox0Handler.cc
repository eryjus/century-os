//===================================================================================================================
//
//  PicMailboxHandler0.cc -- Handle a mailbox0 interrupt, decoding it and acking it
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Mar-01  Initial  v0.5.0h  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "printf.h"
#include "pic.h"


//
// -- Decode and handle a mailbox0 interrupt
//    --------------------------------------
EXTERN_C EXPORT KERNEL
void PicMailbox0Handler(UNUSED(isrRegs_t *))
{
    int msg = MmioRead(IPI_MAILBOX_ACK + ((thisCpu->cpuNum) * 0x10));
    MbHandler_t handler = NULL;

    AtomicInc(&mb0Resp);

    if (msg < 0 || msg >= MAX_IPI) goto exit;

    handler = mbHandlers[msg];
    if (handler == NULL) {
        kprintf("PANIC: Unhandled Mailbox message %d\n", msg);
        CpuPanicPushRegs("");
    }

    handler(NULL);

exit:
    MmioWrite(IPI_MAILBOX_ACK + (thisCpu->cpuNum * 0x10), 0xffffffff);
}

