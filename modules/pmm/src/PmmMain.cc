


#include "libc.h"
#include "pmm.h"


//
// -- This is the PMM dispatcher
//    --------------------------
extern "C" void PmmMain(void)
{
    while (true) {
        Message_t message;
        Message_t reply = {0};

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

        default:
            break;
        }
    }
}
