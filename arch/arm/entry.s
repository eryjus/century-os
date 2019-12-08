@@===================================================================================================================
@@
@@  entry.s -- This will be the entry point for the combined loader/kernel.
@@
@@        Copyright (c)  2017-2019 -- Adam Clark
@@        Licensed under "THE BEER-WARE LICENSE"
@@        See License.md for details.
@@
@@  All of the early initialization will take place here.  For ARM, this will be to set up the Interrupt Vector
@@  Table, the Paging Tables for TTBR0 (no split tables yet), and then enable paging.  Once those are done, we can
@@  begin initialization.
@@
@@  One additional note here, the rpi2b at least will start the CPU in 'hyp' mode.  If this is the case, we need
@@  to thunk it back down into 'svc' mode where this kernel is intended to run.
@@
@@ ------------------------------------------------------------------------------------------------------------------
@@
@@     Date      Tracker  Version  Pgmr  Description
@@  -----------  -------  -------  ----  ----------------------------------------------------------------------------
@@  2019-Feb-09  Initial   0.3.0   ADCL  Initial version
@@
@@===================================================================================================================


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
    .global     entry
    .global     pmmEarlyFrame           @@ -- picked up by the pmm for initialization
    .global     NextEarlyFrame
    .global     JumpKernel
    .global     DecorateRegs

@@
@@ -- This is the multiboot header.  During the link process, it will be relocated to the beginning of the
@@    resulting ELF file.  This is required since the multiboot header needs to be in the first 4/8K (I think).
@@    ---------------------------------------------------------------------------------------------------------
    .section    .mboot,"ax"


@@
@@ -- The following are used to populate the multiboot v1 header
@@    ----------------------------------------------------------
    .equ        MAGIC,0x1badb002
    .equ        FLAGS,1<<1 | 1<<2       @@ -- 4K align; memory map


@@
@@ -- This is the multiboot 1 header
@@    ------------------------------
multiboot_header:
@@ -- magic fields
    .long       MAGIC
    .long       FLAGS
    .long       0-MAGIC-FLAGS


@@
@@ -- This is the loader text section.  This section will be reclaimed once initialization is complete.
@@    -------------------------------------------------------------------------------------------------
    .section    .ldrtext,"ax"


@@
@@ -- This is the entry point.  Multiboot will load this kernel image at 0x100000.  The first order of
@@    business is to dump us out of 'hyp' mode if we are there and to make sure we are in 'svc' mode
@@    before we go on.
@@
@@    On entry r0 holds the proper multiboot signature for MB1, r1 holds the address of the MBI,
@@    and r2 holds the value 0.
@@    ------------------------------------------------------------------------------------------------
entry:
@@
@@ -- figure out which CPU we are on; only CPU 0 continues after this
@@
@@    Note that this code is expected to be _ALWAYS_ executing on Core0.  This check is there just in case
@@    something happens later down the road with a change/bug in `bootcode.bin`.  If this were to happen,
@@    we will emulate the expected behavior of the firmware by holding the core in a holding pen until we
@@    are ready to release it.
@@    ----------------------------------------------------------------------------------------------------
    mrc     p15,0,r3,c0,c0,5            @@ Read Multiprocessor Affinity Register
    and     r3,r3,#0x3                  @@ Extract CPU ID bits
    cmp     r3,#0
    bne     entryApHold                 @@ if we’re not on CPU0 go to the holding pen

@@ -- Save off the MBI structure
save:
    ldr     r2,=mb1Data                 @@ get the address to put it in
    str     r1,[r2]                     @@ and save the address

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

@@ -- everyone continues from here
cont:
    mov     sp,#0x8000                  @@ set the stack

@@
@@ -- some early CPU initialization
@@    -----------------------------
.if ENABLE_BRANCH_PREDICTOR
    mrc     p15,0,r3,c1,c0,0            @@ get the SCTLR
    orr     r3,#(1<<11)                 @@ set the Z bit for branch prediction (may be forced on by HW!)
    mcr     p15,0,r3,c1,c0,0            @@ write the SCTLR back with the branch predictor guaranteed enabled
.endif

.if ENABLE_CACHE
    mrc     p15,0,r3,c1,c0,0            @@ get the SCTLR
    orr     r3,#(1<<2)                  @@ set the data cache enabled
    orr     r3,#(1<<12)                 @@ set the instruction cache enabled
    mcr     p15,0,r3,c1,c0,0            @@ write the SCTLR back with the caches enabled
.endif


@@ -- Clear out bss
initialize:
    ldr     r4,=_bssStart
    ldr     r9,=_bssEnd

    ldr     r5,=kern_loc
    ldr     r5,[r5]
    ldr     r6,=phys_loc
    ldr     r6,[r6]
    ldr     r7,=_loaderEnd
    ldr     r8,=_loaderStart

    sub     r4,r5
    add     r4,r6
    add     r4,r7
    sub     r4,r8

    sub     r9,r5
    add     r9,r6
    add     r9,r7
    sub     r9,r8

    mov     r5,#0
    mov     r6,#0
    mov     r7,#0
    mov     r8,#0

bssLoop:
@@ -- store multiple at r4
    stmia   r4!, {r5-r8}

@@ -- If we're still below _bssEnd, loop
    cmp     r4,r9
    blo     bssLoop

@@
@@ -- This next section is concerned with setting up the Vector table and getting it registered
@@    -----------------------------------------------------------------------------------------

@@ -- first a little housekeeping -- we need to allocate the ttb1 table (see below)
    bl      NextEarlyFrame              @@ call 4 times to get 4 frames!  need 16K
    bl      NextEarlyFrame
    bl      NextEarlyFrame
    bl      NextEarlyFrame              @@ this will be the lowest frame, so the start of the ttb1
    lsl     r0,#12                      @@ r0 now has the address of our new ttb1 table
    ldr     r1,=mmuLvl1Table            @@ get the location we will share the ttb1 frame
    str     r0,[r1]                     @@ save that value

@@ -- get a frame and initialize the Vector Table
    bl      NextEarlyFrame
    lsl     r0,#12                      @@ r0 now has the address of our new Vector Table
    ldr     r1,=intTableAddr            @@ get the location we will share the vbar frame
    str     r0,[r1]                     @@ save that value

@@ -- prepare the bit-coded jump instruction
    movw    r1,#0xf018                  @@ and fill in the bottom 2 bytes
    movt    r1,#0xe59f                  @@ this is the bitcode of the jump instruction

@@ -- fill in the jump instructions
    str     r1,[r0,#0x00]               @@ These are the jump instructions (not used -- reset)
    str     r1,[r0,#0x04]               @@ These are the jump instructions (UNDEF)
    str     r1,[r0,#0x08]               @@ These are the jump instructions (SVC CALL)
    str     r1,[r0,#0x0c]               @@ These are the jump instructions (PREFETCH ABORT)
    str     r1,[r0,#0x10]               @@ These are the jump instructions (DATA ABORT)
    str     r1,[r0,#0x14]               @@ These are the jump instructions (not used -- hyp mode)
    str     r1,[r0,#0x18]               @@ These are the jump instructions (IRQ INT)
    str     r1,[r0,#0x1c]               @@ These are the jump instructions (FIQ INT)

@@ -- fill in the jump targets
    ldr     r1,=ResetTarget
    str     r1,[r0,#0x20]               @@ The target for a reset (never used but filled in anyway)

    ldr     r1,=UndefinedTarget
    str     r1,[r0,#0x24]               @@ The target for an UNDEF

    ldr     r1,=SuperTarget
    str     r1,[r0,#0x28]               @@ The target for SUPER

    ldr     r1,=PrefetchTarget
    str     r1,[r0,#0x2c]               @@ The target for PREFETCH ABORT

    ldr     r1,=DataAbortTarget
    str     r1,[r0,#0x30]               @@ The target for DATA ABORT

    mov     r1,#0
    str     r1,[r0,#0x34]               @@ The target for not used (hyp)

    ldr     r1,=IRQTarget
    str     r1,[r0,#0x38]               @@ The target for IRQ

    ldr     r1,=FIQTarget
    str     r1,[r0,#0x3c]               @@ The target for FIQ

@@ -- Set up the VBAR to use an absolute address
    mrc     p15,0,r1,c1,c0,0
    and     r1,r1,#(~(1<<13))
    mcr     p15,0,r1,c1,c0,0

    mcr     p15,0,r0,c12,c0,0           @@ !!! physical addr (identity mapped); will need to change in kernel mmu



@@===================================================================================================================


@@
@@ -- for debugging purposes, set up the VBAR to be at address 0x00000000; this will override what is configured
@@    above.
@@    ----------------------------------------------------------------------------------------------------------


@@===================================================================================================================



@@
@@ -- Now we want to set up paging for initialization -- we are only concerned with identity mapping the first
@@    4MB of memory and this table needs to be 16K aligned.  This is why we allocated it first -- we can
@@    guarantee that this allocation will be 16K aligned.
@@
@@    IMPORTANT NOTE HERE: I am not going to fully initialize this structure -- just enough for the first 4MB
@@    to work properly.  Any data access outside this first 4MB will result in an exception -- or maybe not --
@@    it will be the decision of the unitialized memory bit gods!
@@    --------------------------------------------------------------------------------------------------------
    ldr     r0,=mmuLvl1Count            @@ get our frame
    mov     r1,#4                       @@ calculate the size of the
    str     r1,[r0]                     @@ set the frame count

    ldr     r0,=mmuLvl1Table            @@ get the address of the frame variable
    ldr     r0,[r0]                     @@ and then get the frame address itself

@@ -- We need to map 4 X 1MB sections to the physical addressed; start by building the value we will store.
@@                             3322222222221111111111
@@                             10987654321098765432109876543210
@@                             --------------------------------
@@    This value needs to be 0b00000000000000010001110000001110 for regular normal memory
@@    This value needs to be 0b00000000000000010000110000000110 for devices (MMIO memory)
@@                             +----------+|||||+-+++|+--+|||++
@@                                   |     ||||| | | N  | ||| |
@@              The 1MB frame addr --+     ||||| | | o  | ||| +-------- Section/Supersection entry
@@              Non Secure ----------------+|||| | | t  | |||
@@              Indicates super section ----+||| | |    | |||
@@              Not Global ------------------+|| | | U  | ||+---------- B (set to 1)
@@              Sharable ---------------------+| | | s  | |+----------- C (set to 1)
@@              Access Permissions no wrt [2] -+ | | e  | +------------ Execute Never
@@              Memory Region -------------------+ | d  |
@@              Access Permissions any priv [1:0] -+    +--------------- Domain (use 0 for now)
@@
@@    all of this mess ends up being 0x00090c0e for 1MB and I need to add 0x00100000 for each MB after that for
@@    4MB total.
@@    ---------------------------------------------------------------------------------------------------------
    movw    r1,#0x1c0e                  @@ set the low bytes of the ttl1 word
    movt    r1,#0x0001                  @@ set the high bytes

    str     r1,[r0,#0x00]               @@ Set section 0 (0-1MB)

    add     r1,#0x100000                @@ move to the next 1MB frame
    str     r1,[r0,#0x04]               @@ Set section 1 (1-2MB)

    add     r1,#0x100000                @@ move to the next 1MB frame
    str     r1,[r0,#0x08]               @@ Set section 2 (2-3MB)

    add     r1,#0x100000                @@ move to the next 1MB frame
    str     r1,[r0,#0x0c]               @@ Set section 3 (3-4MB)

@@ -- this but will find the config base address (typically 0x3f000000) and create an identity mapping for it
@    mrc     p15,4,r1,c15,c0,0           @@ This gets the cp15 register 15 sub-reg 0 (Config Base Addr Reg or CBAR)
@    lsr     r3,r1,#20                   @@ shift to find the TTL1 index number
@    lsl     r3,#2                       @@ multiply by 4 for an offset from the base

@    mov     r2,#0                       @@ clear r2
@    movt    r2,#0xfff0                  @@ and set the top bits for the mask
@    and     r1,r1,r2                    @@ and mask out the 1M section this is in (typ = 0x3f0)
@    mov     r2,#0x0c02                  @@ make the entry
@    movt    r2,#0x0001                  @@ and the top half
@    orr     r1,r1,r2                    @@ make the entry we want to write

@@ -- perform the above commented code manually for a hard-coded address 0x3f000000
    movw    r1,#0x0c06                  @@ set the low bytes of the ttl1 word
    movt    r1,#0x3f01                  @@ set the high bytes

@@ -- 0x3f0 << 2 == 0xfc0; we need to convert entry indexes into byte offsets
    str     r1,[r0,#0xfc0]              @@ and set section for cbar

    add     r1,#0x100000                @@ move to the next 1MB frame
    str     r1,[r0,#0xfc4]              @@ and set section for cbar

    add     r1,#0x100000                @@ move to the next 1MB frame
    str     r1,[r0,#0xfc8]              @@ and set section for cbar

    add     r1,#0x100000                @@ move to the next 1MB frame
    str     r1,[r0,#0xfcc]              @@ and set section for cbar

    add     r1,#0x100000                @@ move to the next 1MB frame
    str     r1,[r0,#0xfd0]              @@ and set section for cbar

    add     r1,#0x100000                @@ move to the next 1MB frame
    str     r1,[r0,#0xfd4]              @@ and set section for cbar

    add     r1,#0x100000                @@ move to the next 1MB frame
    str     r1,[r0,#0xfd8]              @@ and set section for cbar

    add     r1,#0x100000                @@ move to the next 1MB frame
    str     r1,[r0,#0xfdc]              @@ and set section for cbar

    add     r1,#0x100000                @@ move to the next 1MB frame
    str     r1,[r0,#0xfe0]              @@ and set section for cbar

    add     r1,#0x100000                @@ move to the next 1MB frame
    str     r1,[r0,#0xfe4]              @@ and set section for cbar

    add     r1,#0x100000                @@ move to the next 1MB frame
    str     r1,[r0,#0xfe8]              @@ and set section for cbar

    add     r1,#0x100000                @@ move to the next 1MB frame
    str     r1,[r0,#0xfec]              @@ and set section for cbar

    add     r1,#0x100000                @@ move to the next 1MB frame
    str     r1,[r0,#0xff0]              @@ and set section for cbar

    add     r1,#0x100000                @@ move to the next 1MB frame
    str     r1,[r0,#0xff4]              @@ and set section for cbar

    add     r1,#0x100000                @@ move to the next 1MB frame
    str     r1,[r0,#0xff8]              @@ and set section for cbar

    add     r1,#0x100000                @@ move to the next 1MB frame
    str     r1,[r0,#0xffc]              @@ and set section for cbar


@@ -- now we enable caches
    mcr     p15,0,r0,c2,c0,0            @@ write the ttl1 table to the TTBR0 register
    mcr     p15,0,r0,c2,c0,1            @@ write the ttl1 table to the TTBR1 register as well; will use later

    mov     r1,#0                       @@ This is the number of bits to use to determine which table; short format
    mcr     p15,0,r1,c2,c0,2            @@ write these to the TTBCR0

    mov     r1,#0xffffffff              @@ All domains can manage all things by default
    mcr     p15,0,r1,c3,c0,0            @@ write these to the domain access register

@@ -- now we enable paging
    mrc     p15,0,r1,c1,c0,0            @@ This gets the cp15 register 1 and puts it in r0
    orr     r1,#1                       @@ set bit 0
    mcr     p15,0,r1,c1,c0,0            @@ Put the cp15 register 1 back, with the MMU enabled

.if ENABLE_BRANCH_PREDICTOR
    mcr     p15,0,r0,c7,c5,6            @@ invalidate the branch predictor (required maintenance when enabled)
.endif

.if ENABLE_CACHE
    mcr     p15,0,r0,c7,c1,0            @@ invalidate all instruction caches (required maintenance when enabled)
.endif

@@ -- finally jump to the loader initialization function
    b       LoaderMain                  @@ straight jump -- not a call


@@
@@ -- this function will simple get the next frame and decrement the value
@@    Clobbers: r0 (return value), r1, r2
@@    --------------------------------------------------------------------
NextEarlyFrame:         @@ called from C
    ldr     r1,=pmmEarlyFrame           @@ get the address of the frame
    ldr     r2,[r1]                     @@ get the next frame to allocate
    mov     r0,r2                       @@ this will be our return value
    sub     r2,#1                       @@ decrement the value
    str     r2,[r1]                     @@ save the new next frame
    mov     pc,lr                       @@ and return


@@
@@ -- jump to the kernel, resetting the stack in the process
@@    ------------------------------------------------------
JumpKernel:
    mov     sp,r1                       @@ set the stack
    mov     pc,r0                       @@ very simply set the new program counter; no fuss



@@
@@ -- Decorate the registers
@@    ----------------------
DecorateRegs:
    mov     r0,#0x0000
    movt    r0,#0x0000
    mov     r1,#0x1111
    movt    r1,#0x1111
    mov     r2,#0x2222
    movt    r2,#0x2222
    mov     r3,#0x3333
    movt    r3,#0x3333
    mov     r4,#0x4444
    movt    r4,#0x4444
    mov     r5,#0x5555
    movt    r5,#0x5555
    mov     r6,#0x6666
    movt    r6,#0x6666
    mov     r7,#0x7777
    movt    r7,#0x7777
    mov     r8,#0x8888
    movt    r8,#0x8888
    mov     r9,#0x9999
    movt    r9,#0x9999
    mov     r10,#0xaaaa
    movt    r10,#0xaaaa
    mov     r11,#0xbbbb
    movt    r11,#0xbbbb
    mov     r12,#0xcccc
    movt    r12,#0xcccc
    mov     pc,lr

