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


.include "constants.inc"


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
    .global     NextEarlyFrame
    .global     JumpKernel
    .global     earlyFrame
    .global     mmuLvl1Count                    @@ the number of frames for this level 1 mmu table
    .global     mmuLvl1Table                    @@ the frame for the level 1 mmu table
    .global     mb1Data
    .global     mb2Data
    .global     intTableAddr                    @@ IVT location


@@
@@ -- This is the multiboot header.  During the link process, it will be relocated to the beginning of the
@@    resulting ELF file.  This is required since the multiboot header needs to be in the first 4/8K (I think).
@@    ---------------------------------------------------------------------------------------------------------
    .section    .mboot,"ax"


@@
@@ -- This is the multiboot 1 header
@@    ------------------------------
multiboot_header:
@@ -- magic fields
    .long       MAGIC1
    .long       MBFLAGS
    .long       0-MAGIC1-MBFLAGS

@@
@@ -- The PMM frame early allocation mechanism
@@    ----------------------------------------
earlyFrame:
    .long       (4 * 1024)                      @@ start allocating at 4MB


@@
@@ -- these variables hold the physical addresses of the respective table
@@    -------------------------------------------------------------------
mmuLvl1Count:
    .long       0
mmuLvl1Table:
    .long       0
ttl2Mgmt:
    .long       0


@@
@@ -- some additional variables for use later
@@    ---------------------------------------
mb1Data:
    .long       0
mb2Data:
    .long       0

intTableAddr:
    .long       0


@@
@@ -- This is the entry point.  Multiboot will load this kernel image at 0x100000.  The first order of
@@    business is to dump us out of 'hyp' mode if we are there and to make sure we are in 'svc' mode
@@    before we go on.
@@
@@    On entry r0 holds the proper multiboot signature for MB1, r1 holds the address of the MBI,
@@    and r2 holds the value 0.
@@    ------------------------------------------------------------------------------------------------
    .section    .text.entry,"ax"
    .align      4

entry:
@@
@@ -- figure out which CPU we are on; only CPU 0 continues after this
@@
@@    Note that this code is expected to be _ALWAYS_ executing on Core0.  This check is there just in case
@@    something happens later down the road with a change/bug in `bootcode.bin`.  If this were to happen,
@@    we will emulate the expected behavior of the firmware by holding the core in a holding pen until we
@@    are ready to release it.
@@    ----------------------------------------------------------------------------------------------------
    mrc     p15,0,r3,c0,c0,5                    @@ Read Multiprocessor Affinity Register
    and     r3,r3,#0x3                          @@ Extract CPU ID bits
    cmp     r3,#0
    bne     entryApHold                         @@ if we’re not on CPU0, go to the holding pen

@@ -- Save off the MBI structure
save:
    ldr     r2,=mb1Data                         @@ get the address to put it in
    str     r1,[r2]                             @@ and save the address

    mrs     r0,cpsr                             @@ get the current program status register
    and     r0,#0x1f                            @@ and mask out the mode bits
    cmp     r0,#0x1a                            @@ are we in hyp mode?
    beq     hyp                                 @@ if we are in hyp mode, go to that section
    cpsid   iaf,#0x13                           @@ if not switch to svc mode, ensure we have a stack for the kernel; no ints
    b       cont                                @@ and then jump to set up the stack

@@ -- from here we are in hyp mode so we need to exception return to the svc mode
hyp:
    mrs     r0,cpsr                             @@ get the cpsr again
    and     r0,#~0x1f                           @@ clear the mode bits
    orr     r0,#0x013                           @@ set the mode for svc
    orr     r0,#1<<6|1<<7|1<<8                  @@ disable interrupts as well
    msr     spsr_cxsf,r0                        @@ and save that in the spsr

    ldr     r0,=cont                            @@ get the address where we continue
    msr     elr_hyp,r0                          @@ store that in the elr register

    eret                                        @@ this is an exception return

@@ -- everyone continues from here
cont:
    ldr     sp,=stack_top                       @@ set the stack

@@
@@ -- some early CPU initialization
@@    -----------------------------
    mrc     p15,0,r3,c1,c0,0                    @@ get the SCTLR
.if ENABLE_BRANCH_PREDICTOR
    orr     r3,#(1<<11)                         @@ set the Z bit for branch prediction (may be forced on by HW!)
.endif

.if ENABLE_CACHE
    orr     r3,#(1<<2)                          @@ set the data cache enabled
    orr     r3,#(1<<12)                         @@ set the instruction cache enabled
.endif
    mcr     p15,0,r3,c1,c0,0                    @@ write the SCTLR back with the caches enabled


@@ -- Clear out kernel bss -- this needs to be adjusted for the kernel physical load address
initialize:
    ldr     r4,=bssPhys
    ldr     r4,[r4]
    ldr     r9,=bssSize
    ldr     r9,[r9]
    add     r9,r9,r4

@@ -- values to store
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
@@ -- The first thing we need it the TTB1 table -- which must be 16K aligned
@@    ----------------------------------------------------------------------

@@ -- first a little housekeeping -- we need to allocate the ttb1 table (see below)
    bl      MakePageTable                       @@ call 4 times to get 4 frames!  need 16K
    ldr     r1,=mmuLvl1Table                    @@ get the location we will share the ttb1 frame
    str     r0,[r1]                             @@ save that value
    bl      MakePageTable
    bl      MakePageTable
    bl      MakePageTable


@@ -- now, we can get the stack
    bl      NextEarlyFrame                      @@ -- get a stack frame
    add     sp,r0,#STACK_SIZE                   @@ -- and set the new stack


@@ -- get a frame and initialize the Vector Table
    bl      NextEarlyFrame
    ldr     r1,=intTableAddr                    @@ get the location we will share the vbar frame
    str     r0,[r1]                             @@ save that value

@@ -- prepare the bit-coded jump instruction
    movw    r1,#0xf018                          @@ and fill in the bottom 2 bytes
    movt    r1,#0xe59f                          @@ this is the bitcode of the jump instruction

@@ -- fill in the jump instructions
    str     r1,[r0,#0x00]                       @@ These are the jump instructions (not used -- reset)
    str     r1,[r0,#0x04]                       @@ These are the jump instructions (UNDEF)
    str     r1,[r0,#0x08]                       @@ These are the jump instructions (SVC CALL)
    str     r1,[r0,#0x0c]                       @@ These are the jump instructions (PREFETCH ABORT)
    str     r1,[r0,#0x10]                       @@ These are the jump instructions (DATA ABORT)
    str     r1,[r0,#0x14]                       @@ These are the jump instructions (not used -- hyp mode)
    str     r1,[r0,#0x18]                       @@ These are the jump instructions (IRQ INT)
    str     r1,[r0,#0x1c]                       @@ These are the jump instructions (FIQ INT)

@@ -- fill in the jump targets
    ldr     r1,=ResetTarget
    str     r1,[r0,#0x20]                       @@ The target for a reset (never used but filled in anyway)

    ldr     r1,=UndefinedTarget
    str     r1,[r0,#0x24]                       @@ The target for an UNDEF

    ldr     r1,=SuperTarget
    str     r1,[r0,#0x28]                       @@ The target for SUPER

    ldr     r1,=PrefetchTarget
    str     r1,[r0,#0x2c]                       @@ The target for PREFETCH ABORT

    ldr     r1,=DataAbortTarget
    str     r1,[r0,#0x30]                       @@ The target for DATA ABORT

    mov     r1,#0
    str     r1,[r0,#0x34]                       @@ The target for not used (hyp)

    ldr     r1,=IRQTarget
    str     r1,[r0,#0x38]                       @@ The target for IRQ

    ldr     r1,=FIQTarget
    str     r1,[r0,#0x3c]                       @@ The target for FIQ

@@ -- Set up the VBAR to use an absolute address
    mrc     p15,0,r1,c1,c0,0
    and     r1,r1,#(~(1<<13))
    mcr     p15,0,r1,c1,c0,0

    mcr     p15,0,r0,c12,c0,0                   @@ !!! physical addr (identity mapped); need to chg in kernel mmu


@@===================================================================================================================


@@
@@ -- Now we want to set up paging.  The first order of business here is to map the TTL1 table for managing
@@    entries.  This requires a new page to be allocated and mapped into the TTL1 table itself.
@@
@@    The measureable results of this section are expected to be:
@@    * 0x1000000 (for 4 pages) contains the TTL1 table
@@    * create a new TTL2 table block (for managing 4 frames) exp: 0x1006000
@@    * associate a TTL1 entries:
@@        * index 0xff4 (offset 0x3fd0) to address 0x1006000
@@        * index 0xff5 (offset 0x3fd4) to address 0x1006400
@@        * index 0xff6 (offset 0x3fd8) to address 0x1006800
@@        * index 0xff7 (offset 0x3fdc) to address 0x1006c00
@@    * map pages into TTL2 table at 0x1006000:
@@        * index 0x04 (offset 0x10) to address 0x1000000
@@        * index 0x05 (offset 0x14) to address 0x1001000
@@        * index 0x06 (offset 0x18) to address 0x1002000
@@        * index 0x07 (offset 0x1c) to address 0x1003000
@@    --------------------------------------------------------------------------------------------------------
    ldr     r0,=mmuLvl1Count                    @@ get our frame
    mov     r1,#4                               @@ calculate the size of the
    str     r1,[r0]                             @@ set the frame count

@@ -- go make a new TTL2 table for the virtual TTL1 table address
    mov     r0,#(ARMV7_TTL1_TABLE_VADDR&0xffff) @@ The address of the TTL1 table in virtual memory
    movt    r0,#(ARMV7_TTL1_TABLE_VADDR>>16)
    mov     r1,r0                               @@ save this for a future call
    bl      NewTTL2Table                        @@ Go make a table for this address
    mov     r3,r0                               @@ save the TTL2 table physical address for later

@@ -- this address needs to be saved because we need it later for the TTL2 management table init later
    mov     r9,#ARMV7_MMU_DATA_PAGE             @@ get the page flags
    ldr     r2,=mmuLvl1Table                    @@ get the table address
    ldr     r2,[r2]
    orr     r2,r9                               @@ and merge those

@@ -- now, complete the mappings for the TTL1 table (4 mappings)
    mov     r8,#0                               @@ set a counter

.loop1:
    bl      MapPage                             @@ map the page

    add     r1,#4096                            @@ next page
    add     r2,#4096                            @@ next frame

    add     r8,#1                               @@ increment and check if we are done
    cmp     r8,#4
    blo     .loop1


@@
@@ -- this then completes the TTL1 table setup for the management addresses.  Now to
@@    create the TTL2 tables for managing the TTL2 tables.  In this case we will need
@@    to be able to map from address `0xffc00000` on up.  This is 4MB of memory and
@@    will only need 4 tables (1K each), or 1 frame.
@@
@@    Measureable results from this section are:
@@    * create a new TTL2 table block (for managing 4 frames) exp: 0x1007000
@@    * associate a TTL1 entries:
@@        * index 0xffc (offset 0x3ff0) to address 0x1007000
@@        * index 0xffd (offset 0x3ff4) to address 0x1007400
@@        * index 0xffe (offset 0x3ff8) to address 0x1007800
@@        * index 0xfff (offset 0x3ffc) to address 0x1007c00
@@    --------------------------------------------------------------------------------
    mov     r0,#(ARMV7_TTL2_TABLE_VADDR&0xffff) @@ load the address of the TTL2 tables
    movt    r0,#(ARMV7_TTL2_TABLE_VADDR>>16)
    mov     r1,r0                               @@ save this register for a later call
    bl      NewTTL2Table                        @@ make a new table

    ldr     r2,=ttl2Mgmt                        @@ save the table for later maintenance
    str     r0,[r2]


@@
@@ -- Next, the TTL2 table for addresses 0xffc00000 need to be mapped into this very same
@@    physical address at 0x1007000.  This is a single 4K page so there will be a single
@@    TTL2 entry that gets updated.  The address to map is 0xffc00000 and the index to map
@@    is 0x3ff (offset 0xffc) in the physical address.
@@
@@    Measureable results here:
@@    * Address 0x1007ffc points to frame 0x1007000
@@    ---------------------------------------------------------------------------------------
    mov     r2,r0                               @@ make the table address and the frame address match
    mov     r9,#ARMV7_MMU_DATA_PAGE             @@ get the page flags
    orr     r2,r9                               @@ and add them to the address
    mov     r1,#(ARMV7_TTL2_TABLE_VADDR&0xffff)
    movt    r1,#(ARMV7_TTL2_TABLE_VADDR>>16)
    bl      MapTtl2Mgmt


@@
@@ -- Finally, we need to do the same thing as above, but for the TTL1 management entries
@@    starting at address 0xff404000.  There are 4 X 4K pages.  First, we need a TTL2 table
@@    for this address (0xff404000 for 4 pages in total).  We have this saved in r3 so far.
@@    So, it is just a matter of mapping that into the Management space
@@
@@    Measureable results here:
@@    * Address 0x1007ff4 points to frame 0x1006000
@@    -------------------------------------------------------------------------------------
    mov     r2,r3                               @@ get the TTL2 table for TTL1 managment
    mov     r9,#ARMV7_MMU_DATA_PAGE             @@ get the page flags
    orr     r2,r9                               @@ and add them to the address
    mov     r1,#(ARMV7_TTL1_TABLE_VADDR&0xffff)
    movt    r1,#(ARMV7_TTL1_TABLE_VADDR>>16)
    bl      MapTtl2Mgmt


@@
@@ -- At this point we can use a full-featured function to complete the mappings.
@@    ... and we have a lot of them to do.
@@
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
@@    Identity map the mboot section at 1MB
@@
@@    Measureable results:
@@    * 0x1000000 maps to TTL2 table (expect 0x1008000)
@@    * 0x1000004 maps to TTL2 table 0x1008400
@@    * 0x1000008 maps to TTL2 table 0x1008800
@@    * 0x100000c maps to TTL2 table 0x1008c00
@@    * 0x1008400 maps to frame 0x100000
@@    * 0x1008404 maps to frame 0x101000
@@    ---------------------------------------------------------------------------------------------------------
    ldr     r0,=mbPhys                          @@ get the mboot physical address
    ldr     r0,[r0]

    ldr     r3,=mbSize                          @@ get the size of the mboot
    ldr     r3,[r3]
    lsr     r3,#12                              @@ number of pages to write

    mov     r2,#ARMV7_MMU_DATA_PAGE             @@ load the flags

    mov     r8,#0                               @@ start the counter

@@ -- perform the mapping
.loop2:
    mov     r1,r0                               @@ identity map
    bl      MapPageFull                         @@ map the page
    add     r8,#1
    add     r0,#4096

    cmp     r8,r3                               @@ are we done
    blo     .loop2


@@
@@ -- now map the loader code/data section (0x80000000)
@@    -------------------------------------------------
    ldr     r0,=ldrVirt                         @@ get the virtual address of the loader
    ldr     r0,[r0]

    ldr     r1,=ldrPhys                         @@ get the physical address of the loader
    ldr     r1,[r1]

    ldr     r3,=ldrSize                         @@ get the size of the loader
    ldr     r3,[r3]
    lsr     r3,#12

    mov     r2,#ARMV7_MMU_DATA_PAGE             @@ load the flags

    mov     r8,#0                               @@ start a counter

.loop3:
    bl      MapPageFull                         @@ map the page
    add     r8,#1
    add     r0,#4096
    add     r1,#4096

    cmp     r8,r3                               @@ are we done?
    blo     .loop3


@@
@@ -- now map the pergatory code/data section (0x80400000)
@@    ----------------------------------------------------
    ldr     r0,=sysVirt                         @@ get the virtual address of the syscall section
    ldr     r0,[r0]

    ldr     r1,=sysPhys                         @@ get the physical address of the syscall section
    ldr     r1,[r1]

    ldr     r3,=sysSize                         @@ get the size of the syscall section
    ldr     r3,[r3]
    lsr     r3,#12

    mov     r2,#ARMV7_MMU_DATA_PAGE             @@ load the flags

    mov     r8,#0                               @@ start a counter

.loop4:
    bl      MapPageFull                         @@ map the page
    add     r8,#1
    add     r0,#4096
    add     r1,#4096

    cmp     r8,r3                               @@ are we done?
    blo     .loop4


@@
@@ -- now map the kernel code section (0x80800000)
@@    --------------------------------------------
    ldr     r0,=txtVirt                         @@ get the virtual address of the kernel code
    ldr     r0,[r0]

    ldr     r1,=txtPhys                         @@ get the physical address of the kernel code
    ldr     r1,[r1]

    ldr     r3,=txtSize                         @@ get the size of the kernel code
    ldr     r3,[r3]
    lsr     r3,#12

    mov     r2,#ARMV7_MMU_CODE_PAGE             @@ load the flags

    mov     r8,#0                               @@ start a counter

.loop5:
    bl      MapPageFull                         @@ map the page
    add     r8,#1
    add     r0,#4096
    add     r1,#4096

    cmp     r8,r3                               @@ are we done?
    blo     .loop5


@@
@@ -- now map the kernel data section (0x81000000)
@@    --------------------------------------------
    ldr     r0,=dataVirt                        @@ get the virtual address of the kernel data
    ldr     r0,[r0]

    ldr     r1,=dataPhys                        @@ get the physical address of the kernel data
    ldr     r1,[r1]

    ldr     r3,=dataSize                        @@ get the size of the kernel data
    ldr     r3,[r3]
    lsr     r3,#12

    mov     r2,#ARMV7_MMU_DATA_PAGE             @@ load the flags

    mov     r8,#0                               @@ start a counter

.loop6:
    bl      MapPageFull                         @@ map the page
    add     r8,#1
    add     r0,#4096
    add     r1,#4096

    cmp     r8,r3                               @@ are we done?
    blo     .loop6


@@
@@ -- map the stack
@@    -------------
    mov     r0,#(STACK_BASE&0xffff)             @@ stack location
    movt    r0,#(STACK_BASE>>16)                @@ stack location
    mov     r8,r0                               @@ save the stack, we wll replace it in a bit
    add     r8,#STACK_SIZE                      @@ and calculate the proper top of stack
    mov     r1,sp                               @@ current stack
    mov     r9,#0xf000
    movt    r9,#0xffff
    and     r1,r9                               @@ mask out the the base physical address
    mov     r2,#ARMV7_MMU_DATA_PAGE
    bl      MapPageFull

@@ -- now we enable caches
    ldr     r0,=mmuLvl1Table
    ldr     r0,[r0]

    mcr     p15,0,r0,c2,c0,0                    @@ write the ttl1 table to the TTBR0 register
    mcr     p15,0,r0,c2,c0,1                    @@ write the ttl1 table to the TTBR1 register; will use later

    mov     r1,#0                               @@ The number of bits to use to determine which table; short format
    mcr     p15,0,r1,c2,c0,2                    @@ write these to the TTBCR0

    mov     r1,#0xffffffff                      @@ All domains can manage all things by default
    mcr     p15,0,r1,c3,c0,0                    @@ write these to the domain access register

@@ -- now we enable paging
    mrc     p15,0,r1,c1,c0,0                    @@ This gets the cp15 register 1 and puts it in r0
    orr     r1,#1                               @@ set bit 0
    mcr     p15,0,r1,c1,c0,0                    @@ Put the cp15 register 1 back, with the MMU enabled

.if ENABLE_BRANCH_PREDICTOR
    mcr     p15,0,r0,c7,c5,6                    @@ invalidate the branch predictor (required maintenance when enabled)
.endif

.if ENABLE_CACHE
    mcr     p15,0,r0,c7,c1,0                    @@ invalidate all instruction caches (required maintenance when enabled)
.endif


@@ -- finally jump to the loader initialization function
    mov     sp,r8                               @@ replace the stack
    b       LoaderMain                          @@ straight jump -- not a call


@@
@@ -- Make a paging table -- clear out to be sure it's blank
@@    ------------------------------------------------------
MakePageTable:
    push    {r1-r9,lr}
    bl      NextEarlyFrame                      @@ get a frame

    mov     r4,r0                               @@ this is the address to start clearing
    add     r9,r4,#4096                         @@ get the end of the block

    mov     r5,#0
    mov     r6,#0
    mov     r7,#0
    mov     r8,#0

.ptloop:
    stmia   r4!,{r5-r8}                         @@ clear the memory and increment r4

    cmp     r4,r9                               @@ are we done?
    blo     .ptloop                             @@ loop if not

    pop     {r1-r9,pc}


@@
@@ -- This function will make a new TTL2 table for the address given in r0.
@@    This function operates only on the TTL1 table physical address.  It is assumed
@@    that the TTL2 table does not exist and if it does will be overwritten.
@@    The management tables are not maintained by this function.
@@    ------------------------------------------------------------------------------
NewTTL2Table:
    push    {r1-r9,lr}                          @@ save a whole bunch of registers

    mov     r5,r0                               @@ our address to map
    bl      KrnTtl1Entry4                       @@ and get the address of the "mod 4" entry in r5
    bl      MakePageTable                       @@ r0 will hold the physical address of the new table

@@ -- finally, we need the value to load into the TTL1 table entry
    mov     r9,#ARMV7_MMU_TTL1_ENTRY
    orr     r4,r0,r9                            @@ r4 holds the value value to load

    mov     r8,#0                               @@ this is the number of entries we loaded

@@ -- we can now load the entries
.ttl2loop:
    str     r4,[r5]

    add     r4,#0x400                           @@ move to the next TTL2 table of 4
    add     r5,#4                               @@ move to the next TTL1 entry

    add     r8,#1                               @@ one more done
    cmp     r8,#4                               @@ are we done?
    blo     .ttl2loop                           @@ loop

@@ -- all done; exit
    pop     {r1-r9,pc}                          @@ clean up our mess


@@
@@ -- This function will complete a page mapping in the physical table address.  It operates on a
@@    4K aligned 4 X 1K group of TTL tables in physcial memory (not in virtual management space).
@@    r0 -- the address of the TTL2 table in physcal memory
@@    r1 -- the address to map (will be converted into an index to the TTL2 table)
@@    r2 -- the address of the frame to complete the mapping OR'd with the flags (the value to store in the table)
@@    r0 to r9 are all preserved.
@@    ------------------------------------------------------------------------------------------------------------
MapPage:
    push    {r0-r2,r7,r9,lr}

    mov     r7,r1                               @@ set the register for the call
    bl      KrnTtl2EntryOffset                  @@ go get the offset into the r0 table

    add     r0,r7                               @@ get the proper physicasl address
    str     r2,[r0]                             @@ complete the mapping

    pop     {r0-r2,r7,r9,pc}

@@
@@ -- This fucntion will perform a full mappng -- nap the page and if neessary make a new TTL2 table
@@    with the necessary mgmt entry to go along with it.  This funtion is only available once the management
@@    tables have been set up.  This function will read those tables to determine how to complete the
@@    mappings.  This differs from the OS functions in that this reads physical addresses whereas the OS
@@    will use the manaement tables to perfoem the same function.
@@    r0 -- the address to map -- this does not need to be a "clean" address
@@    r1 -- the frame to which to map the page (will also be cleaned up)
@@    r2 -- the flags that will be used to decorate the page in the tables
@@    returns a cleaned up virtual address
@@    -------------------------------------------------------------------------------------------------------
MapPageFull:
    push    {r1-r9,lr}

    mov     r9,#0xf000                          @@ establish the cleanup mask
    movt    r9,#0xffff                          @@ establish the cleanup mask
    and     r1,r9                               @@ clean frame
    and     r0,r9                               @@ clean page
    mov     r9,#0xfff
    and     r2,r9                               @@ clean bits
    orr     r1,r2                               @@ r1 holds the complete ttl2 entry
    push    {r0}                                @@ save the return address


@@
@@ -- now, r0 has the address to map; r1 has the frame with its bits; r2-r9 are scratch regs
@@    the first order of business is to determine if we need a new TTL2 table
@@    ---------------------------------------------------------------------------------------
    mov     r5,r0
    bl      KrnTtl1Entry4                       @@ get the address of the TTL1 entry (r5)

    ldr     r3,[r5]                             @@ r3 has the ttl1 entry
    mov     r9,r3                               @@ we need a copy
    and     r9,#0x03                            @@ get the fault bits
    cmp     r9,#(ARMV7_MMU_TTL2)                @@ is there a ttl2 table

    beq     .haveTtl2                           @@ if we have one, no need to make a new one

@@ -- we need to make a new ttl2 table
    push    {r0}                                @@ save our work
    bl      NewTTL2Table                        @@ go get a new table
    mov     r3,r0                               @@ put that entry in the right register
    mov     r4,r0                               @@ we need to save te physical address as well
    pop     {r0}                                @@ restore the saved work

    mov     r9,#(ARMV7_MMU_TTL2)                @@ get the bits to set
    orr     r3,r9                               @@ make the proper ttl1 entry
    str     r3,[r2]                             @@ and put it in place

@@ -- map the ttl2 management entry
    push    {r0-r2}                             @@ save our work again
    mov     r1,r3                               @@ get the address to map
    mov     r2,r4                               @@ get the physical frame
    bl      MapTtl2Mgmt                         @@ take care of manaement mapping
    pop     {r0-r2}                             @@ restore our work

@@ -- we now have a ttl2 address; r3 has the address of the TTL2 table
.haveTtl2:
    mov     r9,#0xf000                          @@ establish the cleanup mask
    movt    r9,#0xffff                          @@ establish the cleanup mask
    and     r4,r3,r9                            @@ r4 now holds the physical ttl2 frame address

@@ -- now the ttl2 entry address
    mov     r7,r0                               @@ the address to map
    bl      KrnTtl2EntryOffset                  @@ get the offset into the physical table

    add     r5,r7,r4                            @@ r5 now holds the address of the ttl2 entry

@@ -- complete the mapping
    str     r1,[r5]                             @@ after all that, this is it

    pop     {r0}                                @@ restore the return address
    pop     {r1-r9,pc}




@@
@@ -- Map a TTL2 frame in Management space.  This function will map a 4K page into the
@@    proper location in the TTL2 table management space (i.e. from 0xffc00000 on).  For this,
@@    we need the top 12 bits of the address, which will put us to an index.   That will then
@@    need to be converted to an offset.
@@    r1 -- an address anywhere in the TTL2 table (will be cleaned up)
@@    r2 -- the physical address of the TTL2 table as a frame of the page (bits will be added)
@@    returns nothing of value
@@    -----------------------------------------------------------------------------------------
MapTtl2Mgmt:
    push    {r2,r6,r9,lr}

    mov     r6,r1                               @@ get the address to map
    bl      KrnTtl2Mgmt                         @@ r6 will not contain the address

    mov     r9,#(ARMV7_MMU_DATA_PAGE&0xffff)    @@ get the flags we want to set
    movt    r9,#(ARMV7_MMU_DATA_PAGE>>16)
    orr     r2,r9                               @@ r2 holds the value value to load with the extra bits

    str     r2,[r6]                             @@ complete the mapping

    pop     {r2,r6,r9,pc}


@@
@@ -- This function will calculate the Kernel TTL1 Entry address for a given address in r5.
@@    The proper TTL1 Entry address will be returned in r5.  No other registers will be
@@    changed.
@@    -------------------------------------------------------------------------------------
KrnTtl1Entry:
    push    {r0, lr}

    lsr     r5,#20                              @@ get the index of the entry
    lsl     r5,#2                               @@ convert that to an offset

    ldr     r0,=mmuLvl1Table                    @@ load the base address of the table
    ldr     r0,[r0]

    add     r5,r0                               @@ now, calculate the address

    pop     {r0, pc}


@@
@@ -- This function will calculate the Kernel TTL1 Entry address for the first of a block
@@    of 4 TTL2 tables.
@@    -----------------------------------------------------------------------------------
KrnTtl1Entry4:
    push    {r9,lr}

    bl      KrnTtl1Entry
    mov     r9,#0xfff0
    movt    r9,#0xffff
    and     r5,r9

    pop     {r9,pc}


@@
@@ -- This function will calculate the Management address for a TTL2 table for an address
@@    provided in r6.  The address will be returned in r6.  All other registers are
@@    unchanged.
@@    -----------------------------------------------------------------------------------
KrnTtl2Mgmt:
    push    {r0, lr}

    lsr     r6,#22                              @@ get the index of the entry
    lsl     r6,#2                               @@ convert that to an offset

    ldr     r0,=ttl2Mgmt                        @@ load the base address of the table
    ldr     r0,[r0]

    add     r6,r0                               @@ now, calculate the address

    pop     {r0, pc}


@@
@@ -- This function will calculate the offset into a physical frame for a TTL2 entry.
@@    The address to calculate is passed in r7 and the offset is returned in the same
@@    register.  No other registers are changed.  Unlike the 3 above functions, this
@@    merely returns an address offset, not the final address itself.
@@    -------------------------------------------------------------------------------
KrnTtl2EntryOffset:
    push    {r0,lr}

    lsr     r7,#12                              @@ get the index
    mov     r0,#0x3ff                           @@ mask out the top bits
    and     r7,r0
    lsl     r7,#2                               @@ make that an offset


    pop     {r0,pc}                             @@ return



@@
@@ -- this function will simple get the next frame and  increment the value
@@    Clobbers: r0 (return value), r1, r2
@@    --------------------------------------------------------------------
NextEarlyFrame:                                 @@ also called from C
    ldr     r1,=earlyFrame                      @@ get the address of the frame
    ldr     r2,[r1]                             @@ get the next frame to allocate
    mov     r0,r2                               @@ this will be our return value
    add     r2,#1                               @@ increment the value
    str     r2,[r1]                             @@ save the new next frame
    lsl     r0,#12                              @@ turn this into an address
    mov     pc,lr                               @@ and return


@@
@@ -- jump to the kernel, resetting the stack in the process
@@    ------------------------------------------------------
JumpKernel:
    mov     sp,r1                               @@ set the stack
    mov     pc,r0                               @@ very simply set the new program counter; no fuss


@@
@@ -- we need a small stack for out first calls to get a stack
@@    --------------------------------------------------------
    .align      4
stack:
    .long       0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
stack_top:
