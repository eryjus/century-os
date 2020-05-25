;;===================================================================================================================
;;
;;  MmuGetTopTable.s -- Return the top-level user MMU table value
;;
;;        Copyright (c)  2017-2020 -- Adam Clark
;;        Licensed under "THE BEER-WARE LICENSE"
;;        See License.md for details.
;;
;; ------------------------------------------------------------------------------------------------------------------
;;
;;     Date      Tracker  Version  Pgmr  Description
;;  -----------  -------  -------  ----  ---------------------------------------------------------------------------
;;  2020-Apr-21  Initial  v0.7.0a  ADCL  Initial version
;;
;;===================================================================================================================


;;
;; -- Expose labels to fucntions that the linker can pick up
;;    ------------------------------------------------------
    global MmuGetTopTable


;;
;; -- This is the beginning of the code segment for this file
;;    -------------------------------------------------------
    section .text
    cpu     586


;;
;; -- MmuGetTopTable -- get the CR3
;;    ---------------------------------
MmuGetTopTable:
    mov     eax,cr3
    ret
