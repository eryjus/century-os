//===================================================================================================================
//
//  MessageSend.cc -- The kernel-level implementation of `msgsnd()`
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  Programmer's note:
//  While I use hex numbers quite a bit to express bit flags, this file contains several octal numbers.  Be aware
//  of this fact as you are reading this source.
//
//  POSIX variances:
//  A) Permissions are not yet implemented.  Therefore, all processes have all permissions.
//  B) Times are not yet maintained and will all be reported as 0.
//  C) Linux departs from the POSIX standard for `msgsnd()` in the following way:
//     * POSIX measures the total number of system-wide messages and limits the number of messages on all the
//       queues based on this system-wide limit.
//     * Linux measures the total number of messages on any given queue and compares that to the number of bytes
//       allowed on that queue -- which basically treats any 0-length messages as having a minimum of one byte.
//     Since CenturyOS is not going to have a system-imposed total message count limit, the latter (Linux) method
//     will work better for this implementation.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-May-16  Initial   0.4.4   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "errno.h"
#include "mmu.h"
#include "process.h"
#include "user-group.h"
#include "heap.h"
#include "message.h"


//
// -- Send a message, optionally blocking if no space is available
//    ------------------------------------------------------------
int __krntext MessageSend(int msqid, void *msgp, size_t msgsz, int msgflg)
{
    if (msqid < 0 || msqid >= msgmni) return -EINVAL;
    if (!IS_MAPPED(msgp, sizeof(long) + msgsz)) return -EFAULT;

    MessageQueue_t *queue;
    scheduler.currentProcess->pendingErrno = 0;

    SPIN_BLOCK(messageAll.globalLock) {
        queue = messageAll.queues[msqid];
        SPIN_RLS(queue->lock);
    }

    if (queue == NULL) return -EINVAL;

    while (true) {
        SPIN_BLOCK(messageAll.globalLock) {
            queue = messageAll.queues[msqid];
            if (queue == NULL) {
                SPIN_RLS(queue->lock);
                return -EIDRM;
            }

            SPIN_BLOCK(queue->lock) {
                SPIN_RLS(messageAll.globalLock);

                if (HasWritePermission(perm)) {
                    if (queue->msg_cbytes + msgsz > queue->msg_qbytes || queue->msg_count >= queue->msg_qbytes) {
                        // -- here, the queue is considered full
                        if ((msgflg & IPC_NOWAIT) != 0) {
                            SPIN_RLS(queue->lock);
                            return -EAGAIN;
                        } else {
                            MsgWaiting_t *ent = NEW(MsgWaiting_t);
                            ListInit(&ent->list);
                            ent->proc = scheduler.currentProcess;
                            ListAddTail(&queue->sendList, &ent->list);
                            SPIN_RLS(queue->lock);
                            ProcessBlock(PROC_MSGW);

                            // -- check for a pending error
                            if (scheduler.currentProcess->pendingErrno != 0) {
                                int rv = scheduler.currentProcess->pendingErrno;
                                scheduler.currentProcess->pendingErrno = 0;
                                return rv;
                            }

                            continue;
                        }
                    } else {
                        // -- there is room; add the message
                        MsgUser_t *msgUser = (MsgUser_t *)msgp;
                        Message_t *message = (Message_t *)HeapAlloc(sizeof(long) + msgsz, false);
                        ListInit(&message->list);
                        message->msgType = msgUser->msgType;
                        message->msgLen = msgsz;
                        if (msgsz > 0) kMemMove(message->mtext, msgUser->mtext, msgsz);

                        SPIN_BLOCK(queue->msgList.lock) {
                            ListAddTail(&queue->msgList, &message->list);
                            queue->msgList.count ++;
                            SPIN_RLS(queue->msgList.lock);
                        }

                        queue->msg_count ++;
                        queue->msg_cbytes += msgsz;
                        queue->msg_lspid = GetUid();
                        queue->msg_stime = 0;       // TODO: implement date/time

                        MessageWakeAll(&queue->recvList);
                        SPIN_RLS(queue->lock);
                        return 0;
                    }
                } else {
                    SPIN_RLS(queue->lock);
                    return -EACCES;
                }
            }
        }
    }
}

