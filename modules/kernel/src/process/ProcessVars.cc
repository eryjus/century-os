//===================================================================================================================
//
// ProcessVars.cc -- Global variables for process management
//
//        Copyright (c)  2017-2019 -- Adam Clark
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
// -- This is the current running process
//    -----------------------------------
__krndata Process_t *currentProcess = NULL;


//
// -- This is the next PID that will be allocated
//    -------------------------------------------
__krndata PID_t nextPID = 0;


//
// -- This is the round robin queue of processes to execute
//    -----------------------------------------------------
__krndata QueueHead_t roundRobin;


//
// -- This is the number of times we have entered a critical section
//    --------------------------------------------------------------
__krndata int schedulerLocksHeld = 0;


//
// -- Is there a pending process change?
//    ----------------------------------
__krndata bool processChangePending = false;


//
// -- This is the list of sleeping tasks
//    ----------------------------------
__krndata QueueHead_t sleepingTasks;


//
// -- This is the next time we have to wake something up
//    --------------------------------------------------
__krndata volatile uint64_t nextWake = (uint64_t)-1;


//
// -- the lock to hold to be able to increment the locks held count
//    -------------------------------------------------------------
__krndata Spinlock_t schedulerLock;

