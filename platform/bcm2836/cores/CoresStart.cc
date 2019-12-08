
#include "types.h"
#include "timer.h"
#include "hardware.h"
#include "pic.h"
#include "printf.h"
#include "mmu.h"
#include "cpu.h"


extern "C" void entryAp(void);

__CENTURY_FUNC__ void CoresStart(void)
{
    for (int i = 1; i < cpus.cpusDiscovered; i ++) {
        kprintf("Starting core with message to %p\n", IPI_MAILBOX_BASE + 0x0c + (0x10 * i));
        MmioWrite(IPI_MAILBOX_BASE + 0x0c + (0x10 * i), (uint32_t)entryAp);
        SEV();
    }
}