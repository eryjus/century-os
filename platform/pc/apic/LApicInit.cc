//===================================================================================================================
//
//  LApicInit.cc -- Initialize the Local APIC (timer)
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Apr-24  Initial   0.4.1   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "interrupt.h"
#include "printf.h"
#include "mmu.h"
#include "cpu.h"
#include "pic.h"


//
// -- This is the spurious IRQ handler
//    --------------------------------
static void __krntext LApicSpurious(isrRegs_t *regs)
{
    kprintf("!");
}


//
// -- this is used during initialization to calibrate the timer
//    ---------------------------------------------------------
static void __ldrtext LApicInitTimeout(isrRegs_t *regs)
{

}


void __ldrtext _LApicInit(TimerDevice_t *dev, uint32_t freq)
{
    if (!dev) return;

    kprintf("Local APIC Init\n");

    // -- get the per cpu address
    dev->pic = picControl;
    dev->base = (TimerBase_t)LAPIC_MMIO;          // TODO: re-enable this line
    archsize_t base = dev->base;


    MmuDumpTables(LAPIC_MMIO);


    //
    // -- Take care of the initialization for the Local APIC address
    //    ----------------------------------------------------------
    kprintf("Base is %p whereas LAPIC_MMIO is %p\n", base, LAPIC_MMIO);
    kprintf("The value coming from MSR `0x1b` is: %p\n", RDMSR(0x1b));
    kprintf("The address of the target location is: %p\n", LAPIC_MMIO);
    WRMSR(0x1b, LAPIC_MMIO | (1<<11) | (RDMSR(0x1b) & 0xfff));          // -- include global enable just in case
    kprintf("The updated value fpr MSR `0x1b` is: %p\n", RDMSR(0x1b));
    MmuMapToFrame(LAPIC_MMIO, LAPIC_MMIO >> 12, PG_DEVICE | PG_KRN | PG_WRT);


    //
    // -- SW enable the Local APIC timer
    //    ------------------------------
    MmioWrite(base + LAPIC_ESR, 0);
    kprintf(".. Before setting the spurious interrupt at %p, the value is %p\n", base + LAPIC_SPURIOUS_VECT,
            MmioRead(base + LAPIC_SPURIOUS_VECT));
    __asm volatile("nop\n");
    MmioWrite(base + LAPIC_SPURIOUS_VECT, 39 | (1<<8));
    __asm volatile("nop\n");
    kprintf(".. After setting the spurious interrupt at %p, the value is %p\n", base + LAPIC_SPURIOUS_VECT,
            MmioRead(base + LAPIC_SPURIOUS_VECT));
    kprintf(".. The LAPIC error register is: %p\n", MmioRead(base + LAPIC_ESR));




    IsrRegister(32, LApicInitTimeout);          // this is temporary until we get calibrated
    IsrRegister(39, LApicSpurious);

    // -- here we initialize the LAPIC to a defined state -- taken from Century32
    MmioWrite(base + LAPIC_DEST_FMT, 0xffffffff);       // ipi flat model??
    MmioWrite(base + LAPIC_LOGICAL_DEST, MmioRead(base + LAPIC_LOGICAL_DEST) | (1<<24));    // set logical apic to 1
    MmioWrite(base + LAPIC_LVT_TMR, (1<<16));           // mask the timer during setup
    MmioWrite(base + LAPIC_LVT_PERF, (1<<16));
    MmioWrite(base + LAPIC_LVT_LINT0, (1<<16));
    MmioWrite(base + LAPIC_LVT_LINT1, (1<<16));
    MmioWrite(base + LAPIC_LVT_ERR, (1<<16));
    MmioWrite(base + LAPIC_TPR, 0);
    MmioWrite(base + LAPIC_TMRDIV, 0x03);       // divide value is 16
    MmioWrite(base + LAPIC_LVT_TMR, 32);        // timer is vector 32; now unmasked

    // -- enable the PIC timer in one-shot mode
    outb(0x61, (inb(0x61) & 0xfd) | 1);
    outb(0x43, 0xb2);

    //
    // -- So, here is the math:
    //    We need to divide the clock by 20 to have a value large enough to get a decent time.
    //    So, we will be measuring 1/20th of a second.
    // -- 1193180 Hz / 20 == 59659 cycles == e90b cycles
    outb(0x42, 0x0b);
    inb(0x60);      // short delay
    outb(0x42, 0xe9);

    // -- now reset the PIT timer and start counting
    uint8_t tmp = inb(0x61) & 0xfe;
    outb(0x61, tmp);
    outb(0x61, tmp | 1);

    kprintf("Prior to calibration, the timer count is %p\n", MmioRead(base + LAPIC_TMRCURRCNT));

    // -- reset the APIC counter to -1
    MmioWrite(base + LAPIC_TMRINITCNT, 0xffffffff);

    kprintf("During calibration, the timer count is %p\n", MmioRead(base + LAPIC_TMRCURRCNT));

    while (!(inb(0x61) & 0x20)) {}  // -- busy wait here

    MmioWrite(base + LAPIC_LVT_TMR, (1<<16));

    // -- disable the PIC
    outb(0x21, 0xff);
    outb(0xa1, 0xff);


    //
    // -- Now we can calculate the cpu frequency, converting back to a full second
    //    ------------------------------------------------------------------------
    uint64_t cpuFreq = (0xffffffff - MmioRead(base + LAPIC_TMRCURRCNT)) * 16 * 20;
    uint64_t factor = cpuFreq / freq / 16;

    if ((factor >> 32) != 0) {
        kprintf("PANIC: The factor is too large for the architecture!\n");
        HaltCpu();
    }

    kprintf("So, the calculated clock divider is %p\n", (uint32_t)factor);

    // -- This will also unmask IRQ0 with the PIC, so nothing else should be needed
    IsrRegister(32, dev->TimerCallBack);

    //
    // -- Now, program the Timer
    //    ----------------------
    MmioWrite(base + LAPIC_TMRINITCNT, factor);
    MmioWrite(base + LAPIC_LVT_TMR, 32 | (0b01<<17));
}



