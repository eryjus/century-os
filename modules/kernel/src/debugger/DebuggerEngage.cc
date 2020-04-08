//===================================================================================================================
//
//  DebuggerEngage.cc -- Signal the cores to stop processing until we release them
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
// -- Signal the other cores to stop and wait for confirmation that they have
//    -----------------------------------------------------------------------
EXTERN_C EXPORT KERNEL
void DebuggerEngage(DbgIniCommand_t cmd)
{
    debugCommunication.debuggerFlags = DisableInterrupts();
    AtomicSet(&debugCommunication.coresEngaged, 1);
    debugCommunication.command = cmd;
    PicBroadcastIpi(picControl, IPI_DEBUGGER);
    while (AtomicRead(&debugCommunication.coresEngaged) != cpus.cpusRunning) {}
}
