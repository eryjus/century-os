//===================================================================================================================
//
//  MailboxVars.cc -- These are the variables for the BCM2835 Mailboxes
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


//
// -- This is the device structure that will be used for the kernel to access the gpio
//    --------------------------------------------------------------------------------
EXPORT KERNEL_DATA
MailboxDevice_t kernelMailbox = {
    .base = KRN_MAILBOX_BASE,
    .MailboxSend = _MailboxSend,
    .MailboxReceive = _MailboxReceive,
};

