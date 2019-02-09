//===================================================================================================================
//
//  ipc-msg.h -- Structures for Inter-Process Communication (IPC)
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
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#ifndef __IPC_H__
#   error "Do not include 'ipc-msg.h' directly.  Instead include 'ipc.h'."
#endif


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

