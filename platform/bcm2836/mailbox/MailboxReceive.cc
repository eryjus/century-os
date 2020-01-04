//===================================================================================================================
//
//  MailboxReceive.cc -- Receive a message from a mailbox
//
//        Copyright (c)  2017-2020 -- Adam Clark
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
//  2019-Feb-15  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "printf.h"
#include "hardware.h"


//
// -- Receive a message from the mailbox
//    ----------------------------------
archsize_t _MailboxReceive(MailboxDevice_t *dev, uint32_t mailbox)
{
    if (!dev) return -1;
    if ((mailbox & 0xfffffff0) != 0) return -1;

    kprintf(".. Preparing to receive data\n");

    while (true) {
        while (MmioRead(dev->base + MB_STATUS) & (1 << 30)) { }

        uint32_t msg = MmioRead(dev->base + MB_READ);

        if ((msg & 0x0f) == mailbox) {
            kprintf(".. Data received\n");
            return ((msg & 0xfffffff0) + 0x40000000);
        }
    }
}
