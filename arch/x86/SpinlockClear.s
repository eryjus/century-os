;;===================================================================================================================
;;
;;  SpinlockClear.s -- Atomically set the lock to 0, unlocking it
;;
;;        Copyright (c)  2017-2019 -- Adam Clark
;;        Licensed under "THE BEER-WARE LICENSE"
;;        See License.md for details.
;;
;;  Atomically perform an unlock for a spinlock
;;
;;  The stack will look like this on entry:
;;  +-----------+------------------------------------+
;;  | esp + 04  |  Address of the structure          |
;;  +-----------+------------------------------------+
;;  |   esp     |  Return EIP                        |
;;  +-----------+------------------------------------+
;;  | esp - 04  |  EBX                               |
;;  +-----------+------------------------------------+
;;
;; -----------------------------------------------------------------------------------------------------------------
;;
;;     Date      Tracker  Version  Pgmr  Description
;;  -----------  -------  -------  ----  ---------------------------------------------------------------------------
;;  2018-Oct-14  Initial   0.1.0   ADCL  Initial version
;;  2019-Feb-09  Initial   0.3.0   ADCL  Relocated
;;
;;===================================================================================================================


;;
;; -- Now, expose our function to everyone
;;    ------------------------------------
global  SpinlockClear


;;
;; -- This is the beginning of the code segment for this file
;;    -------------------------------------------------------
section .text
cpu        586


;;
;; -- Perform the compare and exchange
;;    --------------------------------
SpinlockClear:
        push        ebp                        ;; save the stack frame
        mov            ebp,esp                    ;; create a new frame
        push        ebx                        ;; save the ebx register

        mov            ebx,[ebp+8]                ;; get the address of the spinlock struct (note: offset is 0)
           mov            dword [ebx],0            ;; set the value to 0 -- notice the LOCK prefix

        pop            ebx                        ;; restore the ebx register
        pop            ebp                        ;; restore stack frame
        ret
