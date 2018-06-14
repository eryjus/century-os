;;===================================================================================================================
;;
;; libk/src/x86-common/EnableInterrupts.s -- Enable Interrupts
;;
;; Enable Interrupts
;;
;; -----------------------------------------------------------------------------------------------------------------
;;
;;    Date     Tracker  Version  Pgmr  Description
;; ----------  -------  -------  ----  ----------------------------------------------------------------------------
;; 2012-05-28  Initial           	   Initial version
;; 2012-09-16  Initial                 Leveraged from Century
;; 2018-05-25  Initial   0.1.0   ADCL  Copied this file from century23 to century-os
;;
;;===================================================================================================================


;;
;; -- Expose labels to fucntions that the linker can pick up
;;    ------------------------------------------------------
global 	EnableInterrupts


;;
;; -- This is the beginning of the code segment for this file
;;    -------------------------------------------------------
section .text
cpu		586


;;
;; -- EnableInterrupts() -- Enable Interrupts for the x86 CPU family
;;    --------------------------------------------------------------
EnableInterrupts:
	sti														;; enable interrupts -- trivial
	ret
