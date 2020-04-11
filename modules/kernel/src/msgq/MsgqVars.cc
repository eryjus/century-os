//===================================================================================================================
//
//  MsgqVars.cc -- Message Queue variables
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
#include "msgq.h"



//
// -- This is the pointer to the message queue structure
//    --------------------------------------------------
EXPORT KERNEL_BSS
MessageQueueList_t msgqList;


