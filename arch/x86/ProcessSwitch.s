;;===================================================================================================================
;;
;;  ProcessSwitch.s -- Execute a task switch at the lowest level
;;
;;  This function will perform a task switch.
;;
;;  The following represent the stack structure based on the esp/frame set up at entry:
;;  +-----------+------------------------------------+
;;  |    ESP    |  Point of reference                |
;;  +===========+====================================+
;;  | esp + 04  |  The target Process_t structure    |
;;  +-----------+------------------------------------+
;;  |   esp     |  Return EIP                        |
;;  +-----------+------------------------------------+
;;
;;  There are a couple of call-outs here for the changes.
;;  1) I was saving everything and it was overkill.  There is no need for this complexity
;;  2) eax,ecx, and edx are considered to be argument/return registers for the arm abi.  Any interesting value
;;     would have been saved by the caller so no need to worry about them.
;;  3) The virtual address space register does not need to be saved; it is static once identified and therefore
;;     it is already saved in the structure.
;;
;; ------------------------------------------------------------------------------------------------------------------
;;
;;  Function Prototype:
;;  extern "C" void ProcessSwitch(Process_t *nextProcess);
;;
;; ------------------------------------------------------------------------------------------------------------------
;;
;;     Date      Tracker  Version  Pgmr  Description
;;  -----------  -------  -------  ----  ---------------------------------------------------------------------------
;;  2012-Sep-23                          Initial version
;;  2018-May-29  Initial   0.1.0   ADCL  Copied this file from century32 to century-os
;;  2019-Feb-09  Initial   0.3.0   ADCL  Relocated
;;  2019-Mar-16  Initial   0.3.2   ADCL  Clean up the scheduler
;;
;;===================================================================================================================


;;
;; -- Now, expose our function to everyone
;;    ------------------------------------
    global  ProcessSwitch


;;
;; -- Some global variables that are referenced
;;    -----------------------------------------
    extern  currentProcess
    extern  schedulerLocksHeld
    extern  processChangePending


;;
;; -- Some local equates for use with access structure elements
;;    ---------------------------------------------------------
PROC_TOP_OF_STACK       EQU     0
PROC_VIRT_ADDR_SPACE    EQU     4
PROC_STATUS             EQU     8
PROC_PRIORITY           EQU     12
PROC_QUANTUM_LEFT       EQU     16


;;
;; -- Some additional constants for use when managing process status
;;    --------------------------------------------------------------
PROC_STS_RUNNING        EQU     1
PROC_STS_READY          EQU     2


;;
;; -- This is the beginning of the code segment for this file
;;    -------------------------------------------------------
section     .text
cpu         586


;;
;; -- Execute a process switch
;;    ------------------------
ProcessSwitch:
;;
;; -- before we do too much, do we need to postpone?
;;    ----------------------------------------------
        cmp     dword [schedulerLocksHeld],0
        je      .cont

        mov     dword [processChangePending],1
        ret

;;
;; -- From here we take on the task change
;;    ------------------------------------
.cont:
        push    ebx                         ;; save ebx
        push    esi                         ;; save esi
        push    edi                         ;; save edi
        push    ebp                         ;; save the caller's frame pointer


;;
;; -- Get the current task structure
;;    ------------------------------
        mov     esi,[currentProcess]        ;; get the address of the current process

        cmp     dword [esi+PROC_STATUS],PROC_STS_RUNNING      ;; is this the current running process
        jne     .saveStack
        mov     dword [esi+PROC_STATUS],PROC_STS_READY    ;; make the status read to run

.saveStack:
        mov     [esi+PROC_TOP_OF_STACK],esp ;; save the top of the current stack


;;
;; -- next, we get the next task and prepare to switch to that
;;    --------------------------------------------------------
        mov     edi,[esp+((4+1)*4)]         ;; get the new task's structure
        mov     [currentProcess],edi        ;; this is now the currnet task

        mov     esp,[edi+PROC_TOP_OF_STACK] ;; get the stop of the next process stack
        mov     dword [edi+PROC_STATUS],PROC_STS_RUNNING    ;; set the new process to be running
        mov     eax,dword [edi+PROC_PRIORITY]   ;; get the priority, which becomes the next quantum
        add     dword [edi+PROC_QUANTUM_LEFT],eax   ;; add it to the amount left to overcome "overdrawn" procs
        mov     eax,[edi+PROC_VIRT_ADDR_SPACE]  ;; get the paing tables address
        mov     ecx,cr3                     ;; get the current paging tables

        cmp     eax,ecx                     ;; are they the same?
        je      .noVASchg                   ;; no need to perform a TLB flush

        mov     cr3,eax                     ;; replace the paging tables

.noVASchg:
        pop     ebp                         ;; restore ebp
        pop     edi                         ;; restore edi
        pop     esi                         ;; restore esi
        pop     ebx                         ;; restore ecx

        ret

