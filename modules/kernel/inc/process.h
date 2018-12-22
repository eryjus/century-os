//===================================================================================================================
//
//  process.h -- Structures for the Process management
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  This file contains the structures and prototypes needed to manage processes.  This file was copied from
//  Century32 and will need to be updated for this kernel.
//
//  The process structure will be allocated with the new process stack -- adding it to the overhead of the process
//  itself.  Since the stacks will be multiples of 4K, one extra page will be allocated for the process stucture
//  overhead.  This will be placed below the stack in a manner where a stack underflow will not impact this
//  structure unless something really goes horrible and deliberately wrong.  The scheduler will be able to check
//  that the stack is within bounds properly and kill the process if needed (eventually anyway).  This simple
//  decision will eliminate the need for process structures or the need to allocate a process from the heap.
//
//  There are several statuses for processes that should be noted.  They are:
//  * INIT -- the process is initializing and is not ready to run yet.
//  * RUN -- the process is in a runnable state.  In this case, the process may or may not be the current process.
//  * END -- the process has ended and the Butler process needs to clean up the structures, memory, IPC, locks, etc.
//  * MTXW -- the process is waiting for a mutex and is ineligible to run.
//  * SEMW -- the process is waiting for a semaphore and is ineligible to run.
//  * DLYW -- the process is waiting for a timed delay and is ineligible to run.
//  * MSGW -- the process is waiting for the delivery of a message and is ineligible to run.
//  * ZOMB -- the process has died at the OS level for some reason or violation and the Butler process is going to
//            clean it up.
//
//  Additionally, we are going to support the ability for a user to hold a process.  In this case, the process will
//  also be ineligible to run.  These held processes will be moved into another list which will maintain the
//  overall status of the process.
//
//  The process priorities will serve 2 functions.  It will 1) provide a sequence of what is eligibe to run and
//  when from a scheduler perspective.  It will also 2) provide the quantum duration for which a process is able to
//  use the CPU.  In this case, a higher priority process will be able use the CPU longer than a low priority
//  process.  Additionally, the Idle process is also the Butler process.  When there is something that needs to
//  be done, the Butler will artificially raise its CPU priority to be an OS process while it is completing this
//  work.  When complete the Butler will reduce its priority again.
//
//  Finally, threads will be supported not yet be supported at the OS level.  If needed, they will be added at a
//  later time.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2012-Sep-23                             Initial version
//  2013-Aug-20        46                   Added a reference to an allocated tty session
//  2013-Aug-22     58                   Normalize the TTY functions to use tty#
//  2013-Aug-25     60                   Create the ability to find a process quickly by PID
//  2013-Aug-25     67                   Add a list of locks a process holds
//  2013-Aug-27     66                   Added a Zombied process status
//  2013-Aug-27     36                   Add messages to the process structure
//  2013-Sep-01     83                   Add a spinlock to the PID list array (removed 2018-Oct-14)
//  2013-Sep-02     73                   Add a lock to the process structure (removed 2018-Oct-14)
//  2013-Sep-03     73                   Encapsulate Process Structure
//  2018-Oct-14  Initial   0.1.0   ADCL  Copied this file from Century32 and cleaned it up for century-os
//
//===================================================================================================================


#ifndef __PROCESS_H__
#define __PROCESS_H__


#include "types.h"
#include "printf.h"
#include "lists.h"
#include "spinlock.h"


//
// -- The length of an identifying command
//    ------------------------------------
#define MAX_CMD_LEN             32


//
// -- Define the max number if PIDs that will be supported
//    ----------------------------------------------------
#define MAX_NUM_PID             32768


//
// -- This list is are the statuses available for a running process, indicating what queue it will be on.
//    ---------------------------------------------------------------------------------------------------
typedef enum { PROC_INIT,               // This is being created and is not on a queue yet
        PROC_RUN,                       // This is eligible to run but may or may not be the current process
        PROC_END,                       // This has ended normally and has been added to the reaper queue
        PROC_MTXW,                      // This is waiting for a Mutex lock and is on the waiting queue
        PROC_SEMW,                      // This is waiting on a Semaphore and is on the waiting queue
        PROC_DLYW,                      // This is waiting for a timed event and is on the waiting queue
        PROC_MSGW,                      // This is waiting for a message to be delivered and in on the waiting queue
        PROC_ZOMB,                      // This is a crashed process and is on the reaper queue
} ProcStatus_t;


//
// -- This is list is the priority of the process, which doubles as the quantum that will be given a process
//    ------------------------------------------------------------------------------------------------------
typedef enum { PTY_IDLE = 1,            // This is for the butler process when there is nothing to do
        PTY_LOW = 5,                    // This is a low priority user process
        PTY_NORM = 10,                  // This is a normal user process
        PTY_HIGH = 20,                  // This is a high priority user process
        PTY_OS = 30,                    // This is an OS or Driver process
} ProcPriority_t;


//
// -- This is a process structure
//    ---------------------------
typedef struct Process_t {
    regval_t stackPointer;              // This is the process current esp value (when not executing)
    regval_t ss;                        // This is the process ss value
    regval_t pageTables;                // This is the process cr3 value
    PID_t pid;                          // This is the PID of this process
    ptrsize_t ssAddr;                   // This is the address of the process stack
    size_t ssLength;                    // This is the length of the process stack
    char command[MAX_CMD_LEN];          // The identifying command, includes the terminating null
    size_t totalQuantum;                // This is the total quantum used by this process
    ProcStatus_t status;                // This is the process status
    ProcPriority_t priority;            // This is the process priority
    int quantumLeft;                    // This is the quantum remaining for the process (may be more than priority)
    bool isHeld;                        // Is this process held and on the Held list?
    struct Spinlock_t lock;             // This is the lock needed to read/change values
    ListHead_t::List_t stsQueue;        // This is the location on the current status queue
    ListHead_t lockList;                // This is the list of currently held locks (for the butler to release)
    ListHead_t messages;                // This is the queue of messages for this process
    void *prevPayload;                  // This is the previous payload in case the process did not allocate enough
} __attribute__((packed)) Process_t;


//
// -- The initial flags for the processes
//    -----------------------------------
#define INIT_FLAGS        0x00000200


//
// -- This is the process table
//    -------------------------
extern Process_t *procs[MAX_NUM_PID];


//
// -- Is the scheduler Enabled?
//    -------------------------
extern bool ProcessEnabled;


//
// -- The butler process structure
//    ----------------------------
extern Process_t butler;


//
// -- Several Lists for managing processes
//    ------------------------------------
extern ListHead_t procOsPtyList;
extern ListHead_t procHighPtyList;
extern ListHead_t procNormPtyList;
extern ListHead_t procLowPtyList;
extern ListHead_t procIdlePtyList;
extern ListHead_t procWaitList;
extern ListHead_t procHeldList;
extern ListHead_t procReaper;


//
// -- Initialize the process structures
//    ---------------------------------
void ProcessInit(void);


//
// -- Allocate a new PID from the PID table
//    -------------------------------------
PID_t ProcessNewPID(void);


//
// -- Hold a process by PID
//    ---------------------
int ProcessHold(PID_t pid);


//
// -- Release a process by PID
//    ------------------------
int ProcessRelease(PID_t pid);


//
// -- Ready a process by PID
//    ----------------------
int ProcessReady(PID_t pid);


//
// -- Terminate a process
//    -------------------
int ProcessTerminate(PID_t pid);


//
// -- End a process normally
//    ----------------------
void ProcessEnd(void) __attribute__((noreturn));


//
// -- Put the process on the wait queue and set its status
//    ----------------------------------------------------
int ProcessRelease(ProcStatus_t newStat);


//
// -- Reschedule a process to the next one on the scheduler
//    -----------------------------------------------------
void ProcessReschedule(void);


//
// -- Put the process on the wait queue and set its status
//    ----------------------------------------------------
int ProcessWait(ProcStatus_t newStat);


//
// -- Switch to a new Process_t
//    -------------------------
extern "C" void ProcessSwitch(Process_t *current, Process_t *target);


//
// -- A function to get the Process structure from the PID
//    ----------------------------------------------------
inline Process_t *ProcessGetStruct(PID_t pid) { return (pid>=MAX_NUM_PID?NULL:procs[pid]); }


//
// -- Deallocate a PID from the PID table
//    -----------------------------------
inline void ProcessFreePID(PID_t pid) { if (pid < MAX_NUM_PID) procs[pid] = 0; }


//
// -- We need an additional include
//    -----------------------------
#include "spinlock.h"
#include <errno.h>


//
// -- A spinlock needed for updating the procs table
//    ----------------------------------------------
extern Spinlock_t pidTableLock;


//
// -- These are some standard process PIDs
//    ------------------------------------
enum {
    PID_IDLE = 0,
    PID_BUTLER = 1,
    PID_PMM = 2,
};


//
// -- Standard sanity checks for PIDs
//    -------------------------------
#define SANITY_CHECK_PID(pid,proc)        do {                                          \
                                            if (pid > MAX_NUM_PID) {                    \
                                                return -EUNDEF;                         \
                                            }                                           \
                                                                                        \
                                            proc = procs[pid];                          \
                                                                                        \
                                            if(!proc) {                                 \
                                                return -EUNDEF;                         \
                                            }                                           \
                                        } while (0)


#endif
