//===================================================================================================================
//
//  sys/ipc.h -- A POSIX-compliant header for common XSI IPC structures
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  This file is provided as a POSIX-compliant ipc-related system call implementaiton.  This file will be copied
//  to the target file structure.
//
//  See `man sys_ipc.h` for more information -- I am not going to duplicate that work here.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Apr-29  Initial   0.4.3   ADCL  Initial version
//
//===================================================================================================================


#ifndef __IPC_H__
#define __IPC_H__


#include <stdint.h>


//
// -- `uid_t` must be defined in this file; we will use a 32-bit unsigned int from <stdint.h>
//    ---------------------------------------------------------------------------------------
#ifndef __uid_t_defined__
typedef uint32_t uid_t;
#define __uid_t_defined__
#endif


//
// -- `gid_t` must be defined in this file; we will use a 32-bit unsigned int from <stdint.h>
//    ---------------------------------------------------------------------------------------
#ifndef __gid_t_defined__
typedef uint32_t gid_t;
#define __gid_t_defined__
#endif


//
// -- `mode_t` must be defined in this file; use a 32-bit unsigned int from <stdint.h>
//    --------------------------------------------------------------------------------
#ifndef __mode_t_defined__
typedef uint32_t mode_t;
#define __mode_t_defined__
#endif


//
// -- `key_t` must be defined in this file; use a 32-bit signed in from <stdint.h>
//    ----------------------------------------------------------------------------
#ifndef __key_t_defined__
typedef int32_t key_t;
#define __key_t_defined__
#endif


//
// -- This is the IPC Permissions structure
//    -------------------------------------
struct ipc_perm {
    uid_t uid;          // -- owner's uid
    gid_t gid;          // -- owner's gid
    uid_t cuid;         // -- creator's uid
    gid_t cgid;         // -- creator's gid
    mode_t mode;        // -- read/write permissions
};


//
// -- Mode bits for `msgget`, `semget`, and `shmget`
//    ----------------------------------------------

// -- Create key if it does not exist
#define IPC_CREAT   0x0200

// -- Fail if the key does exist
#define IPC_EXCL    0x0400

// -- Return error on wait
#define IPC_NOWAIT  0x0800


//
// -- Control commands for `msgctl`, `semctl`, and `shmctl`
//    -----------------------------------------------------

// -- Remove the identifier
#define IPC_RMID    0

// -- Set ipc_perm options
#define IPC_SET     1

// -- Get ipc_perm options
#define IPC_STAT    2


//
// -- Special key_t value -- Private key
//    ----------------------------------
#define IPC_PRIVATE	((key_t) 0)


#ifndef __CENTURY_KERNEL__
//
// -- for a file name and a project ID (bits 7:0 only!), create a unique key
//    ----------------------------------------------------------------------
extern key_t ftok(const char *path, int project);
#endif


#endif

