;;===================================================================================================================
;;
;;  EnableInterrupts.s -- Enable Interrupts
;;
;;        Copyright (c)  2017-2019 -- Adam Clark
;;        Licensed under "THE BEER-WARE LICENSE"
;;        See License.md for details.
;;
;;  Enable Interrupts
;;
;; ------------------------------------------------------------------------------------------------------------------
;;
;;    Date      Tracker  Version  Pgmr  Description
;; -----------  -------  -------  ----  ----------------------------------------------------------------------------
;; 2012-May-28  Initial           	   Initial version
;; 2012-Sep-16  Initial                 Leveraged from Century
;; 2018-May-25  Initial   0.1.0   ADCL  Copied this file from century23 to century-os
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
