;;===================================================================================================================
;;
;;  ArchLoadIdt.s -- Load the final IDT and set the proper section selectors
;;
;;        Copyright (c)  2017-2020 -- Adam Clark
;;        Licensed under "THE BEER-WARE LICENSE"
;;        See License.md for details.
;;
;; -----------------------------------------------------------------------------------------------------------------
;;
;;     Date      Tracker  Version  Pgmr  Description
;;  -----------  -------  -------  ----  --------------------------------------------------------------------------
;;  2020-Jan-08  Initial  v0.5.0e  ADCL  Initial version
;;
;;===================================================================================================================


%include "constants.inc"


;;
;; -- Expose some labels to other fucntions that the linker can pick up
;;    -----------------------------------------------------------------
    global      ArchLoadIdt



;;
;; -- Load the IDT and set the the segment selectors
;;    ----------------------------------------------
ArchLoadIdt:
    mov         eax,[esp+4]                     ;; get the idtr values
    lidt        [eax]                           ;; load the idt register
    ret

