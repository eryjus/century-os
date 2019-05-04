//===================================================================================================================
//
//  sys/sem.h -- A POSIX-compliant header for semaphore syscalls
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  This file is provided as a POSIX-compliant semaphore system call implementaiton.  This file will be copied
//  to the target file structure.
//
//  See `man sys_sem.h` for more information -- I am not going to duplicate that work here.
//
//  Variances:
//  The following are variances from the POSIX standard:
//  1. `pid_t` will be type-defined as `PID_t`, currently as `uint32_t`.  POSIX states: "blksize_t, pid_t, and
//     ssize_t shall be signed integer types."
//  2. `size_t` will be defined from <stddef.h>.  POSIX states: "size_t shall be an unsigned integer type."  This
//     does not vary from the POSIX standard except in that sys/sem.h does not define `size_t` itself.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Apr-29  Initial   0.4.3   ADCL  Initial version
//
//===================================================================================================================


#ifndef __SEM_H__
#define __SEM_H__


#define __need_size_t
#include <stddef.h>
#include <stdint.h>


//
// -- Define flags for the semaphore operations
//    -----------------------------------------

// -- Undo the operation on exit
#define SEM_UNDO        0x1000


//
// -- The commands that can be passed to `semop`
//    ------------------------------------------

// -- get sempid
#define GETPID      11

// -- get semval
#define GETVAL      12

// -- get all semvals
#define GETALL      13

// -- get semncnt
#define GETNCNT     14

// -- get semzcnt
#define GETZCNT     15

// -- set semval
#define SETVAL      16

// -- set all semvals
#define SETALL      17


#include "sys/ipc.h"


//
// -- pick up the type definition of `pic_t`
//    --------------------------------------
#if __has_include("types.h")
#   include "types.h"
    typedef PID_t pid_t;
#else
#   warning "\"types.h\" is not available to be included; guessing on some types and sizes.  "\
        "This may break the build."
    typedef uint32_t pid_t;
#endif


//
// -- define the `time_t` type
//    ------------------------
typedef uint32_t time_t;


//
// -- Define the `semid_ds` structure
//    -------------------------------
struct semid_ds {
    struct ipc_perm sem_perm;           // -- operation permission structure
    unsigned short sem_nsems;           // -- number of semaphores in this set
    time_t sem_otime;                   // -- time of the last semop()
    time_t sem_ctime;                   // -- time last changed by semctl()
};


//
// -- Define the `sembuf` structure
//    -----------------------------
struct sembuf {
    unsigned short sem_num;             // -- semaphore number
    short sem_op;                       // -- semaphore operation
    short sem_flg;                      // -- operations flags
};


#ifndef __CENTURY_KERNEL__
//
// -- These prototypes are required -- but these are not the kernel functions
//    -----------------------------------------------------------------------
int semctl(int semid, int semnum, int cmd, ...);
int semget(key_t key, int nsems, int semflg);
int semop(int semid, struct sembuf *sops, size_t nsops);
#endif


// -------------------------------------------------------------------------------------------------
// -- The user program is required (by POSIX) to define the following union for use with `semctl()`:
//
//    union semun {
//        int              val;    /* Value for SETVAL */
//        struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
//        unsigned short  *array;  /* Array for GETALL, SETALL */
//    };
//
//    This is used as an optional 4th argument to the `semctl()` function.
// -------------------------------------------------------------------------------------------------


#endif




