;;===================================================================================================================
;;
;; libk/src/i686/DisableInterrupts.s -- Disables Interrupts and returns the previous value
;;
;; Disable Interrupts, returning the value of flags in eax.  Note that this function is architecture dependent as
;; eflags is a 32-bit field and that field is returned to the caller.
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
global DisableInterrupts


;;
;; -- This is the beginning of the code segment for this file
;;    -------------------------------------------------------
section .text
cpu		586


;;
;; -- DisableInterrupts() -- Disable interrupts for the x86 family and return the eflags register (32-bit)
;;    ----------------------------------------------------------------------------------------------------
DisableInterrupts:
	pushfd					                                ; push the flags into the stack
	pop		eax				                                ; restore the flags for return to caller
	cli						                                ; clear the interrupts flag
	ret