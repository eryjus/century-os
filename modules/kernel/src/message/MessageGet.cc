//===================================================================================================================
//
//  MessageGet.cc -- Get a Message Queue based on the key requested
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


#include "types.h"
#include "spinlock.h"
#include "heap.h"
#include "errno.h"
#include "printf.h"
#include "message.h"


//
// -- Get a message queue
//    -------------------
int __krntext MessageGet(key_t key, int msgflg)
{
    archsize_t flags = SPINLOCK_BLOCK_NO_INT(messageAll.globalLock) {
        int empty = -1;
        int found = (key == IPC_PRIVATE ? -2 : -1);
        int i;
        int rv;

        // -- figure out if we have an empty slot and an existing key
        for (i = 0; i < msgmni; i ++) {
            if (found >= 0 && empty >= 0) break;
            if (messageAll.queues[i] == NULL) {
                if (empty == -1) empty = i;
            } else if (messageAll.queues[i]->key == key) {
                if (found == -1) found = i;
            }
        }

        bool create = (msgflg & IPC_CREAT) != 0;
        bool priv = (key == IPC_PRIVATE);

        kprintf("found: %x; empty: %x\n", found, empty);


        //
        // -- decypher what we want to do
        //    ---------------------------
        if (priv) {
            rv = empty;
        } else {
            if (found >= 0) {
                if (create) {
                    if (msgflg & IPC_EXCL) rv = -EEXIST;
                    else rv = found;
                } else {
                    rv = found;
                }
            } else {
                if (create) {
                    rv = empty;
                } else {
                    rv = -ENOENT;
                }
            }
        }


        //
        // -- the final checks
        //    ----------------
        if (rv == empty && empty < 0) rv = -ENOSPC;
        else if (rv >= 0) {
            MessageQueue_t *queue = NEW(MessageQueue_t);
            if (queue != NULL) {
                kMemSetB(queue, 0, sizeof(MessageQueue_t));

                queue->key = key;
                queue->permissions.mode = (msgflg & 0777);      // low 9 bits are typical linux xrwxrwxrw flags
                queue->msg_qbytes = msgmnb;
                queue->msg_ctime = 0;           // TODO: make this the current time
                ListInit(&queue->sendList.list);
                ListInit(&queue->recvList.list);
                ListInit(&queue->msgList.list);
            }

            messageAll.queues[empty] = queue;

            if (queue == NULL) rv = -ENOMEM;
            else rv = empty;
        }


        //
        // -- return the results
        //    ------------------
        SPINLOCK_RLS_RESTORE_INT(messageAll.globalLock, flags);
        return rv;
    }
}
