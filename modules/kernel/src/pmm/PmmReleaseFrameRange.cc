//===================================================================================================================
//
//  PmmReleaseFrameRange.cc -- Release a frame and place it in the scrub queue
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  The design of the PMM has changed.  I am now storing the PMM data in the frames themselves.  Therefore, the
//  frame will need to be mapped into the `insert` member and then the stack info populated from the values passed
//  in, along with the frame number from the next member in the stack.  Then the frame will need to be unmapped
//  and then remapped to the top of the proper stack.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Mar-11  Initial   0.3.1   ADCL  Initial version
//  2020-Apr-12   #405    v0.6.1c  ADCL  Redesign the PMM to store the stack in the freed frames themselves
//
//===================================================================================================================


#include "types.h"
#include "spinlock.h"
#include "heap.h"
#include "butler.h"
#include "msgq.h"
#include "pmm.h"


//
// -- Add the frames to the scrub queue
//    ---------------------------------
EXTERN_C EXPORT KERNEL
void PmmReleaseFrameRange(const frame_t frame, const size_t count)
{
    // -- there are 2 locks to get
    archsize_t flags = SPINLOCK_BLOCK_NO_INT(pmm.scrubLock) {
        PmmPush(pmm.scrubStack, frame, count);
        SPINLOCK_RLS_RESTORE_INT(pmm.scrubLock, flags);
    }

    AtomicAdd(&pmm.framesAvail, count);
    MessageQueueSend(butlerMsgq, BUTLER_CLEAN_PMM, 0, 0);
}

