;;===================================================================================================================
;;
;;  ArchIntNone.s -- A dummy interrupt handler for all unused interrupts
;;
;;        Copyright (c)  2017-2020 -- Adam Clark
;;        Licensed under "THE BEER-WARE LICENSE"
;;        See License.md for details.
;;
;; -----------------------------------------------------------------------------------------------------------------
;;
;;     Date      Tracker  Version  Pgmr  Description
;;  -----------  -------  -------  ----  --------------------------------------------------------------------------
;;  2020-Feb-01  Initial  v0.5.0f  ADCL  Initial version
;;
;;===================================================================================================================


%include "constants.inc"


;;
;; -- Expose some labels to other fucntions that the linker can pick up
;;    -----------------------------------------------------------------
    global      ArchIntNone


ArchIntNone:
    iret
