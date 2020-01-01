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


#include "loader.h"
#include "hardware.h"


//
// -- This is the device structure that will be used for the loader to access the gpio
//    --------------------------------------------------------------------------------
__ldrdata MailboxDevice_t loaderMailbox = {
    .base = LDR_MAILBOX_BASE,
    .MailboxSend = (void (*)(struct MailboxDevice_t *, archsize_t, archsize_t))PHYS_OF(_MailboxSend),
    .MailboxReceive = (archsize_t (*)(struct MailboxDevice_t *, archsize_t))PHYS_OF(_MailboxReceive),
};


//
// -- This is the device structure that will be used for the kernel to access the gpio
//    --------------------------------------------------------------------------------
__krndata MailboxDevice_t kernelMailbox = {
    .base = KRN_MAILBOX_BASE,
    .MailboxSend = _MailboxSend,
    .MailboxReceive = _MailboxReceive,
};

