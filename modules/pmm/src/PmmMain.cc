//===================================================================================================================
//
//  PmmMain.cc -- The main entry point for the Physical Memory Manager
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  Update the bitmap that the frame is available to be used somewhere else.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Oct-30  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "libc.h"
#include "pmm.h"


//
// -- pmmBitmap is declared as a pointer to an address; the actual data will be provided by the kernel
//    by mapping a page with the data.
//    ------------------------------------------------------------------------------------------------
uint32_t *pmmBitmap;


//
// -- This is the PMM dispatcher
//    --------------------------
extern "C" void PmmMain(void)
{
    Message_t message;
    Message_t reply = {0};

    // -- we can set this up only until we receive the bitmap
    message.dataPayload = pmmBitmap;
    message.payloadSize = BITMAP_SIZE;

    while (true) {
        ReceiveMessage(&message);

        switch (message.msg) {
        case PMM_FREE_FRAME:
            PmmFreeFrame(message.parm1);
            break;

        case PMM_ALLOC_FRAME:
            PmmAllocFrame(message.parm1);
            break;

        case PMM_FREE_RANGE:
            PmmFreeFrameRange(message.parm1, message.parm2);
            break;

        case PMM_ALLOC_RANGE:
            PmmAllocFrameRange(message.parm1, message.parm2);
            break;

        case PMM_NEW_FRAME:
            reply.msg = PMM_NEW_FRAME;
            reply.parm1 = PmmNewFrame();
            SendMessage(message.pid, &reply);
            break;

        case PMM_INIT:
            // -- clear the buffer so that we do not get a new message that overwrites the bitmap
            message.dataPayload = NULL;
            message.payloadSize = 0;
            break;

        default:
            break;
        }
    }
}
