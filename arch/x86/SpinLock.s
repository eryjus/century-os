;;===================================================================================================================
;;
;;  SpinLock.s -- Atomically CMPXCHG a value for a spinlock
;;
;;        Copyright (c)  2017-2019 -- Adam Clark
;;        Licensed under "THE BEER-WARE LICENSE"
;;        See License.md for details.
;;
;;  Atomically perform a lock for a spinlock, returning the old value
;;
;;  The stack will look like this on entry:
;;  +-----------+------------------------------------+
;;  | esp + 04  |  Address of the structure          |
;;  +-----------+------------------------------------+
;;  |   esp     |  Return EIP                        |
;;  +-----------+------------------------------------+
;;  | esp - 04  |  EBX                               |
;;  +-----------+------------------------------------+
;;  | esp - 08  |  ECX                               |
;;  +-----------+------------------------------------+
;;
;; -----------------------------------------------------------------------------------------------------------------
;;
;;     Date      Tracker  Version  Pgmr  Description
;;  -----------  -------  -------  ----  ---------------------------------------------------------------------------
;;  2013-Aug-24    69                    Initial version
;;  2018-Oct-14  Initial   0.1.0   ADCL  Copied this file from Century32 (AtomicCmpXchg.s)
;;  2019-Feb-09  Initial   0.3.0   ADCL  Relocated
;;  2019-Jun-16  Initial   0.4.6   ADCL  Reseurrected this source file and renamed it
;;
;;===================================================================================================================


;;
;; -- Now, we need some things from other functions imported
;;    ------------------------------------------------------
    global  SpinLock


;;
;; -- This is the beginning of the code segment for this file
;;    -------------------------------------------------------
    section .text
    cpu        586


;;
;; -- Perform the compare and exchange
;;    --------------------------------
SpinLock:
    push        ebp                     ;; save the stack frame
    mov         ebp,esp                 ;; create a new stack frame
    push        ebx                     ;; save the ebx register
    push        ecx                     ;; save the ecx register

    mov         ebx,[ebp+8]             ;; get the address of the spinlock struct (note: offset is 0)
    mov         ecx,1                   ;; this is the value to load

loop:
    mov         eax,0                   ;; this is the value it should be to load
LOCK cmpxchg    [ebx],ecx               ;; do the cmpxchg -- notice the LOCK prefix
    jnz         loop                    ;; if the lock was not unlocked, loop

    pop         ecx                     ;; restore the ecx register
    pop         ebx                     ;; restore the ebx register
    pop         ebp                     ;; restore stack frame
    ret
