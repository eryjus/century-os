//===================================================================================================================
//
//  AcpiGetTableSig.cc -- Get the signature of the table at an address
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Apr-06  Initial   0.4.1   ADCL  Initial version
//
//===================================================================================================================


#include "printf.h"
#include "mmu.h"
#include "hardware.h"


//
// -- get the table signature (and check its valid); return 0 if invalid
//    ------------------------------------------------------------------
EXPORT LOADER
uint32_t AcpiGetTableSig(archsize_t loc)
{
    kprintf("Checking ACPI table at %p\n", loc);
    if (!MmuIsMapped(loc)) {
        MmuMapToFrame(loc, loc >> 12, PG_KRN);
    }

    if (!MmuIsMapped(loc) || loc == 0) {
        kprintf("... not mapped: skipping!\n");
        return 0;
    }

    uint32_t rv = *((uint32_t *)loc);

    if (!AcpiCheckTable(loc, rv)) return 0;

    switch(rv) {
    case MAKE_SIG("APIC"):
        kprintf(".. APIC: Multiple APIC Description Table\n");
        AcpiReadMadt(loc);
        break;

    case MAKE_SIG("BERT"):
        kprintf(".. BERT: Boot Error Record Table\n");
        break;

    case MAKE_SIG("BGRT"):
        kprintf(".. BGRT: Boot Graphics Resource Table\n");
        break;

    case MAKE_SIG("BOOT"):
        kprintf(".. BOOT: Simple Boot Flag Table\n");
        break;

    case MAKE_SIG("CPEP"):
        kprintf(".. CPEP: Corrected Platform Error Polling Table\n");
        break;

    case MAKE_SIG("CSRT"):
        kprintf(".. CSRT: Core System Resource Table\n");
        break;

    case MAKE_SIG("DBG2"):
        kprintf(".. DBG2: Debug Port Table 2\n");
        break;

    case MAKE_SIG("DBGP"):
        kprintf(".. DBGP: Debug Port Table\n");
        break;

    case MAKE_SIG("DMAR"):
        kprintf(".. DMAR: DMA Remapping Table\n");
        break;

    case MAKE_SIG("DRTM"):
        kprintf(".. DRTM: Dynamic Root of Trust for Measurement Table\n");
        break;

    case MAKE_SIG("DSDT"):
        kprintf(".. DSDT: Differentiated System Description Table\n");
        break;

    case MAKE_SIG("ECDT"):
        kprintf(".. ECDT: Embedded Controller Boot Resources Table\n");
        break;

    case MAKE_SIG("EINJ"):
        kprintf(".. EINJ: Error Injection Table\n");
        break;

    case MAKE_SIG("ERST"):
        kprintf(".. ERST: Error Record Serialization Table\n");
        break;

    case MAKE_SIG("ETDT"):
        kprintf(".. ETDT: Event Timer Description Table (Obsolete)\n");
        break;

    case MAKE_SIG("FACP"):
        kprintf(".. FACP: Fixed ACPI Description Table (FADT)\n");
        break;

    case MAKE_SIG("FACS"):
        kprintf(".. FACS: Firmware ACPI Control Structure\n");
        break;

    case MAKE_SIG("FPDT"):
        kprintf(".. FPDT: Firmware Performance Data Table\n");
        break;

    case MAKE_SIG("GTDT"):
        kprintf(".. GTDT: Generic Timer Description Table\n");
        break;

    case MAKE_SIG("HEST"):
        kprintf(".. HEST: Hardware Error Source Table\n");
        break;

    case MAKE_SIG("HPET"):
        kprintf(".. HPET: High Performance Event Timer\n");
        break;

    case MAKE_SIG("IBFT"):
        kprintf(".. IBFT: iSCSI Boot Firmware Table\n");
        break;

    case MAKE_SIG("IVRS"):
        kprintf(".. IVRS: I/O Virtualization Reporting Structure\n");
        break;

    case MAKE_SIG("MCFG"):
        kprintf(".. MCFG: PCI Express memory mapped configuration space base address Description Table\n");
        break;

    case MAKE_SIG("MCHI"):
        kprintf(".. MCHI: Management Controller Host Interface Table\n");
        break;

    case MAKE_SIG("MPST"):
        kprintf(".. MPST: Memory Power State Table\n");
        break;

    case MAKE_SIG("MSCT"):
        kprintf(".. MSCT: Maximum System Characteristics Table");
        break;

    case MAKE_SIG("MSDM"):
        kprintf(".. MSDM: Microsoft Data Management Table\n");
        break;

    case MAKE_SIG("OEM0"):
    case MAKE_SIG("OEM1"):
    case MAKE_SIG("OEM2"):
    case MAKE_SIG("OEM3"):
    case MAKE_SIG("OEM4"):
    case MAKE_SIG("OEM5"):
    case MAKE_SIG("OEM6"):
    case MAKE_SIG("OEM7"):
    case MAKE_SIG("OEM8"):
    case MAKE_SIG("OEM9"):
    case MAKE_SIG("OEMA"):
    case MAKE_SIG("OEMB"):
    case MAKE_SIG("OEMC"):
    case MAKE_SIG("OEMD"):
    case MAKE_SIG("OEME"):
    case MAKE_SIG("OEMF"):
    case MAKE_SIG("OEMG"):
    case MAKE_SIG("OEMH"):
    case MAKE_SIG("OEMI"):
    case MAKE_SIG("OEMJ"):
    case MAKE_SIG("OEMK"):
    case MAKE_SIG("OEML"):
    case MAKE_SIG("OEMM"):
    case MAKE_SIG("OEMN"):
    case MAKE_SIG("OEMO"):
    case MAKE_SIG("OEMP"):
    case MAKE_SIG("OEMQ"):
    case MAKE_SIG("OEMR"):
    case MAKE_SIG("OEMS"):
    case MAKE_SIG("OEMT"):
    case MAKE_SIG("OEMU"):
    case MAKE_SIG("OEMV"):
    case MAKE_SIG("OEMW"):
    case MAKE_SIG("OEMX"):
    case MAKE_SIG("OEMY"):
    case MAKE_SIG("OEMZ"):
        kprintf(".. OEMx: OEM Specific Information Table\n");
        break;

    case MAKE_SIG("PMTT"):
        kprintf(".. PMTT: Platform Memory Topology Table\n");
        break;

    case MAKE_SIG("PSDT"):
        kprintf(".. PSDT: Persistent System Description Table\n");
        break;

    case MAKE_SIG("RASF"):
        kprintf(".. RASF: ACPI RAS Feature Table\n");
        break;

    case MAKE_SIG("RSDT"):
        kprintf(".. RSDT: Root System Description Table\n");
        break;

    case MAKE_SIG("SBST"):
        kprintf(".. SBST: Smart Battery Specification Table\n");
        break;

    case MAKE_SIG("SLIC"):
        kprintf(".. SLIC: Microsoft Software Licensing Table Specification\n");
        break;

    case MAKE_SIG("SLIT"):
        kprintf(".. SLIT: System Locality Distance Information Table\n");
        break;

    case MAKE_SIG("SPCR"):
        kprintf(".. SPCR: Serial Port Console Redirection Table\n");
        break;

    case MAKE_SIG("SPMI"):
        kprintf(".. SPMI: Server Platform Management Interface Table\n");
        break;

    case MAKE_SIG("SRAT"):
        kprintf(".. SRAT: System Resource Affinity Table\n");
        break;

    case MAKE_SIG("SSDT"):
        kprintf(".. SSDT: Secondary System Description Table\n");
        break;

    case MAKE_SIG("TCPA"):
        kprintf(".. TCPA: Trusted Computing Platform Alliance Capabilities Table\n");
        break;

    case MAKE_SIG("TPM2"):
        kprintf(".. TPM2: Trusted Platform Module 2 Table\n");
        break;

    case MAKE_SIG("UEFI"):
        kprintf(".. UEFI: UEFI ACPI Data Table\n");
        break;

    case MAKE_SIG("WAET"):
        kprintf(".. WAET: Windows ACPI Emulated Deviced Table\n");
        break;

    case MAKE_SIG("WDAT"):
        kprintf(".. WDAT: Watch Dog Action Table\n");
        break;

    case MAKE_SIG("XSDT"):
        kprintf(".. XSDT: Extended System Description Table\n");
        break;

    default:
        {
            char l1 [2] = {0};
            char l2 [2] = {0};
            char l3 [2] = {0};
            char l4 [2] = {0};

            l4[0] = (rv >> 24) & 0xff;
            l3[0] = (rv >> 16) & 0xff;
            l2[0] = (rv >> 8) & 0xff;
            l1[0] = rv & 0xff;

            kprintf(".. The table for this signature is invalid; ignoring table %s%s%s%s\n", l1, l2, l3, l4);
            return 0;
        }
    }

    return rv;
}
