;;===================================================================================================================
;;
;; loader/src/i686/entry.s -- This is the entry point for the x86 32-bit processors
;;
;;        Copyright (c)  2017-2018 -- Adam Clark
;;        Licensed under "THE BEER-WARE LICENSE"
;;        See License.md for details.
;;
;; Both multiboot 1 and 2 standards are supported.  This means that this loader can be booted by either GRUB
;; and GRUB2.  We will let GRUB establish the video resolution.
;;
;; -----------------------------------------------------------------------------------------------------------------
;;
;;    Date     Tracker  Version  Pgmr  Description
;; ----------  -------  -------  ----  ---------------------------------------------------------------------------
;; 2017-04-09  Initial   0.0.0   ADCL  Initial version
;; 2018-06-02  Initial   0.1.0   ADCL  Copied this file from century to century-os
;;
;;===================================================================================================================


;;
;; -- Expose some global addresses
;;    ----------------------------
global          _start
global          systemFont
global          Halt


;;
;; -- import some external symbols
;;    ----------------------------
extern          kMemMove
extern          gdt
extern          tss
extern          mb1Data
extern          mb2Data
extern          LoaderMain


;;
;; -- This is the multiboot header.  During the link process, it will be relocated to the beginning of the
;;    resulting ELF file.  This is required since the multiboot header needs to be in the first 4/8K (I think).
;;    ---------------------------------------------------------------------------------------------------------
section         .mboot


;;
;; -- The following are used to populate the multiboot v1 header
;;    ----------------------------------------------------------
MAGIC           equ     0x1badb002
FLAGS           equ     (1<<1 | 1<<2)                       ;; get mem & set video

;;
;; -- The following are used to populate the multiboot v2 header
;;    ----------------------------------------------------------
MAGIC2          equ     0xe85250d6
LEN             equ     MultibootHeader2End - MultibootHeader2
CHECK2          equ     (-(MAGIC2 + LEN) & 0xffffffff)


;;
;; -- Standard Video resolution
;;    -------------------------
MODE_TYPE       equ     0
WIDTH           equ     1024
HEIGHT          equ     768
DEPTH           equ     16


;;
;; -- This is the stack size
;;    ----------------------
STACK_SIZE      equ     4096


;;
;; -- This is the multiboot 1 header
;;    ------------------------------
align           4
multiboot_header:
;; -- magic fields
dd      MAGIC
dd      FLAGS
dd      -MAGIC-FLAGS
;; -- address fields (unused placeholders)
dd      0
dd      0
dd      0
dd      0
dd      0
;; -- video fields
dd      MODE_TYPE
dd      WIDTH
dd      HEIGHT
dd      DEPTH


;;
;; -- This is the multiboot 2 header
;;    ------------------------------
align           8
MultibootHeader2:
dd      MAGIC2
dd      0                                                   ;; architecture: 0=32-bit protected mode
dd      LEN                                                 ;; total length of the mb2 header
dd      CHECK2                                              ;; mb2 checksum

align           8
Type4Start:
dw      4                                                   ;; type=4
dw      1                                                   ;; Graphics
dd      Type4End-Type4Start                                 ;; size = 12
dd      0                                                   ;; graphics support
Type4End:

align           8
Type5Start:
dw      5                                                   ;; graphic mode
dw      1                                                   ;; not optional
dd      Type5End-Type5Start                                 ;; size = 20
dd      WIDTH                                               ;; 1024
dd      HEIGHT                                              ;; 768
dd      DEPTH                                               ;; 16
Type5End:

align           8
Type6Start:
dw      6                                                   ;; Type=6
dw      1                                                   ;; Not optional
dd      Type6End-Type6Start                                 ;; size = 8 bytes even tho the doc says 12
Type6End:

align           8
dw      0                                                   ;; Type=0
dw      0                                                   ;; flags=0
dd      8                                                   ;; size=8
MultibootHeader2End:

GdtDesc:
dw      (16*8)-1                                            ;; size minus one...
dd      0                                                   ;; this is the addresss


;;
;; -- This section holds the main entry point.  Again, it will be located behind the multiboot header.
;;    ------------------------------------------------------------------------------------------------
section         .init

_start:
    push        eax
    push        ebx

    push        (16*8)                                      ;; 128 bytes
    lea         eax,[gdt]
    push        eax                                         ;; push the source
    push        0                                           ;; push the target
    call        kMemMove
    add         esp,12

    pop         ebx
    pop         eax

    mov         ecx,GdtDesc                                 ;; Get the gdt address
    lgdt        [ecx]                                       ;; and load the GDT

    mov         ecx,0x08<<3
    mov         ds,cx
    mov         es,cx
    mov         fs,cx
    mov         gs,cx
    mov         ss,cx                                       ;; halts ints for one more instruction to set up stack
    mov         esp,stack+STACK_SIZE                        ;; set up a stack at 2MB growing down

    push        0x07<<3
    push        newGDT
    retf                                                    ;; an almost jump, returning to an addr rather than jump

newGDT:
    cmp         eax,0x2badb002
    jne         chkMB2
    mov         [mb1Data],ebx
    jmp         ldrMain

chkMB2:
    cmp         eax,0x36d76289
    jne         ldrMain
    mov         [mb2Data],ebx

ldrMain:
    jmp         LoaderMain

Halt:
    cli
    hlt
    jmp         Halt


;;
;; -- This is where we include the binary data for the system font
;;    ------------------------------------------------------------
section         .data

systemFont:
incbin          "system-font.bin"


;;
;; -- Declare a stack for the loader
;;    ------------------------------
section         .bss
stack:
    resb        STACK_SIZE


