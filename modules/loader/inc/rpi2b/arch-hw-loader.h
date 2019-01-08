//===================================================================================================================
//
//  arch-hw-loader.h -- This is the include file for any hardware-specific locations for the loader
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


#ifndef __HW_H__
#   error "Do not include 'arch-hw-loader.h' directly.  Include 'hw.h' to pick up this file."
#endif


//
// -- Busy-wait for the mailbox system to be ready to send
//    ----------------------------------------------------
inline void MailboxWaitReadyToSend(void) { while (MmioRead(MB_BASE + MB_STATUS) & (1 << 31)) { } }


//
// -- Busy-wait for the mailbox system to be ready to receive data
//    ------------------------------------------------------------
inline void MailboxWaitReadyToReceive(void) { while (MmioRead(MB_BASE + MB_STATUS) & (1 << 30)) { } }

