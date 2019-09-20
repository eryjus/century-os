//===================================================================================================================
//
//  MessageReceive.cc -- The kernel-level implementation of `msgrcv()`
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
//  2019-Jun-07  Initial   0.4.4   ADCL  Initial version
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
// -- Receive a message, optionally blocking if a message is not available
//    --------------------------------------------------------------------
int __krntext MessageReceive(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg)
{
    if (msqid < 0 || msqid >= msgmni) return -EINVAL;
    if (!IS_MAPPED(msgp, sizeof(long) + msgsz)) return -EFAULT;

    MessageQueue_t *queue;
    scheduler.currentProcess->pendingErrno = 0;

    archsize_t flags = SPINLOCK_BLOCK_NO_INT(messageAll.globalLock) {
        queue = messageAll.queues[msqid];
        SPINLOCK_RLS_RESTORE_INT(messageAll.globalLock, flags);
    }

    if (queue == NULL) return -EINVAL;

    while (true) {
        flags = SPINLOCK_BLOCK_NO_INT(messageAll.globalLock) {
            queue = messageAll.queues[msqid];
            if (queue == NULL) {
                SPINLOCK_RLS_RESTORE_INT(queue->lock, flags);
                return -EIDRM;
            }

            SPINLOCK_BLOCK(queue->lock) {
                SPINLOCK_RLS(messageAll.globalLock);

                if (HasReadPermission(perm)) {
                    ListHead_t::List_t *wrk = queue->msgList.list.next;

                    while (wrk != &queue->msgList.list) {
                        Message_t *msg = FIND_PARENT(wrk, Message_t, list);
                        ListHead_t::List_t *nxt = wrk->next;

                        if (msgtyp == 0 || (msgtyp > 0 && msg->msgType == msgtyp) ||
                                (msgtyp < 0 && msg->msgType <= -msgtyp)) {
                            // -- we found a good message, prepare to return it
                            int rv;
                            ListRemoveInit(wrk);
                            MsgUser_t *uMsg = (MsgUser_t *)msgp;
                            uMsg->msgType = msg->msgType;
                            if (msgsz < msg->msgLen) msgsz = msg->msgLen;
                            if (msg->msgLen > 0) kMemMove(msg->mtext, uMsg->mtext, msgsz);
                            rv = msgsz;

                            queue->msg_count --;
                            queue->msg_cbytes -= msg->msgLen;
                            queue->msg_lrpid = GetUid();
                            queue->msg_rtime = 0;       // TODO: implement date/time

                            FREE(msg);
                            MessageWakeAll(&queue->sendList);
                            SPINLOCK_RLS_RESTORE_INT(queue->lock, flags);
                            return rv;
                        }

                        wrk = nxt;
                    }

                    // -- if we get here, there are no messages to receive, so block
                    if ((msgflg & IPC_NOWAIT) != 0) {
                        SPINLOCK_RLS_RESTORE_INT(queue->lock, flags);
                        return -EAGAIN;
                    } else {
                        MsgWaiting_t *ent = NEW(MsgWaiting_t);
                        ListInit(&ent->list);
                        ent->proc = scheduler.currentProcess;
                        ListAddTail(&queue->recvList, &ent->list);
                        SPINLOCK_RLS_RESTORE_INT(queue->lock, flags);
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
                    SPINLOCK_RLS_RESTORE_INT(queue->lock, flags);
                    return -EACCES;
                }
            }
        }
    }
}

