;;===================================================================================================================
;;
;; ReceiveMessage.s -- Execute a System Call to recieve a message
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
global      ReceiveMessage


;;
;; -- This is the beginning of the code segment for this file
;;    -------------------------------------------------------
section     .text
cpu		    586


;;
;; -- Execute a System Call to Recieve a Message
;;    ------------------------------------------
ReceiveMessage:
    push        ebp
    mov         ebp,esp
    push        edi

    mov         eax,1
    mov         edi,[ebp+8]
    int         100

    pop         edi
    pop         ebp
    ret

