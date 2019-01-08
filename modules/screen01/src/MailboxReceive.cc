//===================================================================================================================
//
//  MailboxReceive.cc -- Receive a message from a mailbox
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Jan-05  Initial   0.2.0   ADCL  Initial version
//
//===================================================================================================================


#include "screen01.h"


//
// -- Receive a message from the mailbox
//    ----------------------------------
uint32_t MailboxReceive(uint32_t mailbox)
{
    if ((mailbox & 0xfffffff0) != 0) return (uint32_t)-1;

    while (true) {
        MailboxWaitReadyToReceive();
        uint32_t msg = MmioRead(MB_BASE + MB_READ);

        if ((msg & 0x0f) == mailbox) return (msg & 0xfffffff0);
    }
}
