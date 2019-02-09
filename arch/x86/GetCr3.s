;;===================================================================================================================
;;
;;  GetCr3.s -- Get the cr3 register value
;;
;;        Copyright (c)  2017-2019 -- Adam Clark
;;        Licensed under "THE BEER-WARE LICENSE"
;;        See License.md for details.
;;
;; ------------------------------------------------------------------------------------------------------------------
;;
;;     Date      Tracker  Version  Pgmr  Description
;;  -----------  -------  -------  ----  ---------------------------------------------------------------------------
;;  2018-Nov-11  Initial   0.1.0   ADCL  Initial Version
;;  2019-Feb-09  Initial   0.3.0   ADCL  Relocated
;;
;;===================================================================================================================



;;
;; -- Expose the function to the linker
;;    ---------------------------------
global GetCr3


;;
;; -- This is the beginning of the code segment for this file
;;    -------------------------------------------------------
section .text
cpu		586


;;
;; -- Trivial function to get the value in cr3
;;    ----------------------------------------
GetCr3:
    mov     eax,cr3
    ret
