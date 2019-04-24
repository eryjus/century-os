//===================================================================================================================
//
//  platform-pic.h -- Programmable Interrupt Controller definitions and functions for the bcm2835
//
//        Copyright (c)  2017-2019 -- Adam Clark
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
typedef archsize_t PicBase_t;


//
// -- These are the possible pic drivers for the computer
//    ---------------------------------------------------
extern struct PicDevice_t picBcm2835;


//
// -- BCM2835 defines IRQs 0-63, plus a handfull of additional IRQs.  These additional ones are being placed
//    starting at IRQ64 and up.
//    ------------------------------------------------------------------------------------------------------
#define IRQ_ARM_TIMER       64
#define IRQ_ARM_MAILBOX     65
#define IRQ_ARM_DOORBELL0   66
#define IRQ_ARM_DOORBALL1   67
#define IRQ_GPU0_HALTED     68
#define IRQ_GPU1_HALTED     69
#define IRQ_ILLEGAL_ACCESS1 70
#define IRQ_ILLEGAL_ACCESS0 71


//
// -- Define the pic data we need to keep track of
//    --------------------------------------------
typedef struct Bcm2835Pic_t {
    GenericDevice_t base;
    PicBase_t picLoc;
    archsize_t timerLoc;
} Bcm2835Pic_t;


//
// -- Here are the function prototypes that the operation functions need to conform to
//    --------------------------------------------------------------------------------
extern void _PicInit(PicDevice_t *dev, const char *name);
extern void _PicUnmaskIrq(PicDevice_t *dev, int irq);
extern void _PicMaskIrq(PicDevice_t *dev, int irq);
extern void _PicEoi(PicDevice_t *dev, int irq);
extern int _PicDetermineIrq(PicDevice_t *dev);


//
// -- This is the base location of the timer on x86
//    ---------------------------------------------
#define PIC                 (MMIO_VADDR + 0x00b000)



#define INT_IRQPEND0        (0x200)                     // The basic interrupt pending register
//-------------------------------------------------------------------------------------------------------------------
#define INTPND0IRQ62        (1<<20)                     // GPU IRQ 62
#define INTPND0IRQ57        (1<<19)                     // GPU IRQ 57
#define INTPND0IRQ56        (1<<18)                     // GPU IRQ 56
#define INTPND0IRQ55        (1<<17)                     // GPU IRQ 55
#define INTPND0IRQ54        (1<<16)                     // GPU IRQ 54
#define INTPND0IRQ53        (1<<15)                     // GPU IRQ 53
#define INTPND0IRQ19        (1<<14)                     // GPU IRQ 19
#define INTPND0IRQ18        (1<<13)                     // GPU IRQ 18
#define INTPND0IRQ10        (1<<12)                     // GPU IRQ 10
#define INTPND0IRQ9         (1<<11)                     // GPU IRQ 9
#define INTPND0IRQ7         (1<<10)                     // GPU IRQ 7
#define INTPND0IRQREG2      (1<<9)                      // Pending Register 0 IRQ
#define INTPND0IRQREG1      (1<<8)                      // Pending Register 1 IRQ
#define INTPND0IRQILL0      (1<<7)                      // Illegal Access type 0 IRQ
#define INTPND0IRQILL1      (1<<6)                      // Illegal Access type 1 IRQ
#define INTPND0IRQGPUH1     (1<<5)                      // GPU1 halted IRQ
#define INTPND0IRQGPUH0     (1<<4)                      // GPU0 halted IRQ
#define INTPND0IRQDOORB1    (1<<3)                      // ARM Doorbell 1
#define INTPND0IRQDOORB0    (1<<2)                      // ARM Doorbell 0
#define INTPND0IRQMAIL      (1<<1)                      // ARM Mailbox IRQ
#define INTPND0IRQTIMER     (1<<0)                      // ARM Timer IRQ


#define INT_IRQPEND1        (0x204)                     // IRQ pending 1
//-------------------------------------------------------------------------------------------------------------------
#define INTPND1IRQ31        (1<<31)                     // IRQ 31 pending
#define INTPND1IRQ30        (1<<30)                     // IRQ 30 pending
#define INTPND1IRQ29        (1<<29)                     // IRQ 29 pending
#define INTPND1IRQ28        (1<<28)                     // IRQ 28 pending
#define INTPND1IRQ27        (1<<27)                     // IRQ 27 pending
#define INTPND1IRQ26        (1<<26)                     // IRQ 26 pending
#define INTPND1IRQ25        (1<<25)                     // IRQ 25 pending
#define INTPND1IRQ24        (1<<24)                     // IRQ 24 pending
#define INTPND1IRQ23        (1<<23)                     // IRQ 23 pending
#define INTPND1IRQ22        (1<<22)                     // IRQ 22 pending
#define INTPND1IRQ21        (1<<21)                     // IRQ 21 pending
#define INTPND1IRQ20        (1<<20)                     // IRQ 20 pending
#define INTPND1IRQ19        (1<<19)                     // IRQ 19 pending
#define INTPND1IRQ18        (1<<18)                     // IRQ 18 pending
#define INTPND1IRQ17        (1<<17)                     // IRQ 17 pending
#define INTPND1IRQ16        (1<<16)                     // IRQ 16 pending
#define INTPND1IRQ15        (1<<15)                     // IRQ 15 pending
#define INTPND1IRQ14        (1<<14)                     // IRQ 14 pending
#define INTPND1IRQ13        (1<<13)                     // IRQ 13 pending
#define INTPND1IRQ12        (1<<12)                     // IRQ 12 pending
#define INTPND1IRQ11        (1<<11)                     // IRQ 11 pending
#define INTPND1IRQ10        (1<<10)                     // IRQ 10 pending
#define INTPND1IRQ9         (1<<9)                      // IRQ 9 pending
#define INTPND1IRQ8         (1<<8)                      // IRQ 8 pending
#define INTPND1IRQ7         (1<<7)                      // IRQ 7 pending
#define INTPND1IRQ6         (1<<6)                      // IRQ 6 pending
#define INTPND1IRQ5         (1<<5)                      // IRQ 5 pending
#define INTPND1IRQ4         (1<<4)                      // IRQ 4 pending
#define INTPND1IRQ3         (1<<3)                      // IRQ 3 pending
#define INTPND1IRQ2         (1<<2)                      // IRQ 2 pending
#define INTPND1IRQ1         (1<<1)                      // IRQ 1 pending
#define INTPND1IRQ0         (1<<0)                      // IRQ 0 pending


#define INT_IRQPEND2        (0x208)                     // IRQ pending 2
//-------------------------------------------------------------------------------------------------------------------
#define INTPND2IRQ63        (1<<31)                     // IRQ 63 pending
#define INTPND2IRQ62        (1<<30)                     // IRQ 62 pending
#define INTPND2IRQ61        (1<<29)                     // IRQ 61 pending
#define INTPND2IRQ60        (1<<28)                     // IRQ 60 pending
#define INTPND2IRQ59        (1<<27)                     // IRQ 59 pending
#define INTPND2IRQ58        (1<<26)                     // IRQ 58 pending
#define INTPND2IRQ57        (1<<25)                     // IRQ 57 pending
#define INTPND2IRQ56        (1<<24)                     // IRQ 56 pending
#define INTPND2IRQ55        (1<<23)                     // IRQ 55 pending
#define INTPND2IRQ54        (1<<22)                     // IRQ 54 pending
#define INTPND2IRQ53        (1<<21)                     // IRQ 53 pending
#define INTPND2IRQ52        (1<<20)                     // IRQ 52 pending
#define INTPND2IRQ51        (1<<19)                     // IRQ 51 pending
#define INTPND2IRQ50        (1<<18)                     // IRQ 50 pending
#define INTPND2IRQ49        (1<<17)                     // IRQ 49 pending
#define INTPND2IRQ48        (1<<16)                     // IRQ 48 pending
#define INTPND2IRQ47        (1<<15)                     // IRQ 47 pending
#define INTPND2IRQ46        (1<<14)                     // IRQ 46 pending
#define INTPND2IRQ45        (1<<13)                     // IRQ 45 pending
#define INTPND2IRQ44        (1<<12)                     // IRQ 44 pending
#define INTPND2IRQ43        (1<<11)                     // IRQ 43 pending
#define INTPND2IRQ42        (1<<10)                     // IRQ 42 pending
#define INTPND2IRQ41        (1<<9)                      // IRQ 41 pending
#define INTPND2IRQ40        (1<<8)                      // IRQ 40 pending
#define INTPND2IRQ39        (1<<7)                      // IRQ 39 pending
#define INTPND2IRQ38        (1<<6)                      // IRQ 38 pending
#define INTPND2IRQ37        (1<<5)                      // IRQ 37 pending
#define INTPND2IRQ36        (1<<4)                      // IRQ 36 pending
#define INTPND2IRQ35        (1<<3)                      // IRQ 35 pending
#define INTPND2IRQ34        (1<<2)                      // IRQ 34 pending
#define INTPND2IRQ33        (1<<1)                      // IRQ 33 pending
#define INTPND2IRQ32        (1<<0)                      // IRQ 32 pending


#define INT_FIQCTL          (0x20c)                     // FIQ Control
//-------------------------------------------------------------------------------------------------------------------
#define INTFIQ_ENB          (1<<7)                      // FIQ enable
#define INTFIQ_SRC          (0x7f)                      // FIQ Source


#define INT_IRQENB1         (0x210)                     // IRQ Enable 1
//-------------------------------------------------------------------------------------------------------------------
#define INTENB1IRQ31        (1<<31)                     // IRQ 31 Enable
#define INTENB1IRQ30        (1<<30)                     // IRQ 30 Enable
#define INTENB1IRQ29        (1<<29)                     // IRQ 29 Enable
#define INTENB1IRQ28        (1<<28)                     // IRQ 28 Enable
#define INTENB1IRQ27        (1<<27)                     // IRQ 27 Enable
#define INTENB1IRQ26        (1<<26)                     // IRQ 26 Enable
#define INTENB1IRQ25        (1<<25)                     // IRQ 25 Enable
#define INTENB1IRQ24        (1<<24)                     // IRQ 24 Enable
#define INTENB1IRQ23        (1<<23)                     // IRQ 23 Enable
#define INTENB1IRQ22        (1<<22)                     // IRQ 22 Enable
#define INTENB1IRQ21        (1<<21)                     // IRQ 21 Enable
#define INTENB1IRQ20        (1<<20)                     // IRQ 20 Enable
#define INTENB1IRQ19        (1<<19)                     // IRQ 19 Enable
#define INTENB1IRQ18        (1<<18)                     // IRQ 18 Enable
#define INTENB1IRQ17        (1<<17)                     // IRQ 17 Enable
#define INTENB1IRQ16        (1<<16)                     // IRQ 16 Enable
#define INTENB1IRQ15        (1<<15)                     // IRQ 15 Enable
#define INTENB1IRQ14        (1<<14)                     // IRQ 14 Enable
#define INTENB1IRQ13        (1<<13)                     // IRQ 13 Enable
#define INTENB1IRQ12        (1<<12)                     // IRQ 12 Enable
#define INTENB1IRQ11        (1<<11)                     // IRQ 11 Enable
#define INTENB1IRQ10        (1<<10)                     // IRQ 10 Enable
#define INTENB1IRQ9         (1<<9)                      // IRQ 9 Enable
#define INTENB1IRQ8         (1<<8)                      // IRQ 8 Enable
#define INTENB1IRQ7         (1<<7)                      // IRQ 7 Enable
#define INTENB1IRQ6         (1<<6)                      // IRQ 6 Enable
#define INTENB1IRQ5         (1<<5)                      // IRQ 5 Enable
#define INTENB1IRQ4         (1<<4)                      // IRQ 4 Enable
#define INTENB1IRQ3         (1<<3)                      // IRQ 3 Enable
#define INTENB1IRQ2         (1<<2)                      // IRQ 2 Enable
#define INTENB1IRQ1         (1<<1)                      // IRQ 1 Enable
#define INTENB1IRQ0         (1<<0)                      // IRQ 0 Enable


#define INT_IRQENB2         (0x214)                     // IRQ Enable 2
//-------------------------------------------------------------------------------------------------------------------
#define INTENB2IRQ63        (1<<31)                     // IRQ 63 Enable
#define INTENB2IRQ62        (1<<30)                     // IRQ 62 Enable
#define INTENB2IRQ61        (1<<29)                     // IRQ 61 Enable
#define INTENB2IRQ60        (1<<28)                     // IRQ 60 Enable
#define INTENB2IRQ59        (1<<27)                     // IRQ 59 Enable
#define INTENB2IRQ58        (1<<26)                     // IRQ 58 Enable
#define INTENB2IRQ57        (1<<25)                     // IRQ 57 Enable
#define INTENB2IRQ56        (1<<24)                     // IRQ 56 Enable
#define INTENB2IRQ55        (1<<23)                     // IRQ 55 Enable
#define INTENB2IRQ54        (1<<22)                     // IRQ 54 Enable
#define INTENB2IRQ53        (1<<21)                     // IRQ 53 Enable
#define INTENB2IRQ52        (1<<20)                     // IRQ 52 Enable
#define INTENB2IRQ51        (1<<19)                     // IRQ 51 Enable
#define INTENB2IRQ50        (1<<18)                     // IRQ 50 Enable
#define INTENB2IRQ49        (1<<17)                     // IRQ 49 Enable
#define INTENB2IRQ48        (1<<16)                     // IRQ 48 Enable
#define INTENB2IRQ47        (1<<15)                     // IRQ 47 Enable
#define INTENB2IRQ46        (1<<14)                     // IRQ 46 Enable
#define INTENB2IRQ45        (1<<13)                     // IRQ 45 Enable
#define INTENB2IRQ44        (1<<12)                     // IRQ 44 Enable
#define INTENB2IRQ43        (1<<11)                     // IRQ 43 Enable
#define INTENB2IRQ42        (1<<10)                     // IRQ 42 Enable
#define INTENB2IRQ41        (1<<9)                      // IRQ 41 Enable
#define INTENB2IRQ40        (1<<8)                      // IRQ 40 Enable
#define INTENB2IRQ39        (1<<7)                      // IRQ 39 Enable
#define INTENB2IRQ38        (1<<6)                      // IRQ 38 Enable
#define INTENB2IRQ37        (1<<5)                      // IRQ 37 Enable
#define INTENB2IRQ36        (1<<4)                      // IRQ 36 Enable
#define INTENB2IRQ35        (1<<3)                      // IRQ 35 Enable
#define INTENB2IRQ34        (1<<2)                      // IRQ 34 Enable
#define INTENB2IRQ33        (1<<1)                      // IRQ 33 Enable
#define INTENB2IRQ32        (1<<0)                      // IRQ 32 Enable


#define INT_IRQENB0         (0x218)                     // Basic IRQ Enable
//-------------------------------------------------------------------------------------------------------------------
#define INTENB0IRQILL0      (1<<7)                      // Illegal Access type 0 IRQ Enable
#define INTENB0IRQILL1      (1<<6)                      // Illegal Access type 1 IRQ Enable
#define INTENB0IRQGPUH1     (1<<5)                      // GPU1 halted IRQ Enable
#define INTENB0IRQGPUH0     (1<<4)                      // GPU0 halted IRQ Enable
#define INTENB0IRQDOORB1    (1<<3)                      // ARM Doorbell 1 Enable
#define INTENB0IRQDOORB0    (1<<2)                      // ARM Doorbell 0 Enable
#define INTENB0IRQMAIL      (1<<1)                      // ARM Mailbox IRQ Enable
#define INTENB0IRQTIMER     (1<<0)                      // ARM Timer IRQ Enable


#define INT_IRQDIS1         (0x21c)                     // IRQ Disable 1
//-------------------------------------------------------------------------------------------------------------------
#define INTDIS1IRQ31        (1<<31)                     // IRQ 31 Disable
#define INTDIS1IRQ30        (1<<30)                     // IRQ 30 Disable
#define INTDIS1IRQ29        (1<<29)                     // IRQ 29 Disable
#define INTDIS1IRQ28        (1<<28)                     // IRQ 28 Disable
#define INTDIS1IRQ27        (1<<27)                     // IRQ 27 Disable
#define INTDIS1IRQ26        (1<<26)                     // IRQ 26 Disable
#define INTDIS1IRQ25        (1<<25)                     // IRQ 25 Disable
#define INTDIS1IRQ24        (1<<24)                     // IRQ 24 Disable
#define INTDIS1IRQ23        (1<<23)                     // IRQ 23 Disable
#define INTDIS1IRQ22        (1<<22)                     // IRQ 22 Disable
#define INTDIS1IRQ21        (1<<21)                     // IRQ 21 Disable
#define INTDIS1IRQ20        (1<<20)                     // IRQ 20 Disable
#define INTDIS1IRQ19        (1<<19)                     // IRQ 19 Disable
#define INTDIS1IRQ18        (1<<18)                     // IRQ 18 Disable
#define INTDIS1IRQ17        (1<<17)                     // IRQ 17 Disable
#define INTDIS1IRQ16        (1<<16)                     // IRQ 16 Disable
#define INTDIS1IRQ15        (1<<15)                     // IRQ 15 Disable
#define INTDIS1IRQ14        (1<<14)                     // IRQ 14 Disable
#define INTDIS1IRQ13        (1<<13)                     // IRQ 13 Disable
#define INTDIS1IRQ12        (1<<12)                     // IRQ 12 Disable
#define INTDIS1IRQ11        (1<<11)                     // IRQ 11 Disable
#define INTDIS1IRQ10        (1<<10)                     // IRQ 10 Disable
#define INTDIS1IRQ9         (1<<9)                      // IRQ 9 Disable
#define INTDIS1IRQ8         (1<<8)                      // IRQ 8 Disable
#define INTDIS1IRQ7         (1<<7)                      // IRQ 7 Disable
#define INTDIS1IRQ6         (1<<6)                      // IRQ 6 Disable
#define INTDIS1IRQ5         (1<<5)                      // IRQ 5 Disable
#define INTDIS1IRQ4         (1<<4)                      // IRQ 4 Disable
#define INTDIS1IRQ3         (1<<3)                      // IRQ 3 Disable
#define INTDIS1IRQ2         (1<<2)                      // IRQ 2 Disable
#define INTDIS1IRQ1         (1<<1)                      // IRQ 1 Disable
#define INTDIS1IRQ0         (1<<0)                      // IRQ 0 Disable


#define INT_IRQDIS2         (0x220)                     // IRQ Disable 2
//-------------------------------------------------------------------------------------------------------------------
#define INTDIS2IRQ63        (1<<31)                     // IRQ 63 Disable
#define INTDIS2IRQ62        (1<<30)                     // IRQ 62 Disable
#define INTDIS2IRQ61        (1<<29)                     // IRQ 61 Disable
#define INTDIS2IRQ60        (1<<28)                     // IRQ 60 Disable
#define INTDIS2IRQ59        (1<<27)                     // IRQ 59 Disable
#define INTDIS2IRQ58        (1<<26)                     // IRQ 58 Disable
#define INTDIS2IRQ57        (1<<25)                     // IRQ 57 Disable
#define INTDIS2IRQ56        (1<<24)                     // IRQ 56 Disable
#define INTDIS2IRQ55        (1<<23)                     // IRQ 55 Disable
#define INTDIS2IRQ54        (1<<22)                     // IRQ 54 Disable
#define INTDIS2IRQ53        (1<<21)                     // IRQ 53 Disable
#define INTDIS2IRQ52        (1<<20)                     // IRQ 52 Disable
#define INTDIS2IRQ51        (1<<19)                     // IRQ 51 Disable
#define INTDIS2IRQ50        (1<<18)                     // IRQ 50 Disable
#define INTDIS2IRQ49        (1<<17)                     // IRQ 49 Disable
#define INTDIS2IRQ48        (1<<16)                     // IRQ 48 Disable
#define INTDIS2IRQ47        (1<<15)                     // IRQ 47 Disable
#define INTDIS2IRQ46        (1<<14)                     // IRQ 46 Disable
#define INTDIS2IRQ45        (1<<13)                     // IRQ 45 Disable
#define INTDIS2IRQ44        (1<<12)                     // IRQ 44 Disable
#define INTDIS2IRQ43        (1<<11)                     // IRQ 43 Disable
#define INTDIS2IRQ42        (1<<10)                     // IRQ 42 Disable
#define INTDIS2IRQ41        (1<<9)                      // IRQ 41 Disable
#define INTDIS2IRQ40        (1<<8)                      // IRQ 40 Disable
#define INTDIS2IRQ39        (1<<7)                      // IRQ 39 Disable
#define INTDIS2IRQ38        (1<<6)                      // IRQ 38 Disable
#define INTDIS2IRQ37        (1<<5)                      // IRQ 37 Disable
#define INTDIS2IRQ36        (1<<4)                      // IRQ 36 Disable
#define INTDIS2IRQ35        (1<<3)                      // IRQ 35 Disable
#define INTDIS2IRQ34        (1<<2)                      // IRQ 34 Disable
#define INTDIS2IRQ33        (1<<1)                      // IRQ 33 Disable
#define INTDIS2IRQ32        (1<<0)                      // IRQ 32 Disable


#define INT_IRQDIS0         (0x224)                     // Basic IRQ Disable
//-------------------------------------------------------------------------------------------------------------------
#define INTDIS0IRQILL0      (1<<7)                      // Illegal Access type 0 IRQ Disable
#define INTDIS0IRQILL1      (1<<6)                      // Illegal Access type 1 IRQ Disable
#define INTDIS0IRQGPUH1     (1<<5)                      // GPU1 halted IRQ Disable
#define INTDIS0IRQGPUH0     (1<<4)                      // GPU0 halted IRQ Disable
#define INTDIS0IRQDOORB1    (1<<3)                      // ARM Doorbell 1 Disable
#define INTDIS0IRQDOORB0    (1<<2)                      // ARM Doorbell 0 Disable
#define INTDIS0IRQMAIL      (1<<1)                      // ARM Mailbox IRQ Disable
#define INTDIS0IRQTIMER     (1<<0)                      // ARM Timer IRQ Disable


