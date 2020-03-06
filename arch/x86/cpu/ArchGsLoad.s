;;===================================================================================================================
;;
;;  ArchGsLoad.s -- Load the GS selector with the value passed in
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
    global      ArchGsLoad


;;
;; -- Load the GS register
;;    --------------------
ArchGsLoad:
    mov     eax,[esp+4]
    mov     gs,ax
    ret
