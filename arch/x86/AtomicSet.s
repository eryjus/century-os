;;===================================================================================================================
;;
;;  AtomicSet.s -- Atomically set the value of an Atomic Integer, returning the previous value
;;
;;        Copyright (c)  2017-2020 -- Adam Clark
;;        Licensed under "THE BEER-WARE LICENSE"
;;        See License.md for details.
;;
;;  Prototype:
;;  int32_t AtomicSet(AtomicInt *a, int32_t value);
;;
;; -----------------------------------------------------------------------------------------------------------------
;;
;;     Date      Tracker  Version  Pgmr  Description
;;  -----------  -------  -------  ----  ---------------------------------------------------------------------------
;;  2019-Apr-28  Initial   0.4.2   ADCL  Initial version
;;
;;===================================================================================================================


;;
;; -- Now, we need some things from other functions imported
;;    ------------------------------------------------------
    global  AtomicSet


;;
;; -- This is the beginning of the code segment for this file
;;    -------------------------------------------------------
    section .text
    cpu     586


;;
;; -- Perform the compare and exchange
;;    --------------------------------
AtomicSet:
        push        ebp                 ;; save the stack frame
        mov         ebp,esp             ;; create a new stack frame
        push        ebx                 ;; save the ebx register

        mov         ebx,[ebp+8]         ;; get the address of the spinlock struct (note: offset is 0)
        mov         eax,[ebp+12]        ;; get the value to set
LOCK    xchg        [ebx],eax           ;; do the xchg -- notice the LOCK prefix

        pop         ebx                 ;; restore the ebx register
        pop         ebp                 ;; restore stack frame
        ret
