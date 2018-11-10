//===================================================================================================================
//
// kernel/src/process/ProcessVars.cc -- Global variables for process management
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Oct-14  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "process.h"


//
// -- This is the current PID
//    -----------------------
volatile PID_t currentPID;


//
// -- This is the array of process structures indexed by PID; also the global process queue
//    -------------------------------------------------------------------------------------
Process_t *procs[MAX_NUM_PID] = {0};


//
// -- A spinlock needed for updating the procs table
//    ----------------------------------------------
Spinlock_t pidTableLock;


//
// -- This is the ready process queue; anything on this queue is ready to run.
//    Note that there is really one queue per priority.
//    ------------------------------------------------------------------------
NEW_LIST(procOsPtyList);
NEW_LIST(procHighPtyList);
NEW_LIST(procNormPtyList);
NEW_LIST(procLowPtyList);
NEW_LIST(procIdlePtyList);


//
// -- This is the process structure for the butler process
//    ----------------------------------------------------
Process_t butler = {
    0,                      // esp
    0,                      // ss
    0,                      // cr3
    0,                      // pid
    0x200000 - 4096,        // stack location
    4096,                   // stack length
    "Butler",               // process name
    0,                      // total quantum
    PROC_RUN,               // process status
    PTY_OS,                 // the priority
    0,                      // quantum left
    false,                  // not held
    {0},                    // the spinlock for this process
    {0},                    // the status queue
    {0},                    // the lock list
    {0},                    // pending messages
    NULL,                   // the previous payload
};


//
// -- This is a list of waiting processes.  These processes are waiting for some external event.
//    ------------------------------------------------------------------------------------------
NEW_LIST(procWaitList);


//
// -- This is a list of processes that are held externally.  They are ineligible for processing.
//    ------------------------------------------------------------------------------------------
NEW_LIST(procHeldList);


//
// -- This is a queue of processes that the Butler needs to clean up.
//    ---------------------------------------------------------------
NEW_LIST(procReaper);


//
// -- Is the scheduler enabled
//    ------------------------
bool ProcessEnabled = false;