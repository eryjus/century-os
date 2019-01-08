;;===================================================================================================================
;;
;;  kMemSetB.s -- Set a block of memory to the specified value by bytes
;;
;;        Copyright (c)  2017-2019 -- Adam Clark
;;        Licensed under "THE BEER-WARE LICENSE"
;;        See License.md for details.
;;
;;  Set a block of memory to the specified value.  This function operates with bytes being passed into the function,
;;  so cnt contains the number of bytes to fill.
;;
;;  On entry, the stack has the following structure:
;;  +-----------+-----------+------------------------------------+
;;  |  via ebp  |  via esp  |  description of the contents       |
;;  +-----------+-----------+------------------------------------+
;;  | ebp + 16  | esp + 12  |  Number of bytes to set            |
;;  +-----------+-----------+------------------------------------+
;;  | ebp + 12  | esp + 08  |  The byte to set in the memory     |
;;  +-----------+-----------+------------------------------------+
;;  | ebp + 08  | esp + 04  |  The memory location to set        |
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
;;  void kMemSetB(void *buf, uint8_t byt, size_t cnt);
;;
;; ------------------------------------------------------------------------------------------------------------------
;;
;;     Date      Tracker  Version  Pgmr  Description
;;  -----------  -------  -------  ----  ---------------------------------------------------------------------------
;;  2018-May-30  Initial   0.1.0   ADCL  Copied this file from kMemSetW and updated
;;
;;===================================================================================================================


;;
;; -- Expose labels to fucntions that the linker can pick up
;;    ------------------------------------------------------
    global    kMemSetB


;;
;; -- This is the beginning of the code segment for this file
;;    -------------------------------------------------------
section     .text
cpu         586


;;
;; -- Clear or set a block of memory to the specified value
;;    -----------------------------------------------------
kMemSetB:
    push    ebp                                                 ;; create a frame
    mov     ebp,esp                                             ;; ... create a frame
    push    eax                                                 ;; save eax
    push    ecx                                                 ;; save ecx
    push    edi                                                 ;; save edi

    mov     eax,[ebp+8]                                         ;; get the memory location to set
    mov     edi,eax                                             ;; and put it in edi
    mov     ecx,[ebp+16]                                        ;; get the number of bytes to set
    mov     al,[ebp+12]                                         ;; get the byte to set
    cld                                                         ;; make sure we are incrementing
    rep     stosb                                               ;; store the byte

    pop     edi                                                 ;; restore edi
    pop     ecx                                                 ;; restore ecx
    pop     eax                                                 ;; restore eax
    pop     ebp                                                 ;; restore previous frame
    ret
