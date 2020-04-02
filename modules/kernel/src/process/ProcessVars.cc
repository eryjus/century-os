//===================================================================================================================
//
// ProcessVars.cc -- Global variables for process management
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Oct-14  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "process.h"


//
// -- This is the scheduler object
//    ----------------------------
EXPORT KERNEL_DATA
Scheduler_t scheduler = {
    0,                      // nextPID
    0xffffffffffffffff,     // nextWake
    false,                  // processChangePending
    0,                      // flags
    {0},                    // schedulerLockCount
    {0},                    // postponeCount
    -1,                     // lock CPU
    {{&scheduler.queueOS.list, &scheduler.queueOS.list}, {0}, 0},                   // the os ready queue
    {{&scheduler.queueHigh.list, &scheduler.queueHigh.list}, {0}, 0},               // the high ready queue
    {{&scheduler.queueNormal.list, &scheduler.queueNormal.list}, {0}, 0},           // the normal ready queue
    {{&scheduler.queueLow.list, &scheduler.queueLow.list}, {0}, 0},                 // the low ready queue
    {{&scheduler.queueIdle.list, &scheduler.queueIdle.list}, {0}, 0},               // the idle ready queue
    {{&scheduler.listBlocked.list, &scheduler.listBlocked.list}, {0}, 0},           // the list of blocked processes
    {{&scheduler.listSleeping.list, &scheduler.listSleeping.list}, {0}, 0},         // the list of sleeping processes
    {{&scheduler.listTerminated.list, &scheduler.listTerminated.list}, {0}, 0},     // the list of terminated tasks
};


Spinlock_t schedulerLock = {0};
