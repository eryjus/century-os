//===================================================================================================================
//
//  process.h -- Structures for the Process management
//
//        Copyright (c)  2017-2019 -- Adam Clark
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
//  2012-Sep-23                          Initial version
//  2013-Aug-20     46                   Added a reference to an allocated tty session
//  2013-Aug-22     58                   Normalize the TTY functions to use tty#
//  2013-Aug-25     60                   Create the ability to find a process quickly by PID
//  2013-Aug-25     67                   Add a list of locks a process holds
//  2013-Aug-27     66                   Added a Zombied process status
//  2013-Aug-27     36                   Add messages to the process structure
//  2013-Sep-01     83                   Add a spinlock to the PID list array (removed 2018-Oct-14)
//  2013-Sep-02     73                   Add a lock to the process structure (removed 2018-Oct-14)
//  2013-Sep-03     73                   Encapsulate Process Structure
//  2018-Oct-14  Initial   0.1.0   ADCL  Copied this file from Century32 and cleaned it up for century-os
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#ifndef __PROCESS_H__
#define __PROCESS_H__


#include "types.h"
#include "printf.h"
#include "lists.h"
#include "cpu.h"
#include "timer.h"
#include "spinlock.h"


//
// -- This list is are the statuses available for a running process, indicating what queue it will be on.
//    ---------------------------------------------------------------------------------------------------
typedef enum { PROC_INIT,               // This is being created and is not on a queue yet
        PROC_RUNNING,                   // This is currently running (See ProcessSwitch!)
        PROC_READY,                     // This is ready to run (See ProcessSwitch!)
        PROC_TERM,                      // This has ended
        PROC_MTXW,                      // This is waiting for a Mutex lock and is on the waiting queue
        PROC_SEMW,                      // This is waiting on a Semaphore and is on the waiting queue
        PROC_DLYW,                      // This is waiting for a timed event and is on the waiting queue
        PROC_MSGW,                      // This is waiting for a message to be delivered and in on the waiting queue
} ProcStatus_t;


//
// -- This list is the policy choices for a running process; unused currently
//    -----------------------------------------------------------------------
typedef enum { POLICY_0,
    POLICY_1,
    POLICY_2,
    POLICY_3,
} ProcPolicy_t;


//
// -- This is list is the priority of the process, which doubles as the quantum that will be given a process
//    ------------------------------------------------------------------------------------------------------
typedef enum { PTY_LOW = 5,             // This is a low priority user process
        PTY_NORM = 10,                  // This is a normal user process
        PTY_HIGH = 20,                  // This is a high priority user process
        PTY_OS = 30,                    // This is an OS or Driver process
} ProcPriority_t;


//
// -- This is a process structure
//    ---------------------------
typedef struct Process_t {
    archsize_t topOfStack;              // This is the process current esp value (when not executing)
    archsize_t virtAddrSpace;           // This is the process top level page table
    ProcStatus_t status;                // This is the process status
    ProcPriority_t priority;            // This is the process priority
    int quantumLeft;                    // This is the quantum remaining for the process (may be more than priority)
    PID_t pid;                          // This is the PID of this process
    archsize_t ssAddr;                  // This is the address of the process stack
    char *command;                      // The identifying command, includes the terminating null
    ProcPolicy_t policy;                // This is the scheduling policy
    uint64_t timeUsed;                  // This is the relative amount of CPU used
    uint64_t wakeAtMicros;              // Wake this process at or after this micros since boot
    ListHead_t::List_t stsQueue;        // This is the location on the current status queue
} Process_t;


//
// -- This structure encapsulates the whole of the scheduler
//    ------------------------------------------------------
typedef struct Scheduler_t {
    // -- These fields can only be changed after ProcessEnterPostpone(); SMP may change this
    Process_t *currentProcess;              // the process that is currently executing on the cpu
    volatile bool processChangePending;     // whether there is a change that is pending
    PID_t nextPID;                          // the next pid number to allocate
    volatile uint64_t nextWake;             // the next tick-since-boot when a process needs to wake up

    // -- This is a critical field controlled by its lock
    volatile int schedulerLocksHeld;        // the depth of the locks
    Spinlock_t schedulerLock;               // lock must be held to change the above field

    // -- and the different lists a process might be on, locks in each list will be used
    QueueHead_t queueOS;                    // this is the queue for the OS tasks -- if it can run it does
    QueueHead_t queueHigh;                  // this is the queue for High pty tasks
    QueueHead_t queueNormal;                // these are the typical tasks -- most non-OS tasks will be here
    QueueHead_t queueLow;                   // low priority tasks which do not need cpu unless there is nothing else
    ListHead_t  listBlocked;                // these are blocked tasks for any number of reasons
    ListHead_t  listSleeping;               // these are sleeping tasks, which the timer interrupt will investigate
    ListHead_t  listTerminated;             // these are terminated tasks, which are waiting to be torn down
} Scheduler_t;


//
// -- And the scheduler object itself
//    -------------------------------
extern Scheduler_t scheduler;


//
// -- Initialize the process structures
//    ---------------------------------
__CENTURY_FUNC__ void ProcessInit(void);


//
// -- New task initialization tasks
//    -----------------------------
__CENTURY_FUNC__ void ProcessStart(void);


//
// -- Create a new process
//    --------------------
__CENTURY_FUNC__ Process_t *ProcessCreate(void (*startingAddr)(void));


//
// -- Switch to a new process
//    -----------------------
__CENTURY_FUNC__ void ProcessSwitch(Process_t *proc);


//
// -- Create a new stack for a new process, and populate its contents
//    ---------------------------------------------------------------
__CENTURY_FUNC__ frame_t ProcessNewStack(Process_t *proc, void (*startingAddr)(void));


//
// -- Perform a scheduling exercise to determine the next process to run
//    ------------------------------------------------------------------
__CENTURY_FUNC__ void ProcessSchedule(void);


//
// -- Place a process on the correct ready queue
//    ------------------------------------------
__CENTURY_FUNC__ void ProcessReady(Process_t *proc);


//
// -- Block a process
//    ---------------
__CENTURY_FUNC__ void ProcessBlock(ProcStatus_t reason);


//
// -- Unblock a process
//    -----------------
__CENTURY_FUNC__ void ProcessUnblock(Process_t *proc);


//
// -- Update the time used for a process
//    ----------------------------------
__CENTURY_FUNC__ void ProcessUpdateTimeUsed(void);


//
// -- Sleep until the we reach the number of micro-seconds since boot
//    ---------------------------------------------------------------
__CENTURY_FUNC__ void ProcessMicroSleepUntil(uint64_t when);


//
// -- Terminate a task
//    ----------------
__CENTURY_FUNC__ void ProcessTerminate(Process_t *proc);


//
// -- Elect to end the current task
//    -----------------------------
__CENTURY_FUNC__ inline void ProcessEnd(void) { ProcessTerminate(scheduler.currentProcess); }


//
// -- Lock the scheduler for a critical section
//    -----------------------------------------
__CENTURY_FUNC__ void ProcessEnterPostpone(void);


//
// -- Unlock the scheduler after a critical section
//    ---------------------------------------------
__CENTURY_FUNC__ void ProcessExitPostpone(void);


//
// -- remove the process for its list, if it is on one
//    ------------------------------------------------
__CENTURY_FUNC__ void ProcessListRemove(Process_t *proc);


//
// -- Micro-Sleep this number of micro-seconds
//    ----------------------------------------
__CENTURY_FUNC__ inline void ProcessMicroSleep(uint64_t micros) {
    ProcessMicroSleepUntil(TimerCurrentCount(timerControl) + micros);
}


//
// -- Milli-Sleep this number of milli-seconds
//    ----------------------------------------
__CENTURY_FUNC__ inline void ProcessMilliSleep(uint64_t ms) {
    ProcessMicroSleepUntil(TimerCurrentCount(timerControl) + (ms * 1000));
}


//
// -- Sleep this number of seconds
//    ----------------------------
__CENTURY_FUNC__ inline void ProcessSleep(uint64_t secs) {
    ProcessMicroSleepUntil(TimerCurrentCount(timerControl) + (secs * 1000000));
}


//
// -- For the scheduler structure, clean the cache pushing the changes to ram
//    -----------------------------------------------------------------------
#define CLEAN_SCHEDULER()           CLEAN_CACHE(&scheduler, sizeof(Scheduler_t))


//
// -- For the scheduler structure, invalidate the cache forcing a re-read from ram
//    ----------------------------------------------------------------------------
#define INVALIDATE_SCHEDULER()      INVALIDATE_CACHE(&scheduler, sizeof(Scheduler_t))


//
// -- for a Process structure, clean the cache pushing the changes to ram
//    -------------------------------------------------------------------
#define CLEAN_PROCESS(proc)         CLEAN_CACHE(proc, sizeof(Process_t))


//
// -- for a Process structure, invalidate the cache forcing a re-read from ram
//    ------------------------------------------------------------------------
#define INVALIDATE_PROCESS(proc)    INVALIDATE_CACHE(proc, sizeof(Process_t))


#endif
