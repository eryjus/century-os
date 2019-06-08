//===================================================================================================================
//
//  MessageInit.cc -- Initialize the Message system
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
#include "message.h"


//
// -- Perform the message subsystem initialization
//    --------------------------------------------
void __krntext MessageInit(void)
{
    //
    // -- Allocate an array from the kernel heap and initialize it to NULL
    //    ----------------------------------------------------------------
    messageAll.queues = (MessageQueue_t **)HeapAlloc(msgmni * sizeof(MessageQueue_t *), false);
    kMemSetB(messageAll.queues, 0, msgmni * sizeof(MessageQueue_t *));
}

