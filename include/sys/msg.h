//===================================================================================================================
//
//  sys/msg.h -- A POSIX-compliant header for messaging syscalls
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  This file is provided as a POSIX-compliant messaging system call implementaiton.  This file will be copied
//  to the target file structure.
//
//  See `man sys_msg.h` for more information -- I am not going to duplicate that work here.
//
//  Variances:
//  The following are variances from the POSIX standard:
//  1. `pid_t` will be type-defined as `PID_t`, currently as `uint32_t`.  POSIX states: "blksize_t, pid_t, and
//     ssize_t shall be signed integer types."
//  2. `size_t` and `ssize_t` will be defined from <stddef.h>.  POSIX states: "size_t shall be an unsigned integer
//     type."  This does not vary from the POSIX standard except in that sys/sem.h does not define `size_t` itself.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-May-14  Initial   0.4.4   ADCL  Initial version
//
//===================================================================================================================


#ifndef __MSG_H__
#define __MSG_H__

#define __need_size_t
#define __need_ssize_t
#include <stddef.h>
#include <stdint.h>


//
// -- Define flags for the semaphore operations
//    -----------------------------------------


// -- no error if message is too big
#define MSG_NOERROR	010000


#include "sys/ipc.h"


//
// -- pick up the type definition of `pid_t`
//    --------------------------------------
#ifndef __POSIX_PID_T__
#   if __has_include("types.h")
#       include "types.h"
        typedef PID_t pid_t;
#   else
#       warning "\"types.h\" is not available to be included; guessing on some types and sizes.  "\
            "This may break the build."
        typedef uint32_t pid_t;
#   endif
#   define __POSIX_PID_T__
#endif


//
// -- define the `time_t` type
//    ------------------------
#ifndef __POSIX_TIME_T__
typedef uint32_t time_t;
#   define __POSIX_TIME_T__
#endif


//
// -- define the `msgqnum_t` type
//    ---------------------------
#ifndef __POSIX_MSGQNUM_T__
typedef unsigned long msgqnum_t;
#   define __POSIX_MSGQNUM_T__
#endif


//
// -- define the `msglen_t` type
//    --------------------------
#ifndef __POSIX_MSGLEN_T__
typedef unsigned long msglen_t;
#   define __POSIX_MSGLEN_T__
#endif


//
// -- Define the `msqid_ds` structure
//    -------------------------------
struct msqid_ds {
    struct ipc_perm msg_perm;           // -- operation permission structure
    msgqnum_t msg_qnum;                 // -- number of messages currently on the queue
    msglen_t msg_qbytes;                // -- maximum number bytes allowed on the queue
    pid_t msg_lspid;                    // -- Process ID of the last msgsnd()
    pid_t msg_lrpid;                    // -- Process ID of the last msgrcv()
    time_t msg_stime;                   // -- time of the last msgsnd()
    time_t msg_rtime;                   // -- time of the last msgrcv()
    time_t msg_ctime;                   // -- time of the last change
};


#ifndef __CENTURY_KERNEL__
//
// -- These prototypes are required -- but these are not the kernel functions
//    -----------------------------------------------------------------------
extern int msgctl(int msqid, int cmd, struct msqid_ds);
extern int msgget(key_t key, int msgflg);
extern ssize_t msgrcv(int msqid, void *msgp, size_t msgsz, long int msgtyp, int msgflg);
extern int msgsnd(int msqid, void *msgp, size_t msgsz, int msgflg);
#endif


#endif
