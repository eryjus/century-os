//===================================================================================================================
//
//  IoApicInit.cc -- Initialize the IO APIC
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Apr-20  Initial   0.4.1   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "interrupt.h"
#include "hw-disc.h"
#include "mmu.h"
#include "pic.h"


//
// -- Handle a spurioius interrupt from the 8259 -- just in case
//    ----------------------------------------------------------
static void SpurriousPic(isrRegs_t *regs) { }


//
// -- Initialize the IO APIC
//    ----------------------
void _IoApicInit(PicDevice_t *dev, const char *name)
{
    int count = GetIoapicCount();

    if (count > MAX_IOAPIC) {
        kprintf("WARNING! The number of IOAPICs is greater than the compiled support\n");
        kprintf("         %x are supported; %x were found\n", MAX_IOAPIC, count);
        count = MAX_IOAPIC;
    }


    //
    // -- For some buggy implementations, remap the 8259 PIC to space out of the way
    //    --------------------------------------------------------------------------
    outb(PIC1 + PIC_MASTER_DATA, 0xff);            // Disable all IRQs
    outb(PIC2 + PIC_SLAVE_DATA, 0xff);             // Disable all IRQs
    outb(PIC1 + PIC_MASTER_COMMAND, 0x11);
    outb(PIC2 + PIC_SLAVE_COMMAND, 0x11);
    outb(PIC1 + PIC_MASTER_DATA, 0xf0);
    outb(PIC2 + PIC_SLAVE_DATA, 0xf8);
    outb(PIC1 + PIC_MASTER_DATA, 0x04);
    outb(PIC2 + PIC_SLAVE_DATA, 0x02);
    outb(PIC1 + PIC_MASTER_DATA, 0x01);
    outb(PIC2 + PIC_SLAVE_DATA, 0x01);
    outb(PIC1 + PIC_MASTER_DATA, 0xff);            // Disable all IRQs
    outb(PIC2 + PIC_SLAVE_DATA, 0xff);             // Disable all IRQs

    IsrRegister(240, SpurriousPic);
    IsrRegister(241, SpurriousPic);
    IsrRegister(242, SpurriousPic);
    IsrRegister(243, SpurriousPic);
    IsrRegister(244, SpurriousPic);
    IsrRegister(245, SpurriousPic);
    IsrRegister(246, SpurriousPic);
    IsrRegister(247, SpurriousPic);
    IsrRegister(248, SpurriousPic);
    IsrRegister(249, SpurriousPic);
    IsrRegister(250, SpurriousPic);
    IsrRegister(251, SpurriousPic);
    IsrRegister(252, SpurriousPic);
    IsrRegister(253, SpurriousPic);
    IsrRegister(254, SpurriousPic);
    IsrRegister(255, SpurriousPic);


    IoApicDeviceData_t *data = (IoApicDeviceData_t*)dev->device.deviceData;

    for (int i = 0; i < count; i ++) {
        archsize_t addr = GetIoapicAddr(i);
        Ioapicid_t apicid;
        Ioapicver_t apicver;
        Ioapicredtbl_t apicredir;

        MmuMapToFrame(addr, addr>>12, PG_DEVICE | PG_KRN | PG_WRT);

        apicid.reg = IOAPIC_READ(addr,IOAPICID);
        apicver.reg = IOAPIC_READ(addr, IOAPICVER);

        kprintf("IOAPIC located at: %p\n", addr);
        kprintf("  The APIC ID is %x\n", apicid.apicId);
        kprintf("  The APIC Version is %x; the max redir is %x\n", apicver.version, apicver.maxRedir);

        for (int j = 0; j <= apicver.maxRedir; j ++) {
            apicredir.reg0 = IOAPIC_READ(addr, IOREDTBL0 + (j * 2));
            apicredir.reg1 = IOAPIC_READ(addr, IOREDTBL0 + (j * 2) + 1);

            kprintf("  Redirection table entry %x: %p %p\n", j, apicredir.reg1, apicredir.reg0);
        }

        data->ioapicBase = addr;        // TODO: fix this; assumes 1 IOAPIC
    }

    data->localApicBase = RDMSR(0x1b) & 0xfffff000;
    MmuMapToFrame(data->localApicBase, data->localApicBase>>12, PG_DEVICE | PG_KRN | PG_WRT);
}
