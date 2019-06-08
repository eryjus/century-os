//===================================================================================================================
//
//  MessageVars.cc -- These are the variables for message management
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
#include "message.h"


//
// -- these values control the messages
//    ---------------------------------
__krndata int msgmax = MSGMAX;
__krndata int msgmnb = MSGMNB;
__krndata int msgmni = MSGMNI;


//
// -- This is the global message queue list
//    -------------------------------------
__krndata MessageAll_t messageAll = {0};


