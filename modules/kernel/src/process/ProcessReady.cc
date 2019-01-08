//===================================================================================================================
//
//  ProcessReady.cc -- Place a process into the proper queue based on its priority.
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  This process must be on the ready queue if it is on a queue at all.  Therefore it is the responsibility of any
//  calling function to remove it from its queue if it is on it.
//
//  Unless the process is ending, the process status will be updated to PROC_RUN.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Oct-24  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "printf.h"
#include "spinlock.h"
#include "process.h"

#include <errno.h>


//
// -- Ready a process by placing it on the proper ready queue
//    -------------------------------------------------------
int ProcessReady(PID_t pid)
{
    Process_t *proc;

    SANITY_CHECK_PID(pid, proc);
    SPIN_BLOCK(proc->lock) {
        if (proc->isHeld) {
            SPIN_RLS(proc->lock);
            return -EUNDEF;
        }

        switch(proc->status) {
        case PROC_END:
        case PROC_ZOMB:
            SPIN_RLS(proc->lock);
            return -EUNDEF;

        case PROC_INIT:
        case PROC_DLYW:
        case PROC_MSGW:
        case PROC_MTXW:
        case PROC_SEMW:
        case PROC_RUN:
            switch(proc->priority) {
            case PTY_OS:
                SPIN_BLOCK(procOsPtyList.lock) {
                    ListAddTail(&procOsPtyList, &proc->stsQueue);
                    SPIN_RLS(procOsPtyList.lock);
                }
                break;

            case PTY_HIGH:
                SPIN_BLOCK(procHighPtyList.lock) {
                    ListAddTail(&procHighPtyList, &proc->stsQueue);
                    SPIN_RLS(procHighPtyList.lock);
                }
                break;

            case PTY_NORM:
                SPIN_BLOCK(procNormPtyList.lock) {
                    ListAddTail(&procNormPtyList, &proc->stsQueue);
                    SPIN_RLS(procNormPtyList.lock);
                }
                break;

            case PTY_LOW:
                SPIN_BLOCK(procLowPtyList.lock) {
                    ListAddTail(&procLowPtyList, &proc->stsQueue);
                    SPIN_RLS(procLowPtyList.lock);
                }
                break;

            case PTY_IDLE:
                SPIN_BLOCK(procIdlePtyList.lock) {
                    ListAddTail(&procIdlePtyList, &proc->stsQueue);
                    SPIN_RLS(procIdlePtyList.lock);
                }
                break;

            default:
                SPIN_RLS(proc->lock);
                return -EUNDEF;
            }

            break;

        default:
            SPIN_RLS(proc->lock);
            return -EUNDEF;
        }

        SPIN_RLS(proc->lock);
    }

    if (pid != currentPID && proc->priority > procs[currentPID]->priority) ProcessSwitch(procs[currentPID], proc);

    return SUCCESS;
}
