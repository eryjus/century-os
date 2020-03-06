//===================================================================================================================
//
//  platform-acpi.h -- These are the structures and functions for interacting with ACPI for the kernel
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Apr-05  Initial   0.4.1   ADCL  Initial version
//
//===================================================================================================================


#pragma once


#ifndef __HARDWARE_H__
#   error "Use #include \"hardware.h\" and it will pick up this file; do not #include this file directly."
#endif


#include "types.h"
#include "printf.h"
#include "mmu.h"
#include "cpu.h"


//
// -- Check if the ACPI is locatable for our kernel
//    ---------------------------------------------
EXPORT LOADER INLINE
void CheckAcpi(archsize_t loc) {
    if (!((loc) >= ACPI_LO) && ((loc) <= ACPI_HI)) {
        CpuPanicPushRegs("PANIC: ACPI is not is a supported");
    } else {
        MmuMapToFrame(loc & 0xfffff000, loc >> 12, PG_KRN);
    }
}


//
// -- Macro to convert a char[4] into a uint_32
//    -----------------------------------------
#define MAKE_SIG(s)         ((uint32_t)(s[3]<<24)|(uint32_t)(s[2]<<16)|(uint32_t)(s[1]<<8)|s[0])


//
// -- this is the signature of the RSDP, expressed as a uint64_t
//    ----------------------------------------------------------
#define RSDP_SIG            ((uint64_t)' '<<56|(uint64_t)'R'<<48|(uint64_t)'T'<<40|(uint64_t)'P'<<32\
                    |(uint64_t)' '<<24|(uint64_t)'D'<<16|(uint64_t)'S'<<8|(uint64_t)'R')


//
// -- this is the structure known as the RSDP (Root System Description Pointer)
//    -------------------------------------------------------------------------
typedef struct RSDP_t {
    union {
        char signature[8];
        uint64_t lSignature;
    };
    uint8_t checksum;
    char oemid[6];
    uint8_t revision;
    uint32_t rsdtAddress;
    uint32_t length;
    uint64_t xsdtAddress;
    uint8_t extendedChecksum;
    uint8_t reserved[3];
} __attribute__((packed, aligned(16))) RSDP_t;


//
// -- This function will locate the RSDP if available and note it in the hardware discovery table
//    -------------------------------------------------------------------------------------------
EXTERN_C EXPORT LOADER
RSDP_t *AcpiFindRsdp(void);


//
// -- This is the common structure members that occur in nearly all of the acpi tables (except rsdp)
//    ----------------------------------------------------------------------------------------------
#define ACPI_DESCRIPTION_HEADER                                                 \
    union {                                                                     \
        char signature[4];                                                      \
        uint32_t lSignature;                                                    \
    };                                                                          \
    uint32_t length;                                                            \
    uint8_t revision;                                                           \
    uint8_t checksum;                                                           \
    char oemid[6];                                                              \
    uint64_t oemTableId;                                                        \
    uint32_t oemRevision;                                                       \
    uint32_t creatorId;                                                         \
    uint32_t creatorRevision


//
// -- Check the table to see if it is what we expect; note that this memory must be mapped before calling
//    ---------------------------------------------------------------------------------------------------
EXTERN_C EXPORT LOADER
bool AcpiCheckTable(archsize_t locn, uint32_t);


//
// -- get the table signature (and check its valid); return 0 if invalid
//    ------------------------------------------------------------------
EXTERN_C EXPORT LOADER
uint32_t AcpiGetTableSig(archsize_t loc);


//
// -- This is the Root System Description Table (RSDT)
//    ------------------------------------------------
typedef struct RSDT_t {
    ACPI_DESCRIPTION_HEADER;
    uint32_t entry[0];                  // there may be 0 or several of these; length must be checked
} __attribute__((packed)) RSDT_t;


//
// -- read the rdst table
//    -------------------
EXTERN_C EXPORT LOADER
bool AcpiReadRsdt(archsize_t loc);


//
// -- This is the Root System Description Table (RSDT)
//    ------------------------------------------------
typedef struct XSDT_t {
    ACPI_DESCRIPTION_HEADER;
    uint64_t entry[0];                  // there may be 0 or several of these; length must be checked
} __attribute__((packed)) XSDT_t;


//
// -- read the xsdt table
//    -------------------
EXTERN_C EXPORT LOADER
bool AcpiReadXsdt(archsize_t loc);


//
// -- The Multiple APIC Description Table (MADT)
//    ------------------------------------------
typedef struct MADT_t {
    ACPI_DESCRIPTION_HEADER;
    uint32_t localIntCtrlAddr;
    uint32_t flags;
    uint8_t intCtrlStructs[0];
} __attribute__((packed)) MADT_t;


//
// -- Read an ACPI MADT table
//    -----------------------
EXTERN_C EXPORT LOADER
void AcpiReadMadt(archsize_t loc);


//
// -- These are the types of Interrupt Controller Structure Types we can have
//    -----------------------------------------------------------------------
typedef enum {
    MADT_PROCESSOR_LOCAL_APIC = 0,
    MADT_IO_APIC = 1,
    MADT_INTERRUPT_SOURCE_OVERRIDE = 2,
    MADT_NMI_SOURCE = 3,
    MADT_LOCAL_APIC_NMI = 4,
    MADT_LOCAL_APIC_ADDRESS_OVERRIDE = 5,
    MADT_IO_SAPIC = 6,
    MADT_LOCAL_SAPIC = 7,
    MADT_PLATFORM_INTERRUPT_SOURCES = 8,
    MADT_PROCESSOR_LOCAL_X2APIC = 9,
    MADT_LOCAL_X2APIC_NMI = 0xa,
    MADT_GIC = 0xb,
    MADT_GICD = 0xc,
} MadtIcType;


//
// -- Local Processor APIC structure
//    ------------------------------
typedef struct MadtLocalApic_t {
    uint8_t type;
    uint8_t len;
    uint8_t procId;
    uint8_t apicId;
    uint32_t flags;         // 0b00000001 means the processor is enabled
} __attribute__((packed)) MadtLocalApic_t;


//
// -- I/O APIC Structure
//    ------------------
typedef struct MadtIoApic_t {
    uint8_t type;
    uint8_t len;
    uint8_t apicId;
    uint8_t reserved;
    uint32_t ioApicAddr;
    uint32_t gsiBase;
} __attribute__((packed)) MadtIoApic_t;


//
// -- Interrupt Source Override Structure
//    -----------------------------------
typedef struct MadtIntSrcOverride_t {
    uint8_t type;
    uint8_t len;
    uint8_t bus;        // -- fixed: 0 = ISA
    uint8_t source;
    uint32_t gsInt;
    uint32_t flags;
} __attribute__((packed)) MadtIntSrcOverride_t;


//
// -- NMI Interrupt Source Structure
//    ------------------------------
typedef struct MadtMNISource_t {
    uint8_t type;
    uint8_t len;
    uint16_t flags;
    uint32_t gsInt;
} __attribute__((packed)) MadtMNISource_t;


//
// -- Local APIC NMI Structure
//    ------------------------
typedef struct MadtLocalApicNMI_t {
    uint8_t type;
    uint8_t len;
    uint8_t procId;         // -- 0xff is all procs
    uint16_t flags;
    uint8_t localLINT;
} __attribute__((packed)) MadtLocalApicNMI_t;


