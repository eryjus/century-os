


#include "ipc.h"
#include "cpu.h"
#include "pmm.h"


frame_t PmmAllocFrame(void)
{
    Message_t m;

    kMemSetB(&m, 0, sizeof(Message_t));
    m.msg = PMM_NEW_FRAME;

    MessageSend(PID_PMM, &m);
    MessageReceive(&m);

    return m.msg;
}
