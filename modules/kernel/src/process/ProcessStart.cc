//===================================================================================================================
//
// ProcessStart.cc -- Perform the startup tasks for a new process
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  This function will be called for every new process in order to make sure that all new processes have
//  the proper initialization completed.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Mar-16  Initial   0.3.2   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "printf.h"
#include "cpu.h"
#include "process.h"


//
// -- complete any new task initialization
//    ------------------------------------
void __krntext ProcessStart(void)
{
    //
    // -- At this point, complete any initialization needed for a new task
    //    ----------------------------------------------------------------
    EnableInterrupts();
}

