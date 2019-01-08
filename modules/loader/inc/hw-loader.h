//===================================================================================================================
//
//  hw.h -- This is the include file for any hardware-specific lcoations
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-13  Initial   0.2.0   ADCL  Initial version
//
//===================================================================================================================


#ifndef __HW_H__
#define __HW_H__


#include "cpu.h"
#include "arch-hw-prevalent.h"
#include "arch-hw-loader.h"


//
// -- Send a message to the mailbox
//    -----------------------------
bool MailboxSend(uint32_t message, uint32_t mailbox);


//
// -- Receive a message from the mailbox
//    ----------------------------------
uint32_t MailboxReceive(uint32_t mailbox);



#endif
