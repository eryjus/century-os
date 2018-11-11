;;===================================================================================================================
;;
;;  SendMessage.s -- Execute a System Call to send a message
;;
;;        Copyright (c)  2017-2018 -- Adam Clark
;;        Licensed under "THE BEER-WARE LICENSE"
;;        See License.md for details.
;;
;; -----------------------------------------------------------------------------------------------------------------
;;
;;    Date      Tracker  Version  Pgmr  Description
;; -----------  -------  -------  ----  ---------------------------------------------------------------------------
;; 2018-Nov-02  Initial   0.1.0  ADCL  Initial version
;;
;;===================================================================================================================


;;
;; -- Expose labels to fucntions that the linker can pick up
;;    ------------------------------------------------------
    global      SendMessage


;;
;; -- This is the beginning of the code segment for this file
;;    -------------------------------------------------------
section     .text
cpu		    586


;;
;; -- Execute a System Call to Recieve a Message
;;    ------------------------------------------
SendMessage:
    push        ebp
    mov         ebp,esp
    push        edx
    push        edi

    mov         eax,1
    mov         edx,[ebp+8]
    mov         edi,[ebp+12]
    int         100

    pop         edi
    pop         edx
    pop         ebp
    ret

