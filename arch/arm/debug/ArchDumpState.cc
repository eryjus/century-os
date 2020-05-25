//===================================================================================================================
//
//  ArchDumpState.cc -- Dump the state of the architecture
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-May-25  Initial   0.7.0a  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "pic.h"
#include "printf.h"
#include "cpu.h"


EXTERN_C EXPORT KERNEL
void ArchDumpState(DumpStates_t states)
{
    // Dump the CPU state no matter what, and do that first
    ArchDumpCpsr();


    //
    // -- Finally, is we are asked to halt, do so
    //    ---------------------------------------
    if (states & DUMP_STOP) {
        PicBroadcastIpi(picControl, IPI_PANIC);
        while(true) {}
    }
}

