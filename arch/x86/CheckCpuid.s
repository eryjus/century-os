;;===================================================================================================================
;;
;;  CheckCpuid.s -- Checks whether the CPUID opcode is supported
;;
;;        Copyright (c)  2017-2020 -- Adam Clark
;;        Licensed under "THE BEER-WARE LICENSE"
;;        See License.md for details.
;;
;; ------------------------------------------------------------------------------------------------------------------
;;
;;     Date      Tracker  Version  Pgmr  Description
;;  -----------  -------  -------  ----  ---------------------------------------------------------------------------
;;  2019-Apr-09  Initial   0.4.0   ADCL  Initial version
;;
;;===================================================================================================================


;;
;; -- Expose labels to fucntions that the linker can pick up
;;    ------------------------------------------------------
    global CheckCpuid


;;
;; -- This is the beginning of the code segment for this file
;;    -------------------------------------------------------
section .ldrtext
cpu        586


;;
;; -- CheckCpuid() -- Check if CPUID is supported
;;    -------------------------------------------
CheckCpuid:
    pushfd                              ;; -- save the original flags
    pushfd                              ;; -- save a copy of the flags to manipulate
    xor     dword [esp],(1<<21)         ;; -- toggle the ID bit: off is now on; on is now off
    popfd                               ;; -- restore the flags; if not supported this bit will not update
    pushfd                              ;; -- back on the stack to get the new real value
    pop     eax                         ;; -- get the result into eax
    xor     eax,[esp]                   ;; -- if there was a change, then eax will have bit 21 set; no change clear
    popfd                               ;; -- restore the original flags since we are nice programmers
    and     eax,(1<<21)                 ;; -- mask out the bit we are interested in
    ret                                 ;; -- return to the caller; eax <> 0 means supported

