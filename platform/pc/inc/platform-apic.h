//===================================================================================================================
//
//  platform-ioapic.h -- This is any of several IO APICs that may exist on the system board; typically 1 per bus
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


#ifndef __PIC_H__
#   error "Use #include \"pic.h\" and it will pick up this file; do not #include this file directly."
#endif


#include "types.h"
#include "timer.h"


//
// -- This is the data structure where we will store the values we need to manage the PIC
//    -----------------------------------------------------------------------------------
typedef struct IoApicDeviceData_t {
    DeviceData_t work;
    archsize_t ioapicBase;
    archsize_t localApicBase;
    archsize_t redirTableEntry[IRQ_LAST];
} IoApicDeviceData_t;


//
// -- These are the registers that we will read and write
//    ---------------------------------------------------
#define IOREGSEL                (0x00)
#define IOWIN                   (0x10)


//
// -- These are the address offsets that are written to IOREGSEL
//    ----------------------------------------------------------
#define IOAPICID                (0x00)
#define IOAPICVER               (0x01)
#define IOAPICARB               (0x02)
// -- the IOREDTBL registers are in pairs!!
#define IOREDTBL0               (0x10)
#define IOREDTBL1               (0x12)
#define IOREDTBL2               (0x14)
#define IOREDTBL3               (0x16)
#define IOREDTBL4               (0x18)
#define IOREDTBL5               (0x1a)
#define IOREDTBL6               (0x1c)
#define IOREDTBL7               (0x1e)
#define IOREDTBL8               (0x20)
#define IOREDTBL9               (0x22)
#define IOREDTBL10              (0x24)
#define IOREDTBL11              (0x26)
#define IOREDTBL12              (0x28)
#define IOREDTBL13              (0x2a)
#define IOREDTBL14              (0x2c)
#define IOREDTBL15              (0x2e)
#define IOREDTBL16              (0x30)
#define IOREDTBL17              (0x32)
#define IOREDTBL18              (0x34)
#define IOREDTBL19              (0x36)
#define IOREDTBL20              (0x38)
#define IOREDTBL21              (0x3a)
#define IOREDTBL22              (0x3c)
#define IOREDTBL23              (0x3e)


//
// -- These are the offsets to the different Memory Mapped I/O registers for the Local APIC
//    -------------------------------------------------------------------------------------
#define LAPIC_ID            (0x020)
#define LAPIC_VERSION       (0x030)
#define LAPIC_TPR           (0x080)
#define LAPIC_APR           (0x090)
#define LAPIC_PPR           (0x0a0)
#define LAPIC_EOI           (0x0b0)
#define LAPIC_RRD           (0x0c0)
#define LAPIC_LOGICAL_DEST  (0x0d0)
#define LAPIC_DEST_FMT      (0x0e0)
#define LAPIC_SPURIOUS_VECT (0x0f0)
#define LAPIC_ISR_0         (0x100)
#define LAPIC_ISR_1         (0x110)
#define LAPIC_ISR_2         (0x120)
#define LAPIC_ISR_3         (0x130)
#define LAPIC_ISR_4         (0x140)
#define LAPIC_ISR_5         (0x150)
#define LAPIC_ISR_6         (0x160)
#define LAPIC_ISR_7         (0x170)
#define LAPIC_TMR_0         (0x180)
#define LAPIC_TMR_1         (0x190)
#define LAPIC_TMR_2         (0x1a0)
#define LAPIC_TMR_3         (0x1b0)
#define LAPIC_TMR_4         (0x1c0)
#define LAPIC_TMR_5         (0x1d0)
#define LAPIC_TMR_6         (0x1e0)
#define LAPIC_TMR_7         (0x1f0)
#define LAPIC_IRR_0         (0x200)
#define LAPIC_IRR_1         (0x210)
#define LAPIC_IRR_2         (0x220)
#define LAPIC_IRR_3         (0x230)
#define LAPIC_IRR_4         (0x240)
#define LAPIC_IRR_5         (0x250)
#define LAPIC_IRR_6         (0x260)
#define LAPIC_IRR_7         (0x270)
#define LAPIC_ESR           (0x280)
#define LAPIC_ICR_LO        (0x300)
#define LAPIC_ICR_HI        (0x310)
#define LAPIC_LVT_TMR       (0x320)
#define LAPIC_THERMAL       (0x330)
#define LAPIC_LVT_PERF      (0x340)
#define LAPIC_LVT_LINT0     (0x350)
#define LAPIC_LVT_LINT1     (0x360)
#define LAPIC_LVT_ERR       (0x370)
#define LAPIC_TMRINITCNT    (0x380)
#define LAPIC_TMRCURRCNT    (0x390)
#define LAPIC_TMRDIV        (0x3e0)


//
// -- The location in virtual memory for the LAPIC
//    --------------------------------------------
#define LAPIC_MMIO          (0xfee00000)


//
// -- This is the bitting for the IOREGSEL register
//    ---------------------------------------------
typedef struct Ioregsel_t {
    union {
        struct {
            uint32_t address : 8;
            uint32_t reserved : 24;
        } __attribute__((packed));
        uint32_t reg;
    };
} __attribute__((packed)) Ioregsel_t;


//
// -- this is the bitting for the IOAPICID register
//    ---------------------------------------------
typedef struct Ioapicid_t {
    union {
        struct {
            uint32_t reserved : 24;
            uint32_t apicId : 4;
            uint32_t reserved2 : 4;
        } __attribute__((packed));
        uint32_t reg;
    };
} __attribute__((packed)) Ioapicid_t;


//
// -- this is the bitting for the IOAPICVER register
//    ----------------------------------------------
typedef struct Ioapicver_t {
    union {
        struct {
            uint32_t version : 8;
            uint32_t reserved : 8;
            uint32_t maxRedir : 8;
            uint32_t reserved2 : 8;
        } __attribute__((packed));
        uint32_t reg;
    };
} __attribute__((packed)) Ioapicver_t;


//
// -- this is the bitting for the IOAPICARB register
//    ----------------------------------------------
typedef struct Ioapicarb_t {
    union {
        struct {
            uint32_t reserved : 24;
            uint32_t arbId : 4;
            uint32_t reserved2 : 4;
        } __attribute__((packed));
        uint32_t reg;
    };
} __attribute__((packed)) Ioapicarb_t;


//
// -- This is the bitting for the 2 registers that make up an IOREDTBL register
//    -------------------------------------------------------------------------
typedef struct Ioapicredtbl_t {
    union {
        struct {
            uint64_t intvec : 8;
            uint64_t delmod : 3;
            uint64_t destmod : 1;
            uint64_t delivs : 1;
            uint64_t intpol : 1;
            uint64_t remoteIrr : 1;
            uint64_t triggerMode : 1;
            uint64_t intMask : 1;
            uint64_t reserved : 39;
            uint64_t dest : 8;
        } __attribute__((packed));
        struct {
            uint32_t reg0;                  // when reading/writing 32-bit pairs, always do reg0 first!!
            uint32_t reg1;
        } __attribute__((packed));
        uint64_t reg;
    };
} __attribute__((packed)) Ioapicredtbl_t;


//
// -- These are the non-reserved delivery modes in the delmode field
//    --------------------------------------------------------------
enum {
    DELMODE_FIXED   = 0b000,
    DELMODE_LOWEST  = 0b001,
    DELMODE_SMI     = 0b010,
    DELMODE_NMI     = 0b100,
    DELMODE_INIT    = 0b101,
    DELMODE_EXTINT  = 0b111,
};


//
// -- Here are the function prototypes that the operation functions need to conform to
//    --------------------------------------------------------------------------------
__CENTURY_FUNC__ void _IoApicInit(PicDevice_t *dev, const char *name);
__CENTURY_FUNC__ isrFunc_t _IoApicRegisterHandler(PicDevice_t *, Irq_t, int, isrFunc_t);
__CENTURY_FUNC__ void _IoApicUnmaskIrq(PicDevice_t *dev, Irq_t irq);
__CENTURY_FUNC__ void _IoApicMaskIrq(PicDevice_t *dev, Irq_t irq);
__CENTURY_FUNC__ void _IoApicEoi(PicDevice_t *dev, Irq_t irq);
__CENTURY_FUNC__ void _LApicBroadcastIpi(PicDevice_t *dev, int ipi);
__CENTURY_FUNC__ void _LApicBroadcastInit(PicDevice_t *dev);
__CENTURY_FUNC__ void _LApicBroadcastSipi(PicDevice_t *dev);

//
// -- A helper function for translating an IRQ to a redir table entry
//    ---------------------------------------------------------------
inline archsize_t IoApicRedir(IoApicDeviceData_t *data, Irq_t irq) { return data->redirTableEntry[irq]; }


//
// -- Local APIC Timer functions
//    --------------------------
__CENTURY_FUNC__ void _LApicInit(TimerDevice_t *dev, uint32_t frequency);
__CENTURY_FUNC__ void _LApicEoi(TimerDevice_t *dev);
__CENTURY_FUNC__ void _LApicPlatformTick(TimerDevice_t *dev);
__CENTURY_FUNC__ uint64_t _LApicCurrentCount(TimerDevice_t *dev);


//
// -- These 2 macros will assist in reading from/writing to the ioapic registers
//    --------------------------------------------------------------------------
#define IOAPIC_READ(addr,reg)    ({                                 \
            uint32_t _val;                                          \
            MmioWrite(addr + IOREGSEL, reg);                        \
            _val = MmioRead(addr + IOWIN);                          \
            (_val);                                                 \
        })

#define IOAPIC_WRITE(addr,reg,val)                                  \
            do {                                                    \
                MmioWrite(addr + IOREGSEL, reg);                    \
                MmioWrite(addr + IOWIN, val);                       \
            } while (0)

