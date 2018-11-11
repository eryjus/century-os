//===================================================================================================================
//
//  ipc.h -- Structures for Inter-Process Communication (IPC)
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  These are written initially for kernel messaging, but will also be used for user messages with a larger payload.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Oct-27  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#ifndef __IPC_H__
#define __IPC_H__


#include "types.h"
#include "lists.h"
#include "process.h"


//
// -- get the structure definition
//    ----------------------------
#include "ipc-msg.h"


//
// -- Determine if we have a message waiting
//    --------------------------------------
inline bool MessageWaiting(void) { return !IsListEmpty(&procs[currentPID]->messages); }


//
// -- Send a message to another process by PID
//    ----------------------------------------
int MessageSend(PID_t pid, Message_t *msg);


//
// -- Receive a message
//    -----------------
int MessageReceive(Message_t *m);


#endif
