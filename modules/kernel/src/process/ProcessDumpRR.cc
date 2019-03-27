//===================================================================================================================
//
// ProcessDumpRR.cc -- Dump the Round Robin Queue
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Mar-26  Initial   0.3.2   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "lists.h"
#include "process.h"



//
// -- Dump the Round Robin Queue
//    --------------------------
void __krntext ProcessDumpRR(void)
{
    kprintf("Dumping the roundRobin queue:\n");
    ListHead_t::List_t *list = roundRobin.list.next;
    while (list != &roundRobin.list) {
        ListHead_t::List_t *next = list->next;      // must be saved before it is changed below
        Process_t *wrk = FIND_PARENT(list, Process_t, stsQueue);

        kprintf(".. pid %x at address %p\n", wrk->pid, wrk);

        list = next;
    }
}
