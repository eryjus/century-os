//===================================================================================================================
//
//  ButlerInit.cc -- This is the main butler process
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
#include "msgq.h"
#include "butler.h"


//
// -- The main butler process, dispatching tasks to complete
//    ------------------------------------------------------
EXTERN_C EXPORT KERNEL NORETURN
void Butler(void)
{
    long msgt;
//    kprintf("Assuming the Butler role!\n");

    ButlerInit();
//    kprintf(".. Butler Initialization complete\n");

    while (true) {
        // -- block until we have something to do
        MessageQueueReceive(butlerMsgq, &msgt, 0, 0, true);
//        kprintf("Have a message to process...\n");

        switch (msgt) {
        case BUTLER_CLEAN_PMM:
            ButlerCleanPmm();
            break;

        case BUTLER_CLEAN_PROCESS:
            ButlerCleanProcess();
            break;

        default:
            assert(false);
            break;
        }
    }
}

