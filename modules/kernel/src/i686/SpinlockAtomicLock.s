;;===================================================================================================================
;;
;;  SpinlockAtomicLock.s -- Atomically CMPXCHG a value for a spinlock
;;
;;        Copyright (c)  2017-2018 -- Adam Clark
;;        Licensed under "THE BEER-WARE LICENSE"
;;        See License.md for details.
;;
;;  Atomically perform a lock for a spinlock, returning the old value
;;
;;  The stack will look like this on entry:
;;  +-----------+------------------------------------+
;;  | esp + 12  |  Value to load                     |
;;  +-----------+------------------------------------+
;;  | esp + 08  |  Value expected                    |
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
;;
;;===================================================================================================================


;;
;; -- Now, we need some things from other functions imported
;;    ------------------------------------------------------
global  SpinlockAtomicLock


;;
;; -- This is the beginning of the code segment for this file
;;    -------------------------------------------------------
section .text
cpu		586


;;
;; -- Perform the compare and exchange
;;    --------------------------------
SpinlockAtomicLock:
		push		ebp						;; save the stack frame
		mov			ebp,esp					;; create a new stack frame
		push	    ebx				        ;; save the ebx register
		push	    ecx				        ;; save the ecx register

		mov		    ebx,[ebp+8] 	        ;; get the address of the spinlock struct (note: offset is 0)
		mov		    eax,[ebp+12]	        ;; get the expected value
		mov		    ecx,[ebp+16]	        ;; get the value to xchg
LOCK	cmpxchg	    [ebx],ecx		        ;; do the cmpxchg -- notice the LOCK prefix

		pop		    ecx				        ;; restore the ecx register
		pop		    ebx				        ;; restore the ebx register
		pop			ebp						;; restore stack frame
		ret
