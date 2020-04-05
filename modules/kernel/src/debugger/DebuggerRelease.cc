//===================================================================================================================
//
//  DebuggerRelease.cc -- Release the other cores from a stopped state
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Apr-03  Initial  v0.6.0a  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "pic.h"
#include "debugger.h"


//
// -- Release the other cores from a stopped state
//    --------------------------------------------
EXTERN_C EXPORT KERNEL
void DebuggerRelease(void)
{
    AtomicSet(&debugCommunication.coresEngaged, 0);
    RestoreInterrupts(debugCommunication.debuggerFlags);
}
