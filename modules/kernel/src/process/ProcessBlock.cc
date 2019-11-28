//===================================================================================================================
//
// ProcessBlock.cc -- Block a process
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  Note that this function will leave the current process not on any queue.  It is up to the calling procedure to
//  manage the queue that this Process_t structure is left on.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Mar-22  Initial   0.3.2   ADCL  Initial version
//  2019-Nov-27  Initial   0.4.6   ADCL  Reformat for a single spinlock lock
//
//===================================================================================================================


#include "types.h"
#include "lists.h"
#include "process.h"


//
// -- Block the current process
//    -------------------------
EXPORT KERNEL
void ProcessDoBlock(ProcStatus_t reason)
{
    scheduler.currentProcess->status = reason;
    scheduler.currentProcess->pendingErrno = 0;
    ProcessSchedule();
}
