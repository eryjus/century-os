//===================================================================================================================
//
//  ProcessIdle.cc -- This is an idle process to use on the CPU when there is nothing else to do
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Mar-27  Initial  v0.5.1a  ADCL  Initial version
//
//===================================================================================================================


#include "cpu.h"
#include "heap.h"
#include "timer.h"
#include "process.h"


//
// -- Idle when there is nothing to do
//    --------------------------------
EXTERN_C EXPORT KERNEL
void ProcessIdle(void)
{
    currentThread->priority = PTY_IDLE;

    while (true) {
        assert(currentThread->status == PROC_RUNNING);
        EnableInterrupts();
        HaltCpu();
    }
}

