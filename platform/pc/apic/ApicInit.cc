//===================================================================================================================
//
//  ApicInit.cc -- Initialize the IO APIC
//
//        Copyright (c)  2017-2019 -- Adam Clark
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
#include "hw-disc.h"
#include "mmu.h"
#include "pic.h"


//
// -- Initialize the IO APIC
//    ----------------------
void _ApicInit(PicDevice_t *dev, const char *name)
{
    int count = GetIoapicCount();

    if (count > MAX_IOAPIC) {
        kprintf("WARNING! The number of IOAPICs is greater than the compiled support\n");
        kprintf("         %x are supported; %x were found\n", MAX_IOAPIC, count);
        count = MAX_IOAPIC;
    }

    //
    // -- disable the 8259 PIC
    //    --------------------
    outb(0xa1, 0xff);               // Disable all IRQs
    outb(0x21, 0xff);               // Disable all IRQs


    ApicDeviceData_t *data = (ApicDeviceData_t*)dev->device.deviceData;

    for (int i = 0; i < count; i ++) {
        archsize_t addr = GetIoapicAddr(i);
        Ioapicid_t apicid;
        Ioapicver_t apicver;
        Ioapicredtbl_t apicredir;

        MmuMapToFrame(addr, addr>>12, PG_DEVICE);

        apicid.reg = IOAPIC_READ(addr,IOAPICID);
        apicver.reg = IOAPIC_READ(addr, IOAPICVER);

        kprintf("IOAPIC located at: %p\n", addr);
        kprintf("  The APIC ID is %x\n", apicid.apicId);
        kprintf("  The APIC Version is %x; the max redir is %x\n", apicver.version, apicver.maxRedir);

        for (int j = 0; j <= apicver.maxRedir; j ++) {
            apicredir.reg0 = IOAPIC_READ(addr, IOREDTBL0 + (i * 2));
            apicredir.reg1 = IOAPIC_READ(addr, IOREDTBL0 + (i * 2) + 1);

            kprintf("  Redirection table entry %x: %p %p\n", j, apicredir.reg1, apicredir.reg0);
        }

        data->ioapicBase = addr;        // TODO: fix this; assumes 1 IOAPIC
    }

    data->localApicBase = RDMSR(0x1b) & 0xfffff000;
    MmuMapToFrame(data->localApicBase, data->localApicBase>>12, PG_DEVICE);
}
