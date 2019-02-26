//===================================================================================================================
//
//  MailboxSend.cc -- SEnd a message to a mailbox
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  Please note that this function will perform the adjustment between ARM/VC address space.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Jan-05  Initial   0.2.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "printf.h"
#include "hardware.h"


//
// -- Send a message to the mailbox
//    -----------------------------
void _MailboxSend(MailboxDevice_t *dev, archsize_t mb, archsize_t msg)
{
    kprintf("Checking dev..\n");
    if (!dev) return;
    kprintf("Checking msg..\n");
    if ((msg & 0x0f) != 0) return;
    kprintf("Checking mb..\n");
    if ((mb & 0xfffffff0) != 0) return;
    kprintf(".. Preparing to send data...\n");

    while (MmioRead(dev->base + MB_STATUS) & (1 << 31)) { }

    msg -= 0x40000000;
    MmioWrite(dev->base + MB_WRITE, msg | mb);

    kprintf(".. Data Sent...\n");
}
