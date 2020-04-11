//===================================================================================================================
//
//  butler.h -- The butler process
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Apr-10  Initial  v0.6.1b  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "process.h"
#include "msgq.h"


//
// -- The name of the butler process
//    ------------------------------
EXTERN EXPORT KERNEL_DATA
const char *butlerName;


//
// -- This is the message queue the butler will use
//    ---------------------------------------------
EXTERN EXPORT KERNEL_BSS
MessageQueue_t *butlerMsgq;


//
// -- Initialize the Butler and perform the initial cleanup
//    -----------------------------------------------------
EXTERN_C EXPORT KERNEL
void ButlerInit(void);


//
// -- The main butler process, dispatching tasks to complete
//    ------------------------------------------------------
EXTERN_C EXPORT KERNEL NORETURN
void Butler(void);


