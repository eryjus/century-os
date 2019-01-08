//===================================================================================================================
//
//  libc.h -- This is the user-side interface to the syscall functions
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-11  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#ifndef __LIBC_H__
#define __LIBC_H__


#include "types.h"
#include "ipc.h"


//
// -- SYSCALL 1: Receive a message
//    ----------------------------
extern "C" int ReceiveMessage(Message_t *msg);


//
// -- SYSCALL 2: Send a message
//    ----------------------------
extern "C" int SendMessage(PID_t pid, Message_t *msg);


#endif
