;;===================================================================================================================
;;
;; SpinlockClear.s -- Atomically set the lock to 0, unlocking it
;;
;; Atomically perform an unlock for a spinlock
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
cpu		586


;;
;; -- Perform the compare and exchange
;;    --------------------------------
SpinlockClear:
		push	    ebx				        ;; save the ebx register

		mov		    ebx,[esp+12]	        ;; get the address of the spinlock struct (note: offset is 0)
    	mov		    dword [ebx],0	        ;; set the value to 0 -- notice the LOCK prefix

		pop		    ebx				        ;; restore the ebx register
		ret
