//===================================================================================================================
//
//  msgq.h -- This is the prototypes for message queues
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  This is the kernel implementation of message queues.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  --------------------------------------------------------------------------
//  2020-Apr-09  Initial  v0.6.1a  ADCL  Initial version
//
//===================================================================================================================


#pragma once


#define __MSGQ_H__

#include "types.h"


//
// -- These are the possible statuses of an individual queue
//    ------------------------------------------------------
typedef enum {
    MSGQ_INITIALIZING = 0,
    MSGQ_ALLOCATED = 1,
    MSGQ_DELETE_PENDING = 2,
} MsgqStatus_t;


//
// -- Get a text description of the status
//    ------------------------------------
EXTERN_C INLINE
const char *MsgqStatName(int s) {
    if (s == MSGQ_INITIALIZING)         return "INIT";
    else if (s == MSGQ_ALLOCATED)       return "ALLOC";
    else if (s == MSGQ_DELETE_PENDING)  return "DLT PEND";
    else                                return "UNKNOWN";
}


//
// -- This is one individual message queue
//    ------------------------------------
typedef struct MessageQueue_t {
    AtomicInt_t status;                         // -- The current status of this queue; backed by MsgqStatus_t
    ListHead_t::List_t list;                    // -- This list entry on the msgqList
    QueueHead_t queue;                          // -- The queue of msg, has its own lock; queue has its own count
    ListHead_t procList;                        // -- This is the list of processes with this queue open
    ListHead_t waiting;                         // -- the list of processes waiting on this queue
} MessageQueue_t;


//
// -- This is list of all message queues
//    ----------------------------------
typedef ListHead_t MessageQueueList_t;          // -- contains its own lock


//
// -- This is an actual message in the queue
//    --------------------------------------
typedef struct Message_t {
    ListHead_t::List_t list;                    // -- how the message is placed in queue
    size_t payloadSize;                         // -- this is the size of the payload (specifically the struct below)
    struct {
        long type;                              // -- Posix message type -- must be positive
        uint8_t data[0];                        // -- Additional data
    } payload;
} Message_t;


//
// -- This is the pointer to the message queue structure
//    --------------------------------------------------
EXTERN EXPORT KERNEL_BSS
MessageQueueList_t msgqList;


//
// -- Initialize the global Message Queue Structures
//    ----------------------------------------------
EXTERN_C EXPORT LOADER
void MessageQueueInit(void);


//
// -- Allocate and create a new message queue
//    ---------------------------------------
EXTERN_C EXPORT KERNEL
MessageQueue_t *MessageQueueCreate(void);


//
// -- Release the reference to this message queue; marking for deletion when the reference count is 0
//    -----------------------------------------------------------------------------------------------
EXTERN_C EXPORT KERNEL
void MessageQueueRelease(MessageQueue_t *msgq);


//
// -- Mark the message queue for deletion
//    -----------------------------------
EXTERN_C INLINE
void MessageQueueDelete(MessageQueue_t *msgq) {
    AtomicSet(&msgq->status, MSGQ_DELETE_PENDING);
    MessageQueueRelease(msgq);
}


//
// -- This is the working function to send a message to a message queue
//    -----------------------------------------------------------------
EXTERN_C EXPORT KERNEL
void _MessageQueueSend(MessageQueue_t *msgq, long type, size_t sz, void *payload, bool lock);


//
// -- This is the normal API for sending a message
//    --------------------------------------------
EXTERN_C INLINE
void MessageQueueSend(MessageQueue_t *msgq, long type, size_t sz, void *payload) {
    _MessageQueueSend(msgq, type, sz, payload, true);
}


//
// -- Receive a message from a message queue, optionally blocking
//    -----------------------------------------------------------
EXTERN_C EXPORT KERNEL
bool MessageQueueReceive(MessageQueue_t *msgq, long *type, size_t sz, void *payload, bool block);


