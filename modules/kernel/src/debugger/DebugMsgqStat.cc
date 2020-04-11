//===================================================================================================================
//
//  DebugMsgqStat.cc -- Dump the message queue list status
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Apr-10  Initial  v0.6.1a  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "msgq.h"
#include "debugger.h"


//
// -- Print an individual Message Queue Stats
//    ---------------------------------------
EXTERN_C HIDDEN KERNEL
void PrintMsgqStats(MessageQueue_t *q)
{
    kprintf(ANSI_ATTR_BOLD);
    DbgSpace(12, kprintf("| %p ", q));
    kprintf(ANSI_ATTR_NORMAL);
    DbgSpace(10, kprintf("| %s ", MsgqStatName(AtomicRead(&q->status))));
    DbgSpace(14, kprintf("| %d ", q->queue.count));
    DbgSpace(12, kprintf("| %d ", ListCount(&q->queue)));
    DbgSpace(14, kprintf("| %d ", q->waiting.count));
    DbgSpace(12, kprintf("| %d ", ListCount(&q->waiting)));
    kprintf("|\n");
}


//
// -- Debug the message queues
//    ------------------------
EXTERN_C EXPORT KERNEL
void DebugMsgqStatus(void)
{
    DebuggerEngage(DIPI_ENGAGE);

    kprintf("+------------+----------+--------------+------------+--------------+------------+\n");

    kprintf("| " ANSI_ATTR_BOLD ANSI_FG_BLUE "Queue Addr" ANSI_ATTR_NORMAL " |  " ANSI_ATTR_BOLD ANSI_FG_BLUE
                "Status" ANSI_ATTR_NORMAL "  | " ANSI_ATTR_BOLD ANSI_FG_BLUE "#Msgs Stated" ANSI_ATTR_NORMAL
                " | " ANSI_ATTR_BOLD ANSI_FG_BLUE "#Msgs Calc" ANSI_ATTR_NORMAL " | " ANSI_ATTR_BOLD ANSI_FG_BLUE
                "#Wait Stated" ANSI_ATTR_NORMAL " | " ANSI_ATTR_BOLD ANSI_FG_BLUE "#Wait Calc" ANSI_ATTR_NORMAL
                " |\n");

    kprintf("+------------+----------+--------------+------------+--------------+------------+\n");

    ListHead_t::List_t *wrk = msgqList.list.next;
    while (wrk != &msgqList.list) {
        PrintMsgqStats(FIND_PARENT(wrk, MessageQueue_t, list));
        wrk = wrk->next;
    }

    kprintf("+------------+----------+--------------+------------+--------------+------------+\n");

    DebuggerRelease();
}


