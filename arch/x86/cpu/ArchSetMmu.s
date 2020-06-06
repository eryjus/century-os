;;===================================================================================================================
;;
;;  ArchSetMmu.s -- Set the top level MMU structure (user)
;;
;;        Copyright (c)  2017-2020 -- Adam Clark
;;        Licensed under "THE BEER-WARE LICENSE"
;;        See License.md for details.
;;
;; -----------------------------------------------------------------------------------------------------------------
;;
;;     Date      Tracker  Version  Pgmr  Description
;;  -----------  -------  -------  ----  --------------------------------------------------------------------------
;;  2020-Apr-19  Initial  v0.7.0a  ADCL  Initial version
;;
;;===================================================================================================================


;;
;; -- Expose some addresses for other things to use
;;    ---------------------------------------------
    global  ArchSetMmu


;;
;; -- This is the beginning of the code segment for this file
;;    -------------------------------------------------------
    section .text
    cpu     586


;;
;; -- Set the top level MMU structure
;;    -------------------------------
ArchSetMmu:
    push ebx                ;; save the contents of ebx
    mov  ebx,[esp+8]        ;; get the new CR3 value

    mov  eax,cr3            ;; get the current value
    mov  cr3,ebx            ;; set the new value (includes TLB shootdown)

    pop  ebx                ;; restore
    ret


