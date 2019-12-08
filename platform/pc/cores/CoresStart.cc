
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
    //
    // -- Load the trampoline code into the low 1MB of memory
    //    ---------------------------------------------------
    uint8_t *trampoline = (uint8_t *)0x8000;        // for S&G, start at 32K
    extern uint32_t intTableAddr;
    extern uint8_t _smpStart[];
    extern uint8_t _smpEnd[];

    kprintf("Copying the AP entry code to %p\n", trampoline);
    kprintf("... start at %p\n", _smpStart);
    kprintf("... length is %p\n", _smpEnd - _smpStart);
    kMemMove(trampoline, _smpStart, _smpEnd - _smpStart);

    // -- Patch up some memory locations
    *((uint32_t *)(&trampoline[14])) = intTableAddr;
    *((uint32_t *)(&trampoline[20])) = intTableAddr + 0x800;

    for (int i = 1; i < cpus.cpusDiscovered; i ++) {
        picControl->PicBroadcastInit(picControl, i);
        picControl->PicBroadcastSipi(picControl, i, (archsize_t)trampoline);
    }
}
