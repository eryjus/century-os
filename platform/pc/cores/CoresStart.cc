
#include "types.h"
#include "timer.h"
#include "hardware.h"
#include "process.h"
#include "printf.h"
#include "pmm.h"
#include "pic.h"


extern "C" void entryAp(void);

__CENTURY_FUNC__ void CoresStart(void)
{
    cpus.cpuCount = 1;

    //
    // -- Load the trampoline code into the low 1MB of memory
    //    ---------------------------------------------------
//    uint8_t *trampoline = (uint8_t *)0;             // in this case, we want address 0, not NULL;

    picControl->PicBroadcastInit(picControl);
    picControl->PicBroadcastSipi(picControl);
    picControl->PicBroadcastSipi(picControl);

    ProcessMilliSleep(500);

    kprintf("%x cores are now running\n", cpus.cpuCount);
}
