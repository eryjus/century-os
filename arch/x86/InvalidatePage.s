;;===================================================================================================================
;;
;;  InvalidatePage.s -- Invalidate a page in the TLB buffer
;;
;;        Copyright (c)  2017-2019 -- Adam Clark
;;        Licensed under "THE BEER-WARE LICENSE"
;;        See License.md for details.
;;
;; ------------------------------------------------------------------------------------------------------------------
;;
;;     Date      Tracker  Version  Pgmr  Description
;;  -----------  -------  -------  ----  ---------------------------------------------------------------------------
;;  2019-Feb-22  Initial   0.3.0   ADCL  Initial version
;;
;;===================================================================================================================



;;
;; -- Expose the function to the linker
;;    ---------------------------------
global InvalidatePage


;;
;; -- This is the beginning of the code segment for this file
;;    -------------------------------------------------------
section .text
cpu		686


;;
;; -- Trivial function to get the value in cr3
;;    ----------------------------------------
InvalidatePage:
    mov     eax,[esp+4]
    invlpg  [eax]
    ret
