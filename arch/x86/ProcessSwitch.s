;;===================================================================================================================
;;
;;  ProcessSwitch.s -- Execute a task switch at the lowest switch
;;
;;  This function will perform a voluntary task switch, as opposed to a preemptive switch.  Therefore, there will
;;  never need to have an EOI sent to the PIC/APIC.
;;
;;  The following represent the stack structure based on the esp/frame set up at entry:
;;  +-----------+-----------+------------------------------------+
;;  |    EBP    |    ESP    |  Point of reference                |
;;  +-----------+-----------+------------------------------------+
;;  | ebp + 12  | esp + 08  |  The current Process_t structure   |
;;  +-----------+-----------+------------------------------------+
;;  | ebp + 08  | esp + 04  |  The target Process_t structure    |
;;  +-----------+-----------+------------------------------------+
;;  | ebp + 04  |   esp     |  Return EIP                        |
;;  +-----------+-----------+------------------------------------+
;;  |   ebp     | esp - 04  |  Frame pointer EBP                 |
;;  +-----------+-----------+------------------------------------+
;;  | ebp - 04  | esp - 08  |  EFLAGS                            |
;;  +-----------+-----------+------------------------------------+
;;  | ebp - 08  | esp - 12  |  EAX                               |
;;  +-----------+-----------+------------------------------------+
;;  | ebp - 12  | esp - 16  |  EBX                               |
;;  +-----------+-----------+------------------------------------+
;;  | ebp - 16  | esp - 20  |  ECX                               |
;;  +-----------+-----------+------------------------------------+
;;  | ebp - 20  | esp - 24  |  EDX                               |
;;  +-----------+-----------+------------------------------------+
;;  | ebp - 24  | esp - 28  |  ESI                               |
;;  +-----------+-----------+------------------------------------+
;;  | ebp - 28  | esp - 32  |  EDI                               |
;;  +-----------+-----------+------------------------------------+
;;  | ebp - 32  | esp - 36  |  CR0                               |
;;  +-----------+-----------+------------------------------------+
;;  | ebp - 36  | esp - 40  |  CR3                               |
;;  +-----------+-----------+------------------------------------+
;;  | ebp - 40  | esp - 44  |  DS                                |
;;  +-----------+-----------+------------------------------------+
;;  | ebp - 44  | esp - 48  |  ES                                |
;;  +-----------+-----------+------------------------------------+
;;  | ebp - 48  | esp - 52  |  FS                                |
;;  +-----------+-----------+------------------------------------+
;;  | ebp - 52  | esp - 56  |  GS                                |
;;  +-----------+-----------+------------------------------------+
;;
;; ------------------------------------------------------------------------------------------------------------------
;;
;;     Date      Tracker  Version  Pgmr  Description
;;  -----------  -------  -------  ----  ---------------------------------------------------------------------------
;;  2012-Sep-23                          Initial version
;;  2018-May-29  Initial   0.1.0   ADCL  Copied this file from century32 to century-os
;;  2019-Feb-09  Initial   0.3.0   ADCL  Relocated
;;
;;===================================================================================================================


;;
;; -- Now, expose our function to everyone
;;    ------------------------------------
    global  ProcessSwitch


;;
;; -- Some local equates for use with access structure elements
;;    ---------------------------------------------------------
PROC_STACKPOINTER       EQU     0
PROC_SS                 EQU     4
PROC_PAGE_TABLES        EQU     8

;;
;; -- This is the beginning of the code segment for this file
;;    -------------------------------------------------------
section     .text
cpu         586


;;
;; -- Execute a process switch
;;    ------------------------
ProcessSwitch:
        push    ebp                         ;; save the caller's frame pointer
        mov     ebp,esp                     ;; setup up a frame
        pushfd                              ;; save the current flags register
        cli                                 ;; and turn off interrupts so that we do not have a preempt

        push    eax                         ;; save eax
        push    ebx                         ;; save ebx
        push    ecx                         ;; save ecx
        push    edx                         ;; save edx
        push    esi                         ;; save esi
        push    edi                         ;; save edi

        mov     eax,cr0
        push    eax                         ;; save cr0
        mov     eax,cr3
        push    eax                         ;; save cr3

        xor     eax,eax                     ;; clear eax
        mov     ax,ds                       ;; get ds
        push    eax                         ;; and push it
        mov     ax,es                       ;; get es
        push    eax                         ;; and push it
        mov     ax,fs                       ;; get fs
        push    eax                         ;; and push it
        mov     ax,gs                       ;; get gs
        push    eax                         ;; and push it

        mov     esi,[ebp+8]                 ;; get the currentPID Process_t address
        mov     edi,[ebp+12]                ;; get the target PID Process_t address

        xor     eax,eax                     ;; clear eax
        mov     ax,ss                       ;; capture ss
        mov     [esi+PROC_SS],eax           ;; save the ss register
        mov     [esi+PROC_STACKPOINTER],esp ;; save the stack pointer

        mov     eax,cr3                     ;; get the cr3 value
        mov     [esi+PROC_PAGE_TABLES],eax  ;; and store it in the process structure

;;
;; -- Check how we need to actually perform the task change
;;    -----------------------------------------------------
        mov     eax,[edi+PROC_PAGE_TABLES]  ;; get the new cr3 value
        mov     cr3,eax                     ;; and set the new paging structure (flushes all TLB caches)

        mov     eax,ss                      ;; get the ss register again
        cmp     eax,[edi+PROC_SS]           ;; are we moving to the same stack selector?
        je      NewProcess                  ;; if the same we can do a simple load

        ;; -- Push the new ss register
        xor     eax,eax                     ;; clear eax
        mov     ax,[edi+PROC_SS]            ;; get the stack segment
        push    eax

        ;; -- Push the new esp register
        mov     eax,[edi+PROC_STACKPOINTER] ;; get the target esp
        push    eax                         ;; push it on the stack

        ;; -- Push the current flags register
        pushfd                              ;; push the current flags (will be replaced later)

        ;; -- Push the new cs register
        xor     eax,eax                     ;; clear eax
        mov     ax,[edi+PROC_SS]            ;; get the stack segment
        sub     eax,8                       ;; calcualte the code segment -- this only works because of GDT layout
        push    eax                         ;; and push it on the stack

        ;; -- push the new eip register
        mov     eax,ChangeCPL               ;; get the address of the return point
        push    eax
        iret                                ;; effectively load the ss/esp and cs/eip registers


;;
;; -- This completes saving the current process and below we start to restore the target process context
;;    --------------------------------------------------------------------------------------------------
NewProcess:
        mov     eax,[edi+PROC_SS]           ;; get the ss register
        mov     ss,ax                       ;; and restore it
        mov     esp,[edi+PROC_STACKPOINTER] ;; restore the stack pointer

ChangeCPL:
        pop     eax                         ;; pop the gs register
        mov     gs,ax                       ;; and restore it
        pop     eax                         ;; pop the fs register
        mov     fs,ax                       ;; and restore it
        pop     eax                         ;; pop the es register
        mov     es,ax                       ;; and restore it
        pop     eax                         ;; pop the ds register
        mov     ds,ax                       ;; and restore it

        pop     eax                         ;; pop cr3; discarded
        pop     eax                         ;; pop cr0

        pop     edi                         ;; restore edi
        pop     esi                         ;; restore esi
        pop     edx                         ;; restore edx
        pop     ecx                         ;; restore ecx
        pop     ebx                         ;; restore ebx
        pop     eax                         ;; restore eax

        popfd                               ;; pop flags and restore interrupt status
        pop     ebp                         ;; restore ebp
        ret