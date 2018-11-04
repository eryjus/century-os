//===================================================================================================================
//
// ProcessNewPID.cc -- Get a new PID from the process table
//
// The table can really be full.  So we will start with the last allocated PID number and check the next PID.
// As long as we do not have an empty PID, we will continue and restart at the beginning if needed until we find
// an open PID to use.  If we do not find anything we will issue an error message and end the system.  This failure
// message will need to be cleaned up (Redmine #364).
//
// Additionally, we may be able to still allocate a new PID if the reaper queue has something to process.  If this
// is the case, we will want to signal the Butler process to hurry up and get the job done.  This is also added
// to Redmine (#365).
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Oct-14  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "printf.h"
#include "cpu.h"
#include "process.h"


//
// -- Allocate a new PID from the table, crashing if none are available
//    **NOTE** The caller is responsible for obtaining pidTableLock prior to calling
//    ------------------------------------------------------------------------------
PID_t ProcessNewPID(void)
{
    static PID_t lastPID = MAX_NUM_PID;
    PID_t i = lastPID;

    // -- check each PID in turn to see if there is one available
    do {
        if ((++ i) == MAX_NUM_PID) i = 0;
        if (procs[i] == NULL) {
            lastPID = i;
            return i;
        }
    } while (i != lastPID);

    Halt();
}
