//===================================================================================================================
//
//  MsgqInit.cc -- Initialize the Message Queue global structures
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  --------------------------------------------------------------------------
//  2020-Apr-09  Initial  v0.6.1a  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "heap.h"
#include "butler.h"
#include "msgq.h"


//
// -- Initialize the global Message Queue Structures
//    ----------------------------------------------
EXTERN_C EXPORT LOADER
void MessageQueueInit(void)
{
    ListInit(&msgqList.list);
    msgqList.count = 0;
    msgqList.lock = {0};

    // -- create the butler message queue now so it is ready when the first processes are terminated.
    butlerMsgq = MessageQueueCreate();
}

