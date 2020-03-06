//===================================================================================================================
//
//  platform-mailbox.h -- Mailbox definitions and functions for the bcm2835
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  FrameBufferInit() will be called from the loader code, so this device will need to be available from the loader
//  and the kernel.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Feb-24  Initial   0.3.0   ADCL  Initial version
//
//===================================================================================================================


#pragma once


#ifndef __HARDWARE_H__
#   error "Use #include \"hardware.h\" and it will pick up this file; do not #include this file directly."
#endif


//
// -- Define a common interface for the GPIO functions that are needed
//    ----------------------------------------------------------------
typedef struct MailboxDevice_t {
    archsize_t base;

    void (*MailboxSend)(struct MailboxDevice_t *, archsize_t, archsize_t);
    archsize_t (*MailboxReceive)(struct MailboxDevice_t *, archsize_t);
} MailboxDevice_t;


//
// -- Here, declare the different configurations of the GPIO will use
//    ---------------------------------------------------------------
EXTERN KERNEL_DATA
MailboxDevice_t kernelMailbox;


//
// -- These are the common interface functions we will use to interact with the GPIO.  These functions are
//    not safe in that they will not check for nulls before calling the function.  Therefore, caller beware!
//    -----------------------------------------------------------------------------------------------------------
EXPORT KERNEL INLINE
void MailboxSend(MailboxDevice_t *dev, archsize_t mb, archsize_t msg) { dev->MailboxSend(dev, mb, msg); }

EXPORT KERNEL INLINE
archsize_t MailboxReceive(MailboxDevice_t *dev, archsize_t mb) { return dev->MailboxReceive(dev, mb); }


//
// -- Here are the function prototypes needed for these operations
//    ------------------------------------------------------------
EXTERN_C EXPORT KERNEL
void _MailboxSend(MailboxDevice_t *dev, archsize_t mb, archsize_t);

EXTERN_C EXPORT KERNEL
archsize_t _MailboxReceive(MailboxDevice_t *dev, archsize_t mb);


//
// -- Some mailbox address offsets
//    ----------------------------
#define MB_READ             (0x00)                      // MB: Receiving mail
#define MB_POLL             (0x10)                      // MB: Read witout receiving
#define MB_SENDER           (0x14)                      // MB: Sender information
#define MB_STATUS           (0x18)                      // MB: Information
#define MB_CONFIG           (0x1c)                      // MB: Settings
#define MB_WRITE            (0x20)                      // MB: Send mail


