//===================================================================================================================
//
//  pic.h -- These are the hardware abstractions for interacting with the Programmable Interrupt Controller
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


#pragma once


#ifndef __PIC_H__
#   define __PIC_H__
#endif


#include "types.h"
#include "hardware.h"


//
// -- A forward declaration
//    ---------------------
struct PicDevice_t;


//
// -- get the platform-specific definitions
//    -------------------------------------
#if __has_include("platform-pic.h")
#   include "platform-pic.h"
#endif

#if __has_include("platform-apic.h")
#   include "platform-apic.h"
#endif


//
// -- This is a control structure for the pic, all functions will be registered in this structure
//    -------------------------------------------------------------------------------------------
typedef struct PicDevice_t {
    GenericDevice_t device;

    volatile bool ipiReady;
    void (*PicInit)(PicDevice_t *, const char *);
    isrFunc_t (*PicRegisterHandler)(PicDevice_t *, Irq_t, int, isrFunc_t);
    void (*PicMaskIrq)(PicDevice_t *, Irq_t);
    void (*PicUnmaskIrq)(PicDevice_t *, Irq_t);
    void (*PicEoi)(PicDevice_t *, Irq_t);
    int (*PicDetermineIrq)(PicDevice_t *);
    void (*PicBroadcastIpi)(PicDevice_t *, int);
    void (*PicBroadcastInit)(PicDevice_t *, uint32_t);
    void (*PicBroadcastSipi)(PicDevice_t *, uint32_t, archsize_t);
} PicDevice_t;


//
// -- The global PIC control structure holding pointers to all the proper functions.
//    ------------------------------------------------------------------------------
EXTERN KERNEL_DATA
PicDevice_t *picControl;


//
// -- These are the common interface functions we will use to interact with the PIC.  These functions are
//    not safe in that they will not check for nulls before calling the function.  Therefore, caller beware!
//    ------------------------------------------------------------------------------------------------------
EXPORT KERNEL INLINE
void PicInit(PicDevice_t *dev, const char *name) { dev->PicInit(dev, name); }

EXPORT KERNEL INLINE
isrFunc_t PicRegisterHandler(PicDevice_t *dev, Irq_t irq, int vector, isrFunc_t handler) {
                    return dev->PicRegisterHandler(dev, irq, vector, handler); }

EXPORT KERNEL INLINE
void PicUnmaskIrq(PicDevice_t *dev, Irq_t irq) { dev->PicUnmaskIrq(dev, irq); }

EXPORT KERNEL INLINE
void PicMaskIrq(PicDevice_t *dev, Irq_t irq) { dev->PicMaskIrq(dev, irq); }

EXPORT KERNEL INLINE
void PicEoi(PicDevice_t *dev, Irq_t irq) { dev->PicEoi(dev, irq); }

EXPORT KERNEL INLINE
archsize_t PicDetermineIrq(PicDevice_t *dev) { return dev->PicDetermineIrq(dev); }

EXPORT KERNEL INLINE
void PicBroadcastIpi(PicDevice_t *dev, int ipi) { return dev->PicBroadcastIpi(dev, ipi); }


//
// -- Pick the correct PIC given what we have available
//    -------------------------------------------------
EXTERN_C EXPORT LOADER
PicDevice_t *PicPick(void);

