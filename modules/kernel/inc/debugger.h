//===================================================================================================================
//
//  debugger.h -- The header-level definitions and prototypes for the kernel debugger
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


#pragma once


#include "types.h"
#include "printf.h"


//
// -- Some defines to make the debugger output pretty!
//    ------------------------------------------------
#define     ANSI_PASTE(x)           #x
#define     ANSI_ESC                "\x1b["
#define     ANSI_CLEAR              ANSI_ESC "2J"
#define     ANSI_SET_CURSOR(r,c)    ANSI_ESC ANSI_PASTE(r) ";" ANSI_PASTE(c) "H"
#define     ANSI_CURSOR_UP(x)       ANSI_ESC ANSI_PASTE(x) "A"
#define     ANSI_CURSOR_DOWN(x)     ANSI_ESC ANSI_PASTE(x) "B"
#define     ANSI_CURSOR_FORWARD(x)  ANSI_ESC ANSI_PASTE(x) "C"
#define     ANSI_CURSOR_BACKWARD(x) ANSI_ESC ANSI_PASTE(x) "D"
#define     ANSI_CURSOR_SAVE        ANSI_ESC "s"
#define     ANSI_CURSOR_RESTORE     ANSI_ESC "u"
#define     ANSI_ERASE_LINE         ANSI_ESC "K"

#define     ANSI_ATTR_NORMAL        ANSI_ESC "0m"
#define     ANSI_ATTR_BOLD          ANSI_ESC "1m"
#define     ANSI_ATTR_BLINK         ANSI_ESC "5m"
#define     ANSI_ATTR_REVERSE       ANSI_ESC "7m"

#define     ANSI_FG_BLACK           ANSI_ESC "30m"
#define     ANSI_FG_RED             ANSI_ESC "31m"
#define     ANSI_FG_GREEN           ANSI_ESC "32m"
#define     ANSI_FG_YELLOW          ANSI_ESC "33m"
#define     ANSI_FG_BLUE            ANSI_ESC "34m"
#define     ANSI_FG_MAGENTA         ANSI_ESC "35m"
#define     ANSI_FG_CYAN            ANSI_ESC "36m"
#define     ANSI_FG_WHITE           ANSI_ESC "37m"

#define     ANSI_BG_BLACK           ANSI_ESC "40m"
#define     ANSI_BG_RED             ANSI_ESC "41m"
#define     ANSI_BG_GREEN           ANSI_ESC "42m"
#define     ANSI_BG_YELLOW          ANSI_ESC "43m"
#define     ANSI_BG_BLUE            ANSI_ESC "44m"
#define     ANSI_BG_MAGENTA         ANSI_ESC "45m"
#define     ANSI_BG_CYAN            ANSI_ESC "46m"
#define     ANSI_BG_WHITE           ANSI_ESC "47m"


//
// -- These are the commands that are allowed to be sent to all the cores
//    -------------------------------------------------------------------
typedef enum {
    DIPI_ENGAGE,
    DIPI_TIMER,
} DbgIniCommand_t;


//
// -- The debugger command tree will be implemented as a state machine -- these are the states
//    ----------------------------------------------------------------------------------------
typedef enum {
    DBG_HOME,
    DBG_SCHED,
    DBG_SCHED_RDY,
    DBG_SCHED_LIST,
    DBG_TIMER,
} DebuggerState_t;


//
// -- The debugger commands
//    ---------------------
typedef enum {
    CMD_ERROR,
    CMD_SCHED,
    CMD_SHOW,
    CMD_STAT,
    CMD_RUNNING,
    CMD_READY,
    CMD_ALL,
    CMD_OS,
    CMD_HIGH,
    CMD_NORM,
    CMD_LOW,
    CMD_IDLE,
    CMD_LIST,
    CMD_BLOCKED,
    CMD_SLEEPING,
    CMD_ZOMBIE,
    CMD_HELP,
    CMD_EXIT,
    CMD_TIMER,
    CMD_COUNTS,
    CMD_CONFIG,
} DebuggerCommand_t;


//
// -- The main debugger entry point
//    -----------------------------
EXTERN_C EXPORT KERNEL
void DebugStart(void);


//
// -- Given the current command branch, prompt for and get the next command
//    ---------------------------------------------------------------------
EXTERN_C EXPORT KERNEL
void DebugPrompt(DebuggerState_t state);


//
// -- Signal the other cores to stop and wait for confirmation that they have
//    -----------------------------------------------------------------------
EXTERN_C EXPORT KERNEL
void DebuggerEngage(DbgIniCommand_t cmd);


//
// -- Release the other cores from a stopped state
//    --------------------------------------------
EXTERN_C EXPORT KERNEL
void DebuggerRelease(void);


//
// -- Debug the scheduler
//    -------------------
EXTERN_C EXPORT KERNEL
void DebugScheduler(void);


//
// -- Show the status of the scheduler queues
//    ---------------------------------------
EXTERN_C EXPORT KERNEL
void DebugSchedulerStat(void);


//
// -- Dump the interesting values from the running processes on each CPU
//    ------------------------------------------------------------------
EXTERN_C EXPORT KERNEL
void DebugSchedulerRunning(void);


//
// -- Debug the timer
//    ---------------
EXTERN_C EXPORT KERNEL
void DebugTimer(void);


//
// -- Debug the timer over all CPUs
//    -----------------------------
EXTERN_C EXPORT KERNEL
void DebugTimerCounts(void);


//
// -- Dump the timer config
//    ---------------------
EXTERN_C EXPORT KERNEL
void DebugTimerConfig(void);


//
// -- Parse the entered command line for the next command
//    ---------------------------------------------------
EXTERN_C EXPORT KERNEL
DebuggerCommand_t DebugParse(DebuggerState_t state);


//
// -- This is the current variable that identifies the current state
//    --------------------------------------------------------------
EXTERN EXPORT KERNEL_BSS
DebuggerState_t debugState;


//
// -- the global command buffer
//    -------------------------
EXTERN EXPORT KERNEL_BSS
char debugCommand[DEBUG_COMMAND_LEN];


//
// -- This is the structure the prompts are kept in
//    ---------------------------------------------
typedef struct DebugPrompt_t {
    const char *branch;
    const char *allowed;
} DebugPrompt_t;


//
// -- these are the prompts for any given state
//    -----------------------------------------
EXTERN EXPORT KERNEL_DATA
DebugPrompt_t dbgPrompts[];


//
// -- print spaces to the desired width
//    ---------------------------------
EXPORT INLINE
void DbgSpace(int w, int p) {
    w -= p;
    while (w >= 0) {
        kprintf(" ");
        w --;
    }
}


//
// -- This structure is the communication structure between the cores when debugging
//    ------------------------------------------------------------------------------
typedef struct DebugComm_t {
    AtomicInt_t coresEngaged;                   // -- used to get the cores into the IPI for halting for a short time
    archsize_t debuggerFlags;                   // -- flags on the core where debugger is running
    DbgIniCommand_t command;                    // -- this is what the IPI is required to do on each core
    AtomicInt_t coresResponded;                 // -- depending on command, may be used to indicate response provided
    uint64_t timerValue[MAX_CPUS];              // -- the values for the timers for each CPU
} DebugComm_t;


//
// -- This is the actual debug communication structure
//    ------------------------------------------------
EXTERN EXPORT KERNEL_BSS
DebugComm_t debugCommunication;




