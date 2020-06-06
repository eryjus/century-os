@@===================================================================================================================
@@
@@  entryAp.s -- This is the entry point for the Application Processors, where the cpu number is > 0
@@
@@        Copyright (c)  2017-2020 -- Adam Clark
@@        Licensed under "THE BEER-WARE LICENSE"
@@        See License.md for details.
@@
@@ ------------------------------------------------------------------------------------------------------------------
@@
@@     Date      Tracker  Version  Pgmr  Description
@@  -----------  -------  -------  ----  ----------------------------------------------------------------------------
@@  2019-Jun-08  Initial   0.4.6   ADCL  Initial version
@@
@@===================================================================================================================


.include "constants.inc"

@@
@@ -- make sure we have a value for whether we include debug code
@@    -----------------------------------------------------------
.ifndef ENABLE_DEBUG_ENTRY
    .equ        ENABLE_DEBUG_ENTRY,0
.endif


@@
@@ -- make sure that if the required symbols are defined; Branch Predictor
@@    --------------------------------------------------------------------
.ifndef ENABLE_BRANCH_PREDICTOR
    .equ        ENABLE_BRANCH_PREDICTOR,0
.endif


@@
@@ -- Cache (instruction and data)
@@    ----------------------------
.ifndef ENABLE_CACHE
    .equ        ENABLE_CACHE,0
.endif


@@
@@ -- explose some global symbols
    .global     entryAp
    .global     entryApHold


@@
@@ -- This is the loader text section.  This section will be reclaimed once initialization is complete.
@@    -------------------------------------------------------------------------------------------------
    .section    .text.entry,"ax"


@@
@@ -- Just in case more than one core slips through, we will hold them here; r3 holds core number
@@    -------------------------------------------------------------------------------------------
entryApHold:
    wfe                                 @@ wait to be released
    mov     r0,#0x8c                    @@ the bottom end of the wanted mailbox
    movt    r0,#0x4000                  @@ the top end of the wanted mailbox
    mov     r1,r3                       @@ get the core number
    lsl     r1,#4                       @@ convert to 16-byte offset
    add     r0,r0,r1                    @@ r1 now holds the address of the mailbox we want

    ldr     r2,[r0]                     @@ get the address of the target location
    str     r2,[r0]                     @@ write the value back to clear the bits
    cmp     r2,#0                       @@ is this address 0?
    beq     entryApHold                 @@ if so, loop and wait
    bx      r2                          @@ otherwise jump to the address (should be entryAp below)


@@
@@ -- This is the AP entry point.  In this function, we need to set up the cpu to the state we want to
@@    have it operate in before we start looking for jobs to run.
@@    ------------------------------------------------------------------------------------------------
entryAp:
@@
@@ -- make sure this core is in svc mode -- may be in hyp mode
@@    --------------------------------------------------------
    mrs     r0,cpsr                     @@ get the current program status register
    and     r0,#0x1f                    @@ and mask out the mode bits
    cmp     r0,#0x1a                    @@ are we in hyp mode?
    beq     hyp                         @@ if we are in hyp mode, go to that section
    cpsid   iaf,#0x13                   @@ if not switch to svc mode, ensure we have a stack for the kernel; no ints
    b       cont                        @@ and then jump to set up the stack

@@ -- from here we are in hyp mode so we need to exception return to the svc mode
hyp:
    mrs     r0,cpsr                     @@ get the cpsr again
    and     r0,#~0x1f                   @@ clear the mode bits
    orr     r0,#0x013                   @@ set the mode for svc
    orr     r0,#1<<6|1<<7|1<<8          @@ disable interrupts as well
    msr     spsr_cxsf,r0                @@ and save that in the spsr
    ldr     r0,=cont                    @@ get the address where we continue
    msr     elr_hyp,r0                  @@ store that in the elr register

    eret                                @@ this is an exception return

@@
@@ -- some early CPU initialization
@@    -----------------------------
cont:
    mrc     p15,0,r0,c1,c0,0            @@ get the SCTLR
.if ENABLE_BRANCH_PREDICTOR
    orr     r0,#(1<<11)                 @@ set the Z bit for branch prediction (may be forced on by HW!)
.endif

.if ENABLE_CACHE
    orr     r0,#(1<<2)                  @@ set the data cache enabled
    orr     r0,#(1<<12)                 @@ set the instruction cache enabled
.endif
    mcr     p15,0,r0,c1,c0,0            @@ write the SCTLR back with the caches enabled

    ldr     sp,=stack_top                       @@ set the stack

@@ -- Set up the VBAR to use an absolute address
    ldr     r0,=intTableAddr            @@ get the location we will share the vbar frame
    ldr     r0,[r0]                     @@ get the table address

    mrc     p15,0,r1,c1,c0,0
    and     r1,r1,#(~(1<<13))
    mcr     p15,0,r1,c1,c0,0

    mcr     p15,0,r0,c12,c0,0           @@ !!! physical addr (identity mapped); will need to change in kernel mmu


@@
@@ -- before we can get a stack, we need to enable paging
@@    ---------------------------------------------------
@@ -- before we enable paging, we need to decorate the MAIR registers.
    mov     r2,#(ARMV7_MAIR0_VAL & 0xffff)
    movt    r2,#(ARMV7_MAIR0_VAL >> 16)
    mcr     p15,0,r2,c10,c2,0

    mov     r2,#(ARMV7_MAIR1_VAL & 0xffff)
    movt    r2,#(ARMV7_MAIR1_VAL >> 16)
    mcr     p15,0,r2,c10,c2,1
    dsb
    isb

@@ -- set the TTBR0/1 registers
    ldr     r0,=mmuLvl1Table            @@ get the address of the frame variable
    ldr     r0,[r0]                     @@ and then get the frame address itself
    mov     r1,#0

@@ -- set the paging tables
    mcrr    p15,0,r0,r1,c2              @@ write the ttl1 table to the TTBR0 register
    mcrr    p15,1,r0,r1,c2              @@ write the ttl1 table to the TTBR1 register as well; will use later
    dsb
    isb

@@ -- Updage the TTBCR
    mov     r1,#0x3501                          @@ the low bits for the TTBCR
    movt    r1,#0xb500                          @@ set the long descriptor format
    mcr     p15,0,r1,c2,c0,2                    @@ write these to the TTBCR
    dsb
    isb

@@ -- set the DACR register
    mov     r1,#0xffffffff              @@ All domains can manage all things by default
    mcr     p15,0,r1,c3,c0,0            @@ write these to the domain access register
    dsb
    isb

@@ -- now we enable paging
    mrc     p15,0,r1,c1,c0,0            @@ This gets the cp15 register 1 and puts it in r0
    orr     r1,#1                       @@ set bit 0
    mcr     p15,0,r1,c1,c0,0            @@ Put the cp15 register 1 back, with the MMU enabled
    dsb
    isb

.if ENABLE_BRANCH_PREDICTOR
    mcr     p15,0,r0,c7,c5,6            @@ invalidate the branch predictor (required maintenance when enabled)
.endif

.if ENABLE_CACHE
    mcr     p15,0,r0,c7,c1,0            @@ invalidate all instruction caches (required maintenance when enabled)
.endif


@@
@@ -- Now, we need to set the address of the cpu structure
@@    ----------------------------------------------------
    bl      CpuMyStruct                 @@ perform the initalization and get the address of this cpu struct
    mcr     p15,0,r0,c13,c0,4           @@ save this address in the tpidrprw register
    ldr     sp,[r0,#4]                  @@ get the new stack
    b       kInitAp                     @@ jump to the AP initialization code


