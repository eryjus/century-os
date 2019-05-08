//===================================================================================================================
//
// ProcessEnterPostpone.cc -- Enter a section where any schedule changes will be postponed.
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Mar-18  Initial   0.3.2   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "lists.h"
#include "timer.h"
#include "spinlock.h"
#include "process.h"


//
// -- increase the lock count on the scheduler
//    ----------------------------------------
void __krntext ProcessEnterPostpone(void)
{
    DisableInterrupts();
    AtomicInc(&scheduler.schedulerLockCount);
    kprintf("^(%x)", AtomicRead(&scheduler.schedulerLockCount));
    CLEAN_SCHEDULER();
}

