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
EXTERN_C HIDDEN KERNEL
void SpurriousPic(isrRegs_t *regs) { }


//
// -- Initialize the IO APIC
//    ----------------------
EXTERN_C EXPORT LOADER
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

    IsrRegister(IPI_PANIC, (isrFunc_t)Halt);
    IsrRegister(IPI_TLB_FLUSH, IpiHandleTlbFlush);
    IsrRegister(0xf2, SpurriousPic);
    IsrRegister(0xf3, SpurriousPic);
    IsrRegister(0xf4, SpurriousPic);
    IsrRegister(0xf5, SpurriousPic);
    IsrRegister(0xf6, SpurriousPic);
    IsrRegister(0xf7, SpurriousPic);
    IsrRegister(0xf8, SpurriousPic);
    IsrRegister(0xf9, SpurriousPic);
    IsrRegister(0xfa, SpurriousPic);
    IsrRegister(0xfb, SpurriousPic);
    IsrRegister(0xfc, SpurriousPic);
    IsrRegister(0xfd, SpurriousPic);
    IsrRegister(0xfe, SpurriousPic);
    IsrRegister(IPI_DEBUGGER, IpiHandleDebugger);


    IoApicDeviceData_t *data = (IoApicDeviceData_t*)dev->device.deviceData;

    for (int i = 0; i < count; i ++) {
        archsize_t addr = GetIoapicAddr(i);
        Ioapicid_t apicid;
        Ioapicver_t apicver;
        Ioapicredtbl_t apicredir;

        MmuMapToFrame(addr, addr>>12, PG_DEVICE | PG_KRN | PG_WRT);

        apicid.reg = IoapicRead(addr,IOAPICID);
        apicver.reg = IoapicRead(addr, IOAPICVER);

        kprintf("IOAPIC located at: %p\n", addr);
        kprintf("  The APIC ID is %x\n", apicid.apicId);
        kprintf("  The APIC Version is %x; the max redir is %x\n", apicver.version, apicver.maxRedir);

        for (int j = 0; j <= apicver.maxRedir; j ++) {
            apicredir.reg0 = IoapicRead(addr, IOREDTBL0 + (j * 2));
            apicredir.reg1 = IoapicRead(addr, IOREDTBL0 + (j * 2) + 1);

            kprintf("  Redirection table entry %x: %p %p\n", j, apicredir.reg1, apicredir.reg0);
        }

        data->ioapicBase = addr;        // TODO: fix this; assumes 1 IOAPIC
    }

    data->localApicBase = RDMSR(0x1b) & 0xfffff000;
    MmuMapToFrame(data->localApicBase, data->localApicBase>>12, PG_DEVICE | PG_KRN | PG_WRT);
}
