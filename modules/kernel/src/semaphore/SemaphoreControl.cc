//===================================================================================================================
//
//  SemaphoreControl.cc -- The implementation of the POSIX `semctl` API -- control semaphores
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  Programmer's note:
//  While I use hex numbers quite a bit to express bit flags, this file contains several octal numbers.  Be aware
//  of this fact as you are reading this source.
//
//  POSIX variances:
//  A) Permissions are not yet implemented.  Therefore, all processes have all permissions.
//  B) Times are not yet maintained and will all be reported as 0.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-May-01  Initial   0.4.3   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "errno.h"
#include "mmu.h"
#include "user-group.h"
#include "lists.h"
#include "process.h"
#include "heap.h"
#include "spinlock.h"
#include "semaphore.h"


//
// -- Semaphore Control API
//    ---------------------
int __krntext SemaphoreControl(int semid, int semnum, int cmd, union semun semun)
{
    int rv = 0;         // -- used for some cases

    SPIN_BLOCK(semaphoreAll.globalLock) {
        SemaphoreSet_t *set = semaphoreAll.semaphoreSets[semid];
        if (set == NULL) {
            SPIN_RLS(semaphoreAll.globalLock);
            return -EINVAL;
        }

        switch(cmd) {
        case IPC_STAT:      // -- get the current semaphore settings
            {
                struct semid_ds *buf = semun.buf;
                if (!IS_MAPPED(buf, sizeof(semid_ds))) {
                    SPIN_RLS(semaphoreAll.globalLock);
                    return -EFAULT;
                }

                if (!UidHasPermission(0, 01)) {      // check read permissions
                    SPIN_RLS(semaphoreAll.globalLock);
                    return -EACCES;
                }

                SPIN_BLOCK(set->lock) {
                    SPIN_RLS(semaphoreAll.globalLock);          // release the bigger lock when we have the lower one
                    buf->sem_perm = set->permissions;
                    buf->sem_nsems = set->numSem;
                    buf->sem_otime = set->semOtime;
                    buf->sem_ctime = set->semCtime;
                    SPIN_RLS(set->lock);
                }
            }

            return semid;

        case IPC_SET:       // -- set some new semaphore settings
            {
                struct semid_ds *buf = semun.buf;
                if (!IS_MAPPED(buf, sizeof(semid_ds))) {
                    SPIN_RLS(semaphoreAll.globalLock);
                    return -EFAULT;
                }

                if ((GetUid() == 0 || GetUid() == set->owner || GetUid() == set->creator) == false)  {
                    SPIN_RLS(semaphoreAll.globalLock);
                    return -EPERM;
                }

                SPIN_BLOCK(set->lock) {
                    SPIN_RLS(semaphoreAll.globalLock);          // release the bigger lock when we have the lower one
                    set->permissions.uid = buf->sem_perm.uid;
                    set->permissions.gid = buf->sem_perm.gid;
                    set->permissions.mode &= ~0777;     // clear previous permissions
                    set->permissions.mode |= (buf->sem_perm.mode & 0777);
                    set->semCtime = 0;                  // TODO: set this to the current time
                    SPIN_RLS(set->lock);
                }
            }

            return 0;

        case IPC_RMID:      // -- remove the semaphore set
            {
                if ((GetUid() == 0 || GetUid() == set->owner || GetUid() == set->creator) == false)  {
                    SPIN_RLS(semaphoreAll.globalLock);
                    return -EPERM;
                }

                semaphoreAll.semaphoreSets[semid] = NULL;
                SPIN_RLS(semaphoreAll.globalLock);      // no one else can find this sem set now...
                SPIN_BLOCK(set->lock);      // use this to wait for all other locks to release; never released

                for (int i = 0; i < set->numSem; i ++) {
                    Semaphore_t *sem = &set->semSet[i];

                    while (!IsListEmpty(&sem->waitN)) {
                        Process_t *proc = FIND_PARENT(sem->waitN.list.next, Process_t, stsQueue);
                        ListRemoveInit(sem->waitN.list.next);
                        proc->pendingErrno = -EIDRM;
                        ProcessReady(proc);
                    }

                    while (!IsListEmpty(&sem->waitZ)) {
                        Process_t *proc = FIND_PARENT(sem->waitZ.list.next, Process_t, stsQueue);
                        ListRemoveInit(sem->waitZ.list.next);
                        proc->pendingErrno = -EIDRM;
                        ProcessReady(proc);
                    }
                }

                SPIN_BLOCK(semaphoreAll.undoList.lock) {
                    ListHead_t::List_t *wrk = semaphoreAll.undoList.list.next;
                    while (wrk != &semaphoreAll.undoList.list) {
                        SemaphoreUndo_t *undo = FIND_PARENT(wrk, SemaphoreUndo_t, list);
                        wrk = wrk->next;

                        if (undo->semid == semid && undo->key == set->key) {
                            ListRemoveInit(&undo->list);
                            HeapFree(undo);
                        }
                    }

                    SPIN_RLS(semaphoreAll.undoList.lock);
                }

                HeapFree(set->semSet);
                HeapFree(set);
            }

            return 0;

        case GETALL:        // get all the current counts -- atomically
            {
                unsigned short *array = semun.array;
                if (!IS_MAPPED(array, sizeof(unsigned short) * set->numSem)) {
                    SPIN_RLS(semaphoreAll.globalLock);
                    return -EFAULT;
                }

                if (!UidHasPermission(0, 01)) {      // check read permissions
                    SPIN_RLS(semaphoreAll.globalLock);
                    return -EACCES;
                }

                SPIN_BLOCK(set->lock) {
                    SPIN_RLS(semaphoreAll.globalLock);
                    for (int i = 0; i < set->numSem; i ++) {
                        array[i] = AtomicRead(&set->semSet[i].semval);
                    }

                    SPIN_RLS(set->lock);
                }
            }

            return 0;

        case GETNCNT:       // -- get the waitN count
            {
                if (!UidHasPermission(0, 01)) {      // -- check read permissions
                    SPIN_RLS(semaphoreAll.globalLock);
                    return -EACCES;
                }

                if (semnum < 0 || semnum > set->numSem - 1) {
                    SPIN_RLS(semaphoreAll.globalLock);
                    return -EINVAL;
                }

                rv = set->semSet[semnum].waitN.count;   // -- no need to get lower-level locks; this is quick
                SPIN_RLS(semaphoreAll.globalLock);
            }

            return rv;

        case GETPID:        // -- get the pid of the last process to update
            {
                if (!UidHasPermission(0, 01)) {      // -- check read permissions
                    SPIN_RLS(semaphoreAll.globalLock);
                    return -EACCES;
                }

                if (semnum < 0 || semnum > set->numSem - 1) {
                    SPIN_RLS(semaphoreAll.globalLock);
                    return -EINVAL;
                }

                SPIN_BLOCK(set->lock) {
                    SPIN_RLS(semaphoreAll.globalLock);

                    rv = set->semSet[semnum].semPid;

                    SPIN_RLS(set->lock);
                }
            }

            return rv;

        case GETVAL:        // -- get the value of the nth semaphore
            {
                if (!UidHasPermission(0, 01)) {      // -- check read permissions
                    SPIN_RLS(semaphoreAll.globalLock);
                    return -EACCES;
                }

                if (semnum < 0 || semnum > set->numSem - 1) {
                    SPIN_RLS(semaphoreAll.globalLock);
                    return -EINVAL;
                }

                SPIN_BLOCK(set->lock) {
                    SPIN_RLS(semaphoreAll.globalLock);

                    rv = AtomicRead(&set->semSet[semnum].semval);

                    SPIN_RLS(set->lock);
                }
            }

            return rv;

        case GETZCNT:       // -- get the waitZ count
            {
                if (!UidHasPermission(0, 01)) {      // -- check read permissions
                    SPIN_RLS(semaphoreAll.globalLock);
                    return -EACCES;
                }

                if (semnum < 0 || semnum > set->numSem - 1) {
                    SPIN_RLS(semaphoreAll.globalLock);
                    return -EINVAL;
                }

                rv = set->semSet[semnum].waitZ.count;   // -- no need to get lower-level locks; this is quick
                SPIN_RLS(semaphoreAll.globalLock);
            }

            return rv;

        case SETALL:
            {
                if (!UidHasPermission(0, 02)) {      // -- check write permissions
                    SPIN_RLS(semaphoreAll.globalLock);
                    return -EACCES;
                }

                Semaphore_t *sem = &set->semSet[semnum];
                int val = semun.val;

                SPIN_BLOCK(set->lock) {
                    SPIN_RLS(semaphoreAll.globalLock);
                    set->semCtime = 0;                  // TODO: set the time value

                    for (int i = 0; i < set->numSem; i ++) {
                        int prev;
                        prev = AtomicRead(&sem->semval);
                        AtomicSet(&sem->semval, val);

                        if (val == 0) {
                            SPIN_BLOCK(sem->waitZ.lock) {
                                if (!IsListEmpty(&sem->waitZ)) {
                                    Process_t *proc = FIND_PARENT(sem->waitZ.list.next, Process_t, stsQueue);
                                    ListRemoveInit(&proc->stsQueue);
                                    ProcessReady(proc);
                                }

                                SPIN_RLS(sem->waitZ.lock);
                            }
                        } else if (val > prev) {
                            SPIN_BLOCK(sem->waitN.lock) {
                                if (!IsListEmpty(&sem->waitN)) {
                                    Process_t *proc = FIND_PARENT(sem->waitN.list.next, Process_t, stsQueue);
                                    ListRemoveInit(&proc->stsQueue);
                                    ProcessReady(proc);
                                }

                                SPIN_RLS(sem->waitN.lock);
                            }
                        }

                        SPIN_BLOCK(semaphoreAll.undoList.lock) {
                            ListHead_t::List_t *wrk = semaphoreAll.undoList.list.next;
                            while (wrk != &semaphoreAll.undoList.list) {
                                SemaphoreUndo_t *undo = FIND_PARENT(wrk, SemaphoreUndo_t, list);
                                wrk = wrk->next;

                                if (undo->semid == semid && undo->key == set->key && undo->semnum == i) {
                                    undo->semadj = 0;
                                }
                            }
                        }

                    }

                    SPIN_RLS(set->lock);
                }
            }

            return 0;

        case SETVAL:
            {
                if (!UidHasPermission(0, 02)) {      // -- check write permissions
                    SPIN_RLS(semaphoreAll.globalLock);
                    return -EACCES;
                }

                if (semnum < 0 || semnum > set->numSem - 1) {
                    SPIN_RLS(semaphoreAll.globalLock);
                    return -EINVAL;
                }

                Semaphore_t *sem = &set->semSet[semnum];
                int val = semun.val;
                int prev;

                SPIN_BLOCK(set->lock) {
                    SPIN_RLS(semaphoreAll.globalLock);
                    set->semCtime = 0;                  // TODO: set the time value
                    prev = AtomicRead(&sem->semval);
                    AtomicSet(&sem->semval, val);

                    if (val == 0) {
                        SPIN_BLOCK(sem->waitZ.lock) {
                            if (!IsListEmpty(&sem->waitZ)) {
                                Process_t *proc = FIND_PARENT(sem->waitZ.list.next, Process_t, stsQueue);
                                ListRemoveInit(&proc->stsQueue);
                                ProcessReady(proc);
                            }

                            SPIN_RLS(sem->waitZ.lock);
                        }
                    } else if (val > prev) {
                        SPIN_BLOCK(sem->waitN.lock) {
                            if (!IsListEmpty(&sem->waitN)) {
                                Process_t *proc = FIND_PARENT(sem->waitN.list.next, Process_t, stsQueue);
                                ListRemoveInit(&proc->stsQueue);
                                ProcessReady(proc);
                            }

                            SPIN_RLS(sem->waitN.lock);
                        }
                    }

                    SPIN_BLOCK(semaphoreAll.undoList.lock) {
                        ListHead_t::List_t *wrk = semaphoreAll.undoList.list.next;
                        while (wrk != &semaphoreAll.undoList.list) {
                            SemaphoreUndo_t *undo = FIND_PARENT(wrk, SemaphoreUndo_t, list);
                            wrk = wrk->next;

                            if (undo->semid == semid && undo->key == set->key && undo->semnum == semnum) {
                                undo->semadj = 0;
                            }
                        }

                        SPIN_RLS(semaphoreAll.undoList.lock);
                    }

                    SPIN_RLS(set->lock);
                }
            }

            return 0;

        default:
            SPIN_RLS(semaphoreAll.globalLock);
            return -EINVAL;
        }
    }

    return -EUNDEF;         // -- should never get here, but just in case
}

