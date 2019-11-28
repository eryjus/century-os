//===================================================================================================================
//
//  MessageWakeAll.cc -- For the given queue, wake all messages placing them on the Ready Queue
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
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
// -- Wake all messages on the queue, making them all ready and rescheduling on the way out
//    -------------------------------------------------------------------------------------
int __krntext MessageWakeAll(ListHead_t *list)
{
    ProcessLockAndPostpone();

    ListHead_t::List_t *wrk = list->list.next;

    while (wrk != &list->list) {
        ListHead_t::List_t *nxt = wrk->next;
        MsgWaiting_t *ent = FIND_PARENT(wrk, MsgWaiting_t, list);
        ListRemoveInit(wrk);
        ProcessReady(ent->proc);
        FREE(ent);
        wrk = nxt;
    }

    ProcessUnlockAndSchedule();
    return 0;
}
