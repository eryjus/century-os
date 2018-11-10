//===================================================================================================================
//
// ipc.h -- Structures for Inter-Process Communication (IPC)
//
// These are written initially for kernel messaging, but will also be used for user messages with a larger payload.
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
// -- This is a kernel message.  The dataPayload member is not used by the kernel but might be with user messages
//    -----------------------------------------------------------------------------------------------------------
typedef struct Message_t {
    ListHead_t::List_t list;
    uint32_t msg;
    uint32_t parm1;
    uint32_t parm2;
    PID_t pid;
    size_t payloadSize;
    void *dataPayload;
} Message_t;


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
