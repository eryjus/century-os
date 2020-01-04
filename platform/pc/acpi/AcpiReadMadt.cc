//===================================================================================================================
//
//  AcpiReadMadt.cc -- Read the MADT table and determine what we will do with the information
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Apr-07  Initial   0.4.1   ADCL  Initial version
//
//===================================================================================================================


#include "printf.h"
#include "hw-disc.h"
#include "lists.h"              // for MEMBER_OFFSET
#include "hardware.h"


//
// -- Read the ACPI MADT Table, and figure out what it means to CenturyOS
//    -------------------------------------------------------------------
EXTERN_C EXPORT LOADER
void AcpiReadMadt(archsize_t loc)
{
    MADT_t *madt = (MADT_t *)loc;
    kprintf(".... MADT table length is %p\n", madt->length);
    kprintf(".... MADT flags are %p\n", madt->flags);
    kprintf(".... MADT Local IC Address is %p\n", madt->localIntCtrlAddr);

    uint8_t *wrk = (uint8_t *)(loc + MEMBER_OFFSET(MADT_t,intCtrlStructs));
    uint8_t *first = wrk;
    while (wrk - first < (long)(madt->length - MEMBER_OFFSET(MADT_t,intCtrlStructs))) {
        uint8_t len = wrk[1];

        switch(wrk[0]) {
        case MADT_PROCESSOR_LOCAL_APIC:
            {
                MadtLocalApic_t *local = (MadtLocalApic_t *)wrk;
                cpus.cpusDiscovered ++;
                IncLocalApic();

                kprintf(".... MADT_PROCESSOR_LOCAL_APIC\n");
                kprintf("...... Proc ID %x; APIC ID %x; %s (%d found so far)\n", local->procId, local->apicId,
                        local->flags&1?"enabled":"disabled", cpus.cpusDiscovered);
            }

            break;

        case MADT_IO_APIC:
            {
                MadtIoApic_t *local = (MadtIoApic_t *)wrk;
                kprintf(".... MADT_IO_APIC\n");
                kprintf("...... APIC Addr: %p, Global Sys Int Base: %x\n", local->ioApicAddr, local->gsiBase);
                AddIoapic(local->ioApicAddr, local->gsiBase);
            }

            break;

        case MADT_INTERRUPT_SOURCE_OVERRIDE:
            {
                MadtIntSrcOverride_t *local = (MadtIntSrcOverride_t *)wrk;
                kprintf(".... MADT_INTERRUPT_SOURCE_OVERRIDE\n");
                kprintf("...... source: %x, Global Sys Int: %x\n", (uint32_t)local->source, local->gsInt);
                kprintf("...... Polarity: %x; Trigger Mode: %x\n", local->flags & 0x03, (local->flags >> 2) & 0x03);
            }

            break;

        case MADT_NMI_SOURCE:
            {
                MadtMNISource_t *local = (MadtMNISource_t *)wrk;
                kprintf(".... MADT_NMI_SOURCE\n");
                kprintf("...... Global Sys Int: %x\n", local->gsInt);
                kprintf("...... Polarity: %x; Trigger Mode: %x\n", local->flags & 0x03, (local->flags >> 2) & 0x03);
            }

            break;

        case MADT_LOCAL_APIC_NMI:
            {
                MadtLocalApicNMI_t *local = (MadtLocalApicNMI_t *)wrk;
                kprintf(".... MADT_LOCAL_APIC_NMI\n");
                kprintf("...... APIC Proc ID: %x; local INT#: %x\n", (uint32_t)local->procId, (uint32_t)local->localLINT);
                kprintf("...... Polarity: %x; Trigger Mode: %x\n", local->flags & 0x03, (local->flags >> 2) & 0x03);
            }

            break;

        case MADT_LOCAL_APIC_ADDRESS_OVERRIDE:
            kprintf("!!!! MADT IC Table Type MADT_LOCAL_APIC_ADDRESS_OVERRIDE is not supported\n");
            break;

        case MADT_IO_SAPIC:
            kprintf("!!!! MADT IC Table Type MADT_IO_SAPIC is not supported\n");
            break;

        case MADT_LOCAL_SAPIC:
            kprintf("!!!! MADT IC Table Type MADT_LOCAL_SAPIC is not supported\n");
            break;

        case MADT_PLATFORM_INTERRUPT_SOURCES:
            kprintf("!!!! MADT IC Table Type MADT_PLATFORM_INTERRUPT_SOURCES is not supported\n");
            break;

        case MADT_PROCESSOR_LOCAL_X2APIC:
            kprintf("!!!! MADT IC Table Type MADT_PROCESSOR_LOCAL_X2APIC is not supported\n");
            break;

        case MADT_LOCAL_X2APIC_NMI:
            kprintf("!!!! MADT IC Table Type MADT_LOCAL_X2APIC_NMI is not supported\n");
            break;

        case MADT_GIC:
            kprintf("!!!! MADT IC Table Type GIC is not supported\n");
            break;

        case MADT_GICD:
            kprintf("!!!! MADT IC Table Type GICD is not supported\n");
            break;

        default:
            kprintf("!!!! Unknown MADT IC Table Type: %x\n", (uint32_t)wrk[0]);
            break;
        }


        wrk += len;
    }
}


