//===================================================================================================================
//
//  DebugVars.cc -- Variables used by the debugger
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Apr-02  Initial  v0.6.0a  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "debugger.h"


//
// -- This is the current variable that identifies the current state
//    --------------------------------------------------------------
EXPORT KERNEL_BSS
DebuggerState_t debugState;


//
// -- This is the buffer for the command being entered
//    ------------------------------------------------
EXPORT KERNEL_BSS
char debugCommand[DEBUG_COMMAND_LEN];


//
// -- For each state, this is the visual representation where on the command tree the user is and what the
//    valid commands are (indexed by state).
//    ----------------------------------------------------------------------------------------------------
EXPORT KERNEL_DATA
DebugPrompt_t dbgPrompts[] {
// -- location              allowed
    {"-",                   "scheduler,timer"},                                 // -- DBG_HOME
    {"sched",               "show,status,run,ready,list,exit"},                 // -- DBG_SCHED
    {"sched:ready",         "all,os,high,normal,low,idle,exit"},                // -- DBG_SCHED_RDY
    {"sched:list",          "blocked,sleeping,zombie,exit"},                    // -- DBG_SCHED_LIST
    {"timer",               "counts,config"},                                   // -- DBG_TIMER
};


//
// -- This is the actual debug communication structure
//    ------------------------------------------------
EXPORT KERNEL_BSS
DebugComm_t debugCommunication = {0};
