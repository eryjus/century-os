//===================================================================================================================
//
//  platform-timer.h -- Timer definitions and functions for the x86
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


//
// -- on x86, this is the type we use to refer to the timer port
//    ----------------------------------------------------------
typedef archsize_t TimerBase_t;


//
// -- This is the base location where we will find the pit Timer
//    ----------------------------------------------------------
#define TIMER       0x40


//
// -- These are the offsets we will use for the timer
//    -----------------------------------------------
#define TIMER_CHAN_0    0x00
#define TIMER_CHAN_1    0x01
#define TIMER_CHAN_2    0x02
#define TIMER_COMMAND   0x03


//
// -- This is the timer device structures
//    -----------------------------------
extern TimerDevice_t lapicTimerControl;
extern TimerDevice_t timer8259Control;
