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
#include "hw-loader.h"


//
// -- Send a message to the mailbox
//    -----------------------------
bool MailboxSend(uint32_t message, uint32_t mailbox)
{
    if ((message & 0x0f) != 0 || (mailbox & 0xfffffff0) != 0) return false;

    MailboxWaitReadyToSend();
    message -= 0x40000000;
    MmioWrite(MB_BASE + MB_WRITE, message | mailbox);

    return true;
}
