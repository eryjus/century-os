;;===================================================================================================================
;;
;; libk/src/i686/RestoreInterrupts.s -- Restores Interrupts to the state passed in
;;
;; Restores Interrupts to the state passed via parameter.  Note that this function is architecture dependent as
;; eflags is a 32-bit field and that field is passed by as a parameter.
;;
;; -----------------------------------------------------------------------------------------------------------------
;;
;;    Date     Tracker  Version  Pgmr  Description
;; ----------  -------  -------  ----  ---------------------------------------------------------------------------
;; 2012-05-28  Initial                 Initial version
;; 2012-09-16  Initial                 Leveraged from Century
;; 2018-05-25  Initial   0.1.0   ADCL  Copied this file from century32 to century-os
;;
;;===================================================================================================================


;;
;; -- Expose labels to fucntions that the linker can pick up
;;    ------------------------------------------------------
global RestoreInterrupts


;;
;; -- This is the beginning of the code segment for this file
;;    -------------------------------------------------------
section .text
cpu		586


;;
;; -- RestoreInterrupts() -- Restore interrupts to the state passed in (32-bit)
;;    -------------------------------------------------------------------------
RestoreInterrupts:
	mov		eax,[esp+4]				                        ; get the parameter
	push	eax						                        ; push it on the stack
	popfd							                        ; and pop off the flags register
	ret