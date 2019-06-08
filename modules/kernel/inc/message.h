//===================================================================================================================
//
//  message.h -- The message low-level implementation for Century-OS
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-May-15  Initial   0.4.4   ADCL  Initial version
//
//===================================================================================================================


#ifndef __MESSAGE_H__
#define __MESSAGE_H__


#include "sys/msg.h"
#include "atomic.h"
#include "spinlock.h"
#include "lists.h"


//
// == These values are used by POSIX and will need to be defined; to be configured at boot later
//    ==========================================================================================

// -- Max message size (may be obsolete...??)
#define MSGMAX      8192
extern int msgmax;

// -- maximum number of bytes on any given message queue
#define MSGMNB      16384
extern int msgmnb;

// -- maximum number of system-wide message queues
#define MSGMNI      32000
extern int msgmni;


//
// -- Basic message structure passed in from user space
//    -------------------------------------------------
typedef struct MsgUser_t {
    long msgType;
    unsigned char mtext[0];
} MsgUser_t;


//
// -- The actual internal message
//    ---------------------------
typedef struct Message_t {
    ListHead_t::List_t list;
    long msgType;
    size_t msgLen;
    unsigned char mtext[0];
} Message_t;


//
// -- This is a structure to hold the processes that need to be woken up
//    ------------------------------------------------------------------
typedef struct MsgWaiting_t {
    ListHead_t::List_t list;
    Process_t *proc;
} MsgWaiting_t;


//
// -- Message Queues
//    --------------
typedef struct MessageQueue_t {
    key_t key;
    Spinlock_t lock;
    ListHead_t sendList;
    ListHead_t recvList;
    ListHead_t msgList;
    struct ipc_perm permissions;
    size_t msg_count;
    msglen_t msg_qbytes;
    msglen_t msg_cbytes;
    PID_t msg_lspid;
    PID_t msg_lrpid;
    time_t msg_stime;
    time_t msg_rtime;
    time_t msg_ctime;
} MessageQueue_t;


//
// -- This is the internal structure for all message queues
//    -----------------------------------------------------
typedef struct MessageAll_t {
    Spinlock_t globalLock;
    MessageQueue_t **queues;
} MessageAll_t;


//
// -- all message queues
//    ------------------
extern MessageAll_t messageAll;

//
// -- Perform the message subsystem initialization
//    --------------------------------------------
__CENTURY_FUNC__ void MessageInit(void);


//
// -- Get a message queue
//    -------------------
__CENTURY_FUNC__ int MessageGet(key_t key, int msgflg);


//
// -- Get the current IPC Status information -- caller holds the lock on queue
//    ------------------------------------------------------------------------
__CENTURY_FUNC__ int MsqIpcStat(MessageQueue_t *queue, struct msqid_ds *msqid_ds);


//
// -- Set the IPC Status information -- caller holds the lock on queue
//    ----------------------------------------------------------------
__CENTURY_FUNC__ int MsqIpcSet(MessageQueue_t *queue, struct msqid_ds *msqid_ds);


//
// -- Remove a message queue -- caller holds the lock on queue
//    --------------------------------------------------------
__CENTURY_FUNC__ int MsqRemove(MessageQueue_t *queue, int msqid);


//
// -- Wake all processes on the queue
//    -------------------------------
__CENTURY_FUNC__ int MessageWakeAll(ListHead_t *list);


//
// -- Send a message, optionally blocking if no space is available
//    ------------------------------------------------------------
__CENTURY_FUNC__ int MessageSend(int msqid, void *msgp, size_t msgsz, int msgflg);


//
// -- Receive a message, optionally blocking if a message is not available
//    --------------------------------------------------------------------
__CENTURY_FUNC__ int MessageReceive(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg);


#endif
