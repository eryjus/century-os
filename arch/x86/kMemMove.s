;;===================================================================================================================
;;
;;  kMemMove.s -- Copy a block of memory to the specified location
;;
;;        Copyright (c)  2017-2020 -- Adam Clark
;;        Licensed under "THE BEER-WARE LICENSE"
;;        See License.md for details.
;;
;;  Copy a block of memory to the specified location
;;
;;  On entry, the stack has the following structure:
;;  +-----------+-----------+------------------------------------+
;;  |  via ebp  |  via esp  |  description of the contents       |
;;  +-----------+-----------+------------------------------------+
;;  | ebp + 16  | esp + 12  |  Number of bytes to set            |
;;  +-----------+-----------+------------------------------------+
;;  | ebp + 12  | esp + 08  |  The memory location to copy       |
;;  +-----------+-----------+------------------------------------+
;;  | ebp + 08  | esp + 04  |  The target location in memory     |
;;  +-----------+-----------+------------------------------------+
;;  | ebp + 04  |   esp     |  Return EIP                        |
;;  +-----------+-----------+------------------------------------+
;;  |   ebp     | esp - 04  |  EBP                               |
;;  +-----------+-----------+------------------------------------+
;;  | ebp - 04  | esp - 08  |  EAX                               |
;;  +-----------+-----------+------------------------------------+
;;  | ebp - 08  | esp - 12  |  ECX                               |
;;  +-----------+-----------+------------------------------------+
;;  | ebp - 12  | esp - 16  |  EDI                               |
;;  +-----------+-----------+------------------------------------+
;;
;;  Prototype:
;;  void kMemMove(void *tgt, void *src, size_t cnt);
;;
;; ------------------------------------------------------------------------------------------------------------------
;;
;;     Date      Tracker  Version  Pgmr  Description
;;  -----------  -------  -------  ----  ---------------------------------------------------------------------------
;;  2013-Aug-21    #56                   Initial version
;;  2018-May-29  Initial   0.1.0   ADCL  Copied this file from century32 (kMemcpy.s) to century-os
;;  2019-Feb-09  Initial   0.3.0   ADCL  Relocated
;;
;;===================================================================================================================


;;
;; -- Expose labels to fucntions that the linker can pick up
;;    ------------------------------------------------------
    global  kMemMove


;;
;; -- This is the beginning of the code segment for this file
;;    -------------------------------------------------------
section     .text
cpu         586


;;
;; -- Copy a block of memory to a new location
;;    ----------------------------------------
kMemMove:
    push    ebp                                                 ;; create a frame
    mov     ebp,esp                                             ;; ... create a frame
    push    eax                                                 ;; save eax
    push    ecx                                                 ;; save ecx
    push    esi                                                 ;; save esi
    push    edi                                                 ;; save edi

    mov     eax,[ebp+8]                                         ;; get the memory location to copy
    mov     edi,eax                                             ;; and put it in edi
    mov     ecx,[ebp+16]                                        ;; get the number of bytes to copy
    mov     eax,[ebp+12]                                        ;; get the source memory location
    mov     esi,eax                                             ;; and put it in esi
    cld                                                         ;; make sure we are incrementing
    rep     movsb                                               ;; copy the bytes

    pop     edi                                                 ;; restore edi
    pop     esi                                                 ;; restore esi
    pop     ecx                                                 ;; restore ecx
    pop     eax                                                 ;; restore eax
    pop     ebp                                                 ;; restore previous frame
    ret
