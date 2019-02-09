//===================================================================================================================
//
//  timer.h -- The structures and function prototypes for the PIT timer
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  Note that the use of the PIT is dependent on the use ouf the PIC (Programmable Interrupt Controller)
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Oct-28  Initial   0.1.0   ADCL  Initial version
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#ifndef __TIMER_H__
#define __TIMER_H__


#include "types.h"


//
// -- This is a control structure for the timer, all functions will be registered in this structure
//    ---------------------------------------------------------------------------------------------
typedef struct TimerFunctions_t {
    void (*init)(uint32_t frequency);
    void (*eoi)(uint32_t irq);
} TimerFunctions_t;


//
// -- The global timer control structure holding pointers to all the proper functions
//    -------------------------------------------------------------------------------
extern TimerFunctions_t *timerControl;


//
// -- This is the specific control structure for the PIT
//    --------------------------------------------------
extern TimerFunctions_t pitTimer;


//
// -- Initialize the PIT timer
//    ------------------------
void TimerInit(uint32_t frequency);


//
// -- Provide an EOI for the Timer to the PIC
//    ---------------------------------------
void TimerEoi(uint32_t irq);


#endif
