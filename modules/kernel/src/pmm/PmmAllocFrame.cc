//===================================================================================================================
//
//  PmmAllocFrame.cc -- Request the PMM to allocate a frame
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  This function will send a message to the PMM to allocate a frame, and will wait for a response from the PMM.
//
//  This is a very trivial function that does not ensure that the returned message really came from the PMM.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-10  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "ipc.h"
#include "cpu.h"
#include "pmm.h"


//
// -- Get a new frame from the PMM
//    ----------------------------
frame_t PmmAllocFrame(void)
{
    Message_t m;

    kMemSetB(&m, 0, sizeof(Message_t));
    m.msg = PMM_NEW_FRAME;

    MessageSend(PID_PMM, &m);
    MessageReceive(&m);

    return m.msg;
}
