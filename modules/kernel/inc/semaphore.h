//===================================================================================================================
//
//  semaphore.h -- The semaphore low-level implementation for Century-OS
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Apr-30  Initial   0.4.3   ADCL  Initial version
//
//===================================================================================================================


#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__


#include "sys/sem.h"
#include "atomic.h"
#include "spinlock.h"
#include "lists.h"


//
// == These values are used by POSIX and will need to be defined; to be configured at boot later
//    ==========================================================================================

// -- Max semaphores per set
#define SEMMSL      250
extern int semmsl;

// -- system-wide max number of semaphores
#define SEMMNS      32000
extern int semmns;

// -- maximum number of operations for semop()??
#define SEMOPM      32
extern int semopm;

// -- Mex number of semaphore sets
#define SEMMNI      128
extern int semmni;


//
// -- Semaphore Control System Function
//    ---------------------------------
__CENTURY_FUNC__ int SemaphoreControl(int semid, int semnum, int cmd, ...);


//
// -- Get a Semaphore System Function
//    -------------------------------
__CENTURY_FUNC__ int SemaphoreGet(key_t key, int nsems, int semflg);


//
// -- Semaphore Operations System Function
//    ------------------------------------
__CENTURY_FUNC__ int SemaphoreOperations(int semid, struct sembuf *sops, size_t nsops);


//
// -- The Semaphore Initialization function
//    -------------------------------------
__CENTURY_FUNC__ void SemaphoreInit(void);


//
// -- This is the structure of an actual semaphore
//    --------------------------------------------
typedef struct Semaphore_t {
    AtomicInt_t semval;
    pid_t semPid;
    ListHead_t waitN;
    ListHead_t waitZ;
} Semaphore_t;


//
// -- This structure internally defines a semaphore set
//    -------------------------------------------------
typedef struct SemaphoreSet_t {
    key_t key;
    Spinlock_t lock;
    struct ipc_perm permissions;
    int numSem;
    Semaphore_t *semSet;
    time_t semOtime;
    time_t semCtime;
    uid_t owner;
    uid_t creator;
} SemaphoreSet_t;


//
// -- This structure will be the Undo List for the structure
//    ------------------------------------------------------
typedef struct SemaphoreUndo_t {
    ListHead_t::List_t list;
    int semid;                      // -- the semid to be tracked
    key_t key;                      // -- confirmation of the key to track
    PID_t pid;                      // -- the pid that needs to be tracked
    int semnum;                     // -- this is the sem number in the set to be tracked
    int semadj;                     // -- this is the pending adjustment
} SemaphoreUndo_t;


//
// -- This is the global semaphore implementation structure
//    -----------------------------------------------------
typedef struct SemaphoreAll_t {
    Spinlock_t globalLock;                  // -- the lock to access semaphoreSets
    SemaphoreSet_t **semaphoreSets;
    ListHead_t undoList;
} SemaphoreAll_t;


//
// -- We need the following union at the kernel level to handle semctl() argument 4 (see `man semctl`)
//    ------------------------------------------------------------------------------------------------
union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
};


//
// -- this is the global semaphore structure, encapsulating all system semaphores
//    ---------------------------------------------------------------------------
extern SemaphoreAll_t semaphoreAll;


#endif
