//===================================================================================================================
//
//  platform-pic.h -- Programmable Interrupt Controller definitions and functions for the x86
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Feb-24  Initial   0.3.0   ADCL  Initial version
//
//===================================================================================================================


#ifndef __PIC_H__
#   error "Use #include \"pic.h\" and it will pick up this file; do not #include this file directly."
#endif


//
// -- on x86, this is the type we use to refer to the pic port
//    --------------------------------------------------------
typedef uint16_t PicBase_t;


//
// -- This enum contains the IRQ numbers for the system (caution -- may vary by platform & PIC)
//    These are initially derived from the IOAPIC redirection tables entries.
//    -----------------------------------------------------------------------------------------
typedef enum {
    IRQ0  =  0,
    IRQ1  =  1,
    IRQ2  =  2,
    IRQ3  =  3,
    IRQ4  =  4,
    IRQ5  =  5,
    IRQ6  =  6,
    IRQ7  =  7,
    IRQ8  =  8,
    IRQ9  =  9,
    IRQ10 = 10,
    IRQ11 = 11,
    IRQ12 = 12,
    IRQ13 = 13,
    IRQ14 = 14,
    IRQ15 = 15,     // this is the end of IRQs for the 8259(A) PIC
    PIRQ0 = 16,
    PIRQ1 = 17,
    PIRQ2 = 18,
    PIRQ3 = 19,
    MIRQ0 = 20,
    MIRQ1 = 21,
    GPIRQ = 22,
    SMI   = 23,
    INTR  = 24,

    IRQ_LAST,       // THIS MUST BE THE LAST ENTRY!!
} Irq_t;


//
// -- These are the possible pic drivers for the computer
//    ---------------------------------------------------
extern struct PicDevice_t pic8259;
extern struct PicDevice_t ioapicDriver;

//
// -- This is the base location of the timer on x86
//    ---------------------------------------------
#define PIC1         0x20
#define PIC2         0xa0


//
// -- Here are the offsets for the different ports of interest
//    --------------------------------------------------------
#define PIC_MASTER_COMMAND      0x00
#define PIC_MASTER_DATA         0x01
#define PIC_SLAVE_COMMAND       0x00
#define PIC_SLAVE_DATA          0x01


//
// -- Here are the function prototypes that the operation functions need to conform to
//    --------------------------------------------------------------------------------
__CENTURY_FUNC__ void _PicInit(PicDevice_t *dev, const char *name);
__CENTURY_FUNC__ isrFunc_t _PicRegisterHandler(PicDevice_t *dev, Irq_t irq, int vector, isrFunc_t handler);
__CENTURY_FUNC__ void _PicUnmaskIrq(PicDevice_t *dev, Irq_t irq);
__CENTURY_FUNC__ void _PicMaskIrq(PicDevice_t *dev, Irq_t irq);
__CENTURY_FUNC__ void _PicEoi(PicDevice_t *dev, Irq_t irq);



