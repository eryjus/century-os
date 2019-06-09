//===================================================================================================================
//
//  platform-timer.h -- Timer definitions and functions for the bcm2835
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


#ifndef __TIMER_H__
#   error "Use #include \"timer.h\" and it will pick up this file; do not #include this file directly."
#endif


#include "cpu.h"


//
// -- on x86, this is the type we use to refer to the timer port
//    ----------------------------------------------------------
typedef archsize_t TimerBase_t;


//
// -- Read the low level timer value
//    ------------------------------
struct TimerDevice_t;
extern "C" uint64_t SysTimerCount(struct TimerDevice_t *);


//
// -- This is the base location where we will find the pit Timer
//    ----------------------------------------------------------
#define TIMER           (MMIO_VADDR + 0x01003000)


//
// -- These are the offsets we will use for the timer
//    -----------------------------------------------
#define TIMER_CONTROL               0x00
#define TIMER_PRESCALAR             0x08
#define TIMER_LOCAL_INT_ROUTING     0x24
#define TIMER_LOCAL_CONTROL         0x34
#define TIMER_WRITE_FLAGS           0x38
#define TIMER_INTERRUPT_CONTROL     0x40
#define MAILBOX_INTERRUPT_CONTROL   0x50
#define TIMER_IRQ_SOURCE            0x60
#define TIMER_FIQ_SOURCE            0x70


//
// -- These are the control registers for the timer
//    ---------------------------------------------
#define CNTFRQ      "p15, 0, %0, c14, c0, 0"
#define READ_CNTFRQ()           MRC(CNTFRQ)


#define CNTP_CTL    "p15, 0, %0, c14, c2, 1"
#define READ_CNTP_CTL()         MRC(CNTP_CTL)
#define WRITE_CNTP_CTL(val)     MCR(CNTP_CTL,val)


#define CNTP_TVAL   "p15, 0, %0, c14, c2, 0"
#define READ_CNTP_TVAL()        MRC(CNTP_TVAL)
#define WRITE_CNTP_TVAL(val)    MCR(CNTP_TVAL,val)


#define CNTPCT      "p15, 0, %0, %1, c14"
#define READ_CNTPCT()           MRRC(CNTPCT)


#define CNTP_CVAL   "p15, 2, %0, %1, c14"
#define READ_CNTP_CVAL()        MRRC(CNTP_CVAL)
#define WRITE_CNTP_CVAL(val)    MCRR(CNTP_CVAL,val)




