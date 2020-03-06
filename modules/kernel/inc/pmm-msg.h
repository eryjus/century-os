//===================================================================================================================
//
//  pmm-msg.h -- These are the messages that the PMM will recognize
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-11  Initial    0.1.0  ADCL  Initial version
//
//===================================================================================================================


#pragma once


#ifndef __PMM_H__
#   error "Do not include 'pmm-msg.h' directly.  It can be included through 'pmm.h'"
#endif


#include "types.h"


//
// -- These are the messages that the PMM manager will respond to
//    -----------------------------------------------------------
typedef enum {
    PMM_NOOP,
    PMM_FREE_FRAME,
    PMM_ALLOC_FRAME,
    PMM_FREE_RANGE,
    PMM_ALLOC_RANGE,
    PMM_NEW_FRAME,
    PMM_INIT,
} PmmMessages_t;

