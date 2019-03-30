# The Century OS Development Journal - v0.3.2

## Version 0.3.2

This version will improve the scheduler and process swapping. While things are working at the moment, they are nowhere near correct. Even before getting into this tutorial, I had fears that I had created a monster. So this version will clean all that up.

Using Brandan's Multi-tasking Tutorial (https://wiki.osdev.org/Brendan%27s_Multi-tasking_Tutorial) as a roadmap, this version will complete steps 1-14 and step 16. Step 15 (IPC) will be left for a later release (even though I had that working poorly at one point).

The goals for this version are tracked here: http://eryjus.ddns.net:3000/versions/9

---

### 2019-Mar-14

Happy Pi Day!!

I start this version with the realization that I will be taking a few steps backward in order to take a massive leap forward.  So, with that, the first thing to review is the `Process_t` structure:

```C
typedef struct Process_t {
    archsize_t stackPointer;            // This is the process current esp value (when not executing)
    archsize_t ss;                      // This is the process ss value
    archsize_t pageTables;              // This is the process cr3 value
    PID_t pid;                          // This is the PID of this process
    archsize_t ssAddr;                  // This is the address of the process stack
    size_t ssLength;                    // This is the length of the process stack
    char command[MAX_CMD_LEN];          // The identifying command, includes the terminating null
    size_t totalQuantum;                // This is the total quantum used by this process
    ProcStatus_t status;                // This is the process status
    ProcPriority_t priority;            // This is the process priority
    int quantumLeft;                    // This is the quantum remaining for the process (may be more than priority)
    bool isHeld;                        // Is this process held and on the Held list?
    struct Spinlock_t lock;             // This is the lock needed to read/change values
    ListHead_t::List_t stsQueue;        // This is the location on the current status queue
    ListHead_t lockList;                // This is the list of currently held locks (for the butler to release)
    ListHead_t messages;                // This is the queue of messages for this process
    void *prevPayload;                  // This is the previous payload in case the process did not allocate enough
} Process_t;
```

This structure look very close to what I will end up with.  There are going to need to be a few tweaks to names, but those should be minor.

The first big thought here is about the `ss` field: Do I really need it?  Well, for arm, there is no such thing since the memory is flat.  However, with the x86 architectures, I will need to keep track of the user-space stack section (well, or do I?).  The assumption here is that all task changes will happen in kernel space, not from user space.  Therefore, the `ss` field should be able to go away -- any change back to user space will be stored on the stack.

The next 2 things I am going to do is rename `stackPointer` to be `topOfStack` and `pageTables` to be `virtAddrSpace`.  These terms more accurately describe what they are used for across the architectures.

`pid` I am torn over.  I am going to keep it for now as there may be things that want to find any process by its pid.

`ssAddr` is the allocated base of the stack.  While the tutorial does some dodgy calculation to free the stack, I prefer to store this in the `Process_t` structure.  However, `ssLength` can go away.  I will use a constant value for all stack sizes.  While I am at it, I will also need to replace the TSS `esp0` field on a task change as well for x86.  I will need to read this again in the Intel Manuals about what that is used for.  In the meantime, I will leave this off.

`command` will be changed from an array to a pointer, and I will allocate the memory from the heap -- since I have a working heap much earlier than I thought I would.

`totalQuantum` will be removed; I will add in something more appropriate when I get to step 2.

`status` and `priority` will remain as-is, but I will also be adding a `policy` field.  This field will then need to be backed by an enumeration.

`quantumLeft` will not us used for not and will be removed.  `isHeld` is no longer needed. `lock` is no longer needed.

`stsQueue` is actually going to be used as the `next` member as noted in the tutorial.

`lockList`, `messages`, and `prevPayload` are removed.

---

With `Process_t` cleaned up and the header file `process.h` cleaned of the function prototypes, I have also commented out and renamed files so that I can get the binary to compile.

So, I move on to rebuilding the `ProcessInit()` function.  This function must populate the `Process_t` structure for the current running task -- the kernel initialization process.  I am going to work on the arm version first since it will be more foreign to me.

---

### 2019-Mar-15

I am trying to get the `ProcessInit()` function written today....

---

### 2019-Mar-16

I was able to get the `ProcessInit()` function to be architecture independent.  I did not think I would be able to do that.

The next step is to rewrite the `ProcessSwitch()` function to align with my structure.  For now, I am still ignoring the TSS.  Again, I will take on the arm architecture first.

---

Working on `ProcessCreate()`, there are a couple of things that are arch-dependent.  I am also working on getting those addressed so I can test this function.

---

So, I have come across a challenge related to the stacks.  A new process needs a new stack.  However, the new process stack cannot be in kernel space.  This would allow the stack to be accessible between processes.  Therefore, the stack really needs to be in the user address space in order to protect it.

For the initial version, I think I am going to use stacks in kernel space to make it easier to test incrementally.  To do this, I will be using the `pid` field to create an offset to the next stack in kernel space.

I was able to get to a point where I could execute a basic test.  Page Fault:

```
Page Fault
EAX: 0x80002b90  EBX: 0x00000400  ECX: 0x00000001
EDX: 0x3ffff40a  ESI: 0x90000080  EDI: 0x003fd800
EBP: 0x00000000  ESP: 0xff800f7c  SS: 0x10
EIP: 0x80002912  EFLAGS: 0x00200296
CS: 0x8  DS: 0x10  ES: 0x10  FS: 0x10  GS: 0x10
CR0: 0x80000011  CR2: 0xff40b000  CR3: 0x003fe000
Trap: 0xe  Error: 0x2
```

Interestingly enough, the fault came from `ProcessNewStack()`.  I will try to sort that tomorrow.

---

### 2019-Mar-17

So that problem was a post-decrement problem where I should have been pre-decrementing the stack before adding values to the stack.

With that fixed, it is now triple faulting.

---

x86-pc is working now.  I'm trying to get arm to work.

---

### 2019-Mar-18

OK, I have this now!  The problem is that I am treating the arm architecture as if it is the x86 architecture.  In reality, the return address is not placed on the stack for arm.  If the function wants to preserve this return address, it is responsible for placing the register on the stack and preserving it itself.  So, for arm, to duplicate the call, I will need to simulate this call by populating `lr` with the actual starting address.

After building the stack in a way I can restore things in the proper order, I am finally successful!

Now, while I am not done with step 1, it is time to commit this code.

---

There are 3 things I left outstanding:
1. The stack really needs to be relocated to user space.  I am not yet in user space scheduling, so I will leave this for later.
1. There needs to be a stack for kernel calls.  Again, I am not yet in user space, so I will leave this for later.
1. I should have a top level paging table for each process.  Currently, I am sharing the kernel tables.  As I am not yet back un user space, I will save this for later.

So, this then moves me along to Step 2 in the tutorial.  Step 2 is to implement a trivial round-robin scheduler.  This is still a cooperative multi-tasker.

If I recall the tutorial correctly, a good majority of this code is removed in later steps.  I'm OK with that.

---

### 2019-Mar-19

I was able to get the Round Robin scheduler working yesterday at the end of the day.  This was with 2 processes.  I want to test with 3 or more processes today.  A simple test that worked well.

I am going to commit with step 2.

---

I move on to Step 3 for accounting for CPU time.  This is where the 2 architectures depart from each other.  For rpi2b, I have a 64-bit counter that I can measure for each task change.  For x86, however, there is a only a 16-bit counter.  This is not nearly large enough to support my needs.  So, I will be constructing a counter that increments on each timer tick.  This will allow me to measure the number of ticks since boot.  This will work until I get the APIC implemented and have finer control over the timer for multiple cores.

No matter what, I will have to update the `TimerDevice_t` structure so that I have all the support I need.

---

### 2019-Mar-21

I was able to get some rudimentary time accounting working.  It is not perfect and I really need to switch out the PIT for the APIC.  There are also differences in emulation speed versus the real rpi2b hardware.  At some point down the road I will be working on real hardware for both and I will work to clean that up then.

In the meantime, I start my day with a commit.

---

So, step 4 in the tutorial is a bit of a cleanup of the round robin scheduler.  This change is done to manage the condition of the `Process_t` status.  For me, this will mean a change in the `ProcStatus_t` enumeration.  This change also has be pulling the status field up in the `Process_t` structure so that it can be more easily calculated from the assembly code.

---

### 2019-Mar-22

The new round robin scheduler is working well, so I will commit.

---

Step 5 is to add some basic locking to the scheduler.  While I already have a spinlock structure, I am going to duplicate the algorithms from the tutorial for now.  I know that the tutorial will replace this later and at that point I can make it more robust for me.

---

This was a simple change and is ready to commit.

---

Step 6 in the tutorial is to create the ability to block and unblock tasks.  This should also be relatively simple to build.

And, after a quick cleanup of a condition check in `ProcessSchedule()`, everything still works.  Committing again.

---

So, with Step 7, I am now forced to increase the accuracy of the timer counter.  This means I need to double check the calculations for the timer frequency.

After a quick review, both architectures are calculating the frequency dynamically.  So, I am going to go with the concept that the emulator is not emulating the timing properly.  This means that I am programming the timer to fire every 1ms.  1 nano is 1000000ms.  Therefore, I need to update the timer properly to represent nanos.  This means revisiting step 3 to change everything to nanos.

Now, is nanos too granular?  I think for my purposes it will be.  I think I will be able to get away with micros (or 1000000th of a second).  I am going to go that route.

---

So I have a problem with some of the math parts for arm.  I do not have the necessary library to do soft division and while the cross compiler is for hardware floating point arithmetic, it is still trying to use software floating point.  I will have to figure this out....

---

### 2019-Mar-23

I raised my question to the group on `freenode#osdev`:

```
[10:19] <eryjus> I have finally gotten to a point where I need to do some division.  the x86 cross compiler handles this of course, but the armv7 cross is issuing an undefined reference to __eabi_uldivmod.  I have the cross configured (I thought anyway) for hard float support.  any suggestions?
[10:19] <w1d3m0d3> isnt that a part of libgcc?
[10:19] <geist> eryjus: depends on which kind of division you're doing
[10:19] <w1d3m0d3> (don't quote me on that - i'm very rusty due to a lack of work)
[10:20] <geist> what core are you compiling for?
[10:20] <geist> not all of them have div instruction, so it may be defaulting to no div
[10:20] <eryjus> uint32 / uint64 / uint32
[10:20] <geist> no which arm core
[10:20] <geist> oh i see you were answering my previous
[10:21] <eryjus> the cpu is cortex-a7 for rpi2b
[10:21] <geist> so do you specify any compile flags that call this out?
[10:22] <eryjus> ahhh..  so udiv only has 32-bit division support
[10:22] <eryjus> stand by on the compile flags
[10:22] <eryjus> CFLAGS += -mlittle-endian CFLAGS += -mcpu=cortex-a7 CFLAGS += -mfpu=neon-vfpv4 CFLAGS += -mfloat-abi=hard
[10:23] <geist> excellent. yes it's the 64bit div then
[10:23] <geist> usually that results in it calling into libgcc
[10:23] <geist> so now you have a much bigger problem, which is do you have a libgcc multilib that satisfies this
[10:23] <geist> it's super annoying
[10:23] <eryjus> and I think the answer will be probably not.
[10:24] <geist> a) are you linking a libgcc.a?
[10:24] <eryjus> no
[10:24] <geist> that's probably your problem
[10:24] <eryjus> attempted and failed -- but I did not pursue it well enough to properly give up on it
[10:25] <geist> that's where it's defined. i'm looking at a disassembly of a libgcc.a and i can confirm its there
[10:25] <geist> generally speaking if you're linking everything manually you can find the appropriate libgcc with something like
[10:26] <geist> <path to your arm gcc> <CFLAGS you're using> --print-libgcc-file-name
[10:26] <geist> it'll do the multilib search and give you the approprate libgcc that you can save and then add tothe link line
[10:26] <geist> if that doesn't seem to work right, Dont Panic! it may be that you need to rebuild your toolchain with the appropriate hard float libgccs
[10:28] <eryjus> ok, well my libgcc.a has the function...  I will work on getting it to link in properly.
[10:28] <eryjus> thanks for the help
[10:29] <geist> yep. keep in mind it still may have linkage problems. the trouble is to support all the varying amounts of ways to compile arm binaries there usually has to be multiple libgccs compiled
[10:29] <geist> and depending on where you got your cross compiler, it may not have a specific libgcc that is hard float and whatnot
[10:29] <geist> so may still require additional slething
[10:29] <geist> also suggestion: dont do 64bit divs. as you can see it's going to call a software fallback routine
[10:30] <geist> i've seen that easily spend hundreds of cycles
[10:30] <geist> try to avoid 64bit divs and mods like the plague
[10:34] <eryjus> geist, good advice.  thanks again
[10:35] <geist> this is one of the other reasons i've been somewhat happier with 64bit arm since i started working with it. a lot of these old annoying toolchains and floating point and div and whatnot things go away
[10:35] <geist> since they become standard on the new stuff
[10:35] <eryjus> yeah, this will be called with every task switch -- I'm off to find a better way...
[10:37] <geist> yah, if it's a time calculation those are hard. what i ended up doing in LK is building a fixed point math routine to convert ticks to time
[10:37] <geist> the fixed point routine only uses multiplies but is based on the idea that you do all the complex calculation up front
[10:37] <geist> https://github.com/littlekernel/lk/blob/master/lib/fixed_point/include/lib/fixed_point.h if you're interested
[10:39] <geist> it looks pretty complicated, but all that code tends to flatten out to a handful of multiplies and there are no branches
[10:39] <geist> arm cores can multiply like nobodies business
[10:46] <eryjus> thx, I'll take a look
```

I was able to get the binary to link properly.  However, in execution, I have the following `undefined` abort:

```
Undefined Instruction:
At address: 0xff800f80
 R0: 0x80057120   R1: 0x8005822c   R2: 0x0000710c
 R3: 0x90000040   R4: 0x00101a38   R5: 0x003fc000
 R6: 0x8005c63c   R7: 0x80005d14   R8: 0x80005c88
 R9: 0x800059b4  R10: 0x80057108  R11: 0x97a0abfe
R12: 0x00007124   SP: 0x80000c60   LR_ret: 0x8000269c
SPSR_ret: 0x600001d3     type: 0x1b

Additional Data Points:
User LR: 0xefefcdfe  User SP: 0xadffecf9
Svc LR: 0xff800f90
```

The instruction before the return point is:

```
80002698:       f2c00010        vmov.i32        d16, #0 ; 0x00000000
```

The ARM-ARM states the following about this instruction:

> Depending on settings in the CPACR, NSACR, HCPTR, and FPEXC registers, and the security state and mode in which the instruction is executed, an attempt to execute the instruction might be UNDEFINED, or trapped to Hyp mode. Summary of general controls of CP10 and CP11 functionality on page B1-1231 and Summary of access controls for Advanced SIMD functionality on page B1-1233 summarize these controls.

I know for a fact, I am not yet doing anything with the registers CPACR, NASCR, HCPTR or FPEXC registers.  So, I will need to look there for solutions.

---

### 2019-Mar-24

I found a table in the ARM-ARM that has the settings I need to enable the gloating point operations.  This is `Table B1-21` and this table illustrates that for all modes to be able to execute floating point instructions, the setting need to be:
* CPACR.cp*n* = 0b11
* NSACR.cp*n* = 1
* HCPTR.TCP*n* = 0
* FPEXC.EN = 1

I need to make sure that that control registers cp10 and cp11 are accessible.

So, the first task will be to program the `CPACR.cp10` and `CPACR.cp11` settings to be `0b11` so that they are available from PL0.

Now, where to do this?  I need this all set up prior to the `TimerInit()` function call.  I believe that I should be able to take care of this during the `EarlyInit()` function.

---

So, the code that finally worked is:

```C
    //
    // -- prepare the FPU for accepting commands
    //    --------------------------------------
    archsize_t cpacr = READ_CPACR();
    kprintf("The initial value of CPACR is %p\n", cpacr);
    cpacr |= (0b11<<20);
    cpacr |= (0b11<<22);
    kprintf("Writing CPACR back as %p\n", cpacr);
    WRITE_CPACR(cpacr);

    //
    // -- and enable the fpu
    //    ------------------
    WRITE_FPEXC(1<<30);
```

NACR already had the bits set I needed, so there is no need to write those back; HCPTR is only accessible from PL2.

---

With that done, I can move on to the `sleep()` and related functions.  Well, I was able to implement `ProcessSleep()`, `ProcessMilliSleep()`, and `ProcessMicroSleep()`.  However, `ProcessMicroSleepUntil()` is not yet implemented and we are not ready to do this per the tutorial.

So, I will commit after step 7.

---

Step 8 is to improve locks.  One of the problems I had not really considered was that the CPU really should not change tasks when a lock is held.  In the past I wanted to let other things get a turn at the CPU, but when something else wanted the lock it would donate the CPU time to the lock holder.  Step 8 of this tutorial takes the approach to save the overhead of the task change when a lock is held.

For now, I am going to go down this path with the tutorial.  I may change back to my original thinking later, once the kernel matures a bit more.

With that, this means that I need to update the `Spinlock_t` functions to keep track of lock counts.  For this, I will change a couple of the inline functions into proper functions in their own files.

A successful test means I can commit this code.

---

So, step 9 will require a bit of coding to create a data elements for waiting tasks.  Let's see here:
* I need a wake at micros element in `Process_t`.
* I need a list to keep the waiting tasks in.
* I want a variable holding the next micro since boot to wake something up.

---

### 2019-Mar-25

Debugging still from yesterday....

---

Well, I have some not_good stuff going on:

```
00199567632d[CPU0  ] page fault for address 0000000080002b3d @ 0000000080002b3d
00199567632d[CPU0  ] exception(0x0e): error_code=0000
00199567632d[CPU0  ] interrupt(): vector = 0e, TYPE = 3, EXT = 1
00199567632d[CPU0  ] page walk for address 0x00000000003fd870
00199567632e[CPU0  ] interrupt(): gate not present
00199567632d[CPU0  ] exception(0x0b): error_code=0073
00199567632d[CPU0  ] exception(0x08): error_code=0000
00199567632d[CPU0  ] interrupt(): vector = 08, TYPE = 3, EXT = 1
00199567632e[CPU0  ] fetch_raw_descriptor: GDT: index (40bf) 817 > limit (7f)
00199567632d[CPU0  ] exception(0x0d): error_code=40b9
00199567632i[CPU0  ] CPU is in protected mode (active)
00199567632i[CPU0  ] CS.mode = 32 bit
00199567632i[CPU0  ] SS.mode = 32 bit
00199567632i[CPU0  ] EFER   = 0x00000000
00199567632i[CPU0  ] | EAX=00000000  EBX=8006b078  ECX=003fe000  EDX=00000002
00199567632i[CPU0  ] | ESP=00000000  EBP=00000000  ESI=90000180  EDI=8006b078
00199567632i[CPU0  ] | IOPL=0 ID vip vif ac vm RF nt of df if tf SF zf af PF CF
00199567632i[CPU0  ] | SEG sltr(index|ti|rpl)     base    limit G D
00199567632i[CPU0  ] |  CS:0008( 0001| 0|  0) 00000000 ffffffff 1 1
00199567632i[CPU0  ] |  DS:0010( 0002| 0|  0) 00000000 ffffffff 1 1
00199567632i[CPU0  ] |  SS:0010( 0002| 0|  0) 00000000 ffffffff 1 1
00199567632i[CPU0  ] |  ES:0010( 0002| 0|  0) 00000000 ffffffff 1 1
00199567632i[CPU0  ] |  FS:0010( 0002| 0|  0) 00000000 ffffffff 1 1
00199567632i[CPU0  ] |  GS:0010( 0002| 0|  0) 00000000 ffffffff 1 1
00199567632i[CPU0  ] | EIP=80002b3d (80002b3d)
00199567632i[CPU0  ] | CR0=0xe0000011 CR2=0x80002b3d
00199567632i[CPU0  ] | CR3=0x00000000 CR4=0x00000010
(0).[199567632] ??? (physical address not available)
00199567632p[CPU0  ] >>PANIC<< exception(): 3rd (13) exception with no resolution
```

I had to look up what an exception `0x0b` was.  It's a "Segment Not Present" exception.  Again, not good.  It tells me that I am probably clobbering my GDT somewhere along the way.  Well, check that: it's after a page fault, so maybe I'm clobbering my IDT.

Yeah, the IDT:

```
<bochs:3> info idt
Interrupt Descriptor Table (base=0x00000000003fd800, limit=2047):
IDT[0x00]=??? descriptor hi=0x00000000, lo=0x00000000
IDT[0x01]=??? descriptor hi=0x00000000, lo=0x00000000
IDT[0x02]=??? descriptor hi=0x00000000, lo=0x00000000
IDT[0x03]=??? descriptor hi=0x00000000, lo=0x00000000
IDT[0x04]=??? descriptor hi=0x00000000, lo=0x00000000
IDT[0x05]=??? descriptor hi=0x00000000, lo=0x00000000
IDT[0x06]=??? descriptor hi=0x00000000, lo=0x00000000
IDT[0x07]=??? descriptor hi=0x00000000, lo=0x00000000
IDT[0x08]=32-Bit Interrupt Gate target=0x40b8:0xa1d81e00, DPL=0
IDT[0x09]=Code segment, base=0x4000cf1f, limit=0x80013fff, Execute/Read, Conforming, 64-bit
IDT[0x0a]=??? descriptor hi=0xcf1f0010, lo=0xa1d88e00
IDT[0x0b]=Data segment, base=0x1e53fc80, limit=0x000c9c00, Read-Only, Expand-down
IDT[0x0c]=Code segment, base=0xfc7486fc, limit=0x08006fff, Execute-Only, Conforming, Accessed, 16-bit
IDT[0x0d]=Code segment, base=0x74806018, limit=0x274e8fff, Execute-Only, Conforming, 32-bit
IDT[0x0e]=16-Bit Trap Gate target=0x43b4:0x9d1fe80d, DPL=0
IDT[0x0f]=Code segment, base=0xcb4be995, limit=0x8ede9fff, Execute-Only, Conforming, 32-bit
```

Well, OK both:

```
<bochs:4> info gdt
Global Descriptor Table (base=0x00000000003fd000, limit=127):
GDT[0x00]=??? descriptor hi=0x00000000, lo=0x00000000
GDT[0x01]=??? descriptor hi=0x00000000, lo=0x00000000
GDT[0x02]=??? descriptor hi=0x00000000, lo=0x00000000
GDT[0x03]=??? descriptor hi=0x00000000, lo=0x00000000
GDT[0x04]=??? descriptor hi=0x00000000, lo=0x00000000
GDT[0x05]=??? descriptor hi=0x00000000, lo=0x00000000
GDT[0x06]=??? descriptor hi=0x00000000, lo=0x00000000
GDT[0x07]=??? descriptor hi=0x00000000, lo=0x00000000
GDT[0x08]=Code segment, base=0x04000000, limit=0x00000000, Execute/Read, Non-Conforming, 16-bit
GDT[0x09]=??? descriptor hi=0x000000cf, lo=0xef2ca8b8
GDT[0x0a]=??? descriptor hi=0x00000000, lo=0x00000000
GDT[0x0b]=??? descriptor hi=0x00000000, lo=0x00000000
GDT[0x0c]=32-Bit Trap Gate target=0x0000:0x00000000, DPL=2
GDT[0x0d]=??? descriptor hi=0x00000000, lo=0x00000000
GDT[0x0e]=??? descriptor hi=0x00000000, lo=0x00000000
GDT[0x0f]=??? descriptor hi=0x00000000, lo=0x00000000
```

These are both located in the same frame (`0x3fd000`), so it is probably getting allocated from the PMM somehow.  But, looking at the physical memory at that location looks reasonable.

Ahhh... getting closer:

```
<bochs:8> page 0x3fd000
 PDE: 0x00000000f000ff73    ps         A PCD pwt S W P
 PTE: 0x00000000ffffffff       G PAT D A PCD PWT U W P
linear page 0x00000000003fd000 maps to physical page 0x0000fffff000
```

It looks like my PDE is getting clobbered....  There's a clue....  My `cr3` register is getting clobbered.  This means I am scheduling to something that it never should see...  such as a `Process_t` at `0x00000000`, perhaps?

The problem at this point is that I am unable to look at virtual memory since `cr3` was clobbered.  I will have to set up for a new session and break ahead of damage (I hope anyway).  I am going to start with `ProcessUnblock()` since that is a likely problem.

A second debugging session identified that the problem is happening between the first and second call to `ProcessUnblock()`.  It could very well be in that first call.

---

Ok, to make sure I do not have a problem with `ProcessBlock()` and `ProcessUnblock()`, I removed my tests for `ProcessMicroSleepUntil()`.  This code still works properly (x86-pc), so the problem must be in the `ProcessMicroSleepUntil()` function.

---

I found the bug... I was changing the list before I was getting the next element in the list.

Anyway, this is now testing properly, so I will commit again.

---

The next step in the tutorial is to handle idle time.  I currently have an idle time, but the tutorial coaches that this may not be the best approach -- in particular with today's power saving features.  Since it is not something I have done yet, I will go this route and update support for no task running.

I have this coded, but my first test locks the CPU because I have interrupts disabled when `ProcessSchedule()` is called with nothing more to run, killing everything.  So, the question becomes, can I enable interrupts during this period?  I think I can because I really need to be able to wait for *something* and that *something* can be an interrupt that will allow a process to be scheduled.

However, enabling interrupts allowed 1 extra schedule to happen.  This was not the solution.  In my mind, I keep going back to step 8 of the tutorial where we developed the concept of a delayed schedule change.  I feel I have a malformed process here and should really go back to using `Spinlock_t` to manage the locks.  This implementation would be a scheduler lock, preventing any schedule changes until the scheduler was unlocked.  Compare this to the scheduler structure locks, which would be several.

Now, the problem here if going to be to keep track of the flags and whether a process has interrupts enabled.  Actually, a process should always have interrupts enabled.  What I need to keep track of is (from the perspective of the process) is making sure I enable interrupts on the same path out as I took on the way in.  For example, the timer interrupt will always restore interrupts back to what they were on the way in.  OK, good.  A blocking process will need to enable them on the way back out as well.  This way, when a timer interrupt that disabled interrupts on the way in switches to a process that previously disabled interrupts by blocking, enables them on the way back out from being blocked.

Really, this is going to mean tracing all paths into `ProcessSwitch()` and all paths out of `ProcessSwitch()` and making certain that everything is balanced properly.

So, I need to think about what really needs to be controlled by locks and why....

#### `roundRobin` queue

This is the queue of jobs that are ready to run.  This should be pretty obvious, but we do not want anything changing the list while we are traversing it.

#### `sleepingTasks` queue

This is the same as the `roundRobin` queue.

#### `locksHeld` counter

This is far less obvious.  x86 has `inc` and `dec` opcodes that work with memory locations, so it is almost trivial to increment and decrement a counter atomically.  However, the arm does not.  This counter needs to be read into a register, incremented/decremented by adding/subtracting 1, and then written back out to a memory location.  Because of these several instructions, there is a race condition that needs to be controlled with a lock.  I do not believe that reading `locksHeld` without a subsequent update will require a lock.

It is also important to talk about `processChangePending`.  This does not need its own lock.  When it is read or updated it is read once and we move on with it's value or it is written once (based on the value of `locksHeld`).  When it is updated, it is simply stored a value of `1` to the memory location.  However, to be complete, we really should perform these updates with the `locksHeld` lock, simply to keep the 2 in sync.  These 2 variables work in conjunction with each other.  As a result of this edge case (read the value of 1 and later update a value of 0) race condition, these 2 variables will work with the same lock on this one use case.

#### `Process_t` elements

The `Process_t` structure does not have a lock on it.  Should it?  Once I get to several CPUs will I need one?  The only thing I think I need to really worry about is the list location (or the `stsQueue` member).  For this, I will need to remove it from one queue (for which I will need the proper lock to do) and then add it to the next queue (which again I need the proper lock to do).  The `status` field should correctly control which queue it is on.  So, for now, I will go without.  If nothing else, I can use `locksHeld` as a kind if double-check if need be later.

---

So, now going back to the tutorial, there is a `lock_stuff()` function that is called on entry and an `unlock_stuff()` function that is called on exit.  The premise here is that the scheduler cannot be re-entrant -- meaning that a process cannot be in the middle of blocking when an interrupt takes over and messes with what is going on.  This can probably be better represented with the function names `ProcessEnterPostpone()` and `ProcessExitPostpone()` where the former increments the `locksHeld` counter (which will be renamed to `schedulerLocksHeld` to be proper) and the latter decrements that same counter and when the `schedulerLocksHeld` counter reaches 0 performs a `ProcessSwitch()` to the proper task.  With this a timer interrupt will properly postpone a task swap when the scheduler functions are interrupted the `ProcessSwitch()` happens then the interrupted function unlocks, not from the timer callback function.

Now, step 8 was supposed to take care of all this.  Obviously I missed a few things, or properly fubar'd them.  Either way, tomorrow or Wednesday, I will be going back and fixing this mess and trying to test again.  I will also have to back out some changes to `Spinlock_t`.

---

### 2019-Mar-26

Today I start by mapping the paths into `ProcessSwitch()`.  `ProcessSwitch()` will also postpone a process swap when `schedulerLocksHeld > 0`.

So, `ProcessEnterPostpone()` increments the number of locks.  This needs to be an atomic increment, so `schedulerLocksHeld` will be protected with a `Spinlock_t` lock.  `ProcessEnterPostpone()` will also disable interrupts.

`ProcessExitPostpone()` will decrement the number of locks.  This also needs to be atomic, so the same `Spinlock_t` lock will be used.  However, when `schedulerLocksHeld` reaches 0, then `processChangePending` will be checked and if it is set, then will find and execute the next process, calling `ProcessSwitch()` to do this work.  `ProcessExitPostpone()` will also enable interrupts when `schedulerLocksHeld == 0`.

`ProcessSchedule()` calls `ProcessSwitch()`.  However, it does not directly manage the lock count; the lock *MUST* be established before calling this function and the lock *MUST* be cleared by the calling function after returning.

`ProcessCreate()` will need to call `ProcessEnterPostpone()` to lock the scheduler before adding the process to the `roundRobin` queue.  It will also need to call `ProcessExitPostpone()` to clear the lock.  `ProcessCreate()` should not directly trigger a reschedule, so the only thing that would trigger this would be something from `TimerCallBack()`.

`ProcessInit()` is irrelevant since the scheduler is not active when this function is called.

`ProcessBlock()` will lock and unlock the scheduler, calling `ProcessSchedule()` while the scheduler is locked.  When going through this function, we are guaranteed to have a process change at some point when `ProcessExitPostpone()` is called.

`ProcessMicroSleetUntil()` will lock and unlock the scheduler, calling `ProcessBlock()` after the scheduler is unlocked.  However, there are 2 paths out of this function.  Path 1 is when the current task remains the executing task.  In this case, the `schedulerLocksHeld` will be decremented then returned immediately to the executing task.  Path 2 is when there is actually some time to pass before this task can run again.  In this case, the scheduler is unlocked and then `ProcessBlock()` will be called to find something else to process.  Recall that `ProcessBlock()` will get its own lock (and I think that is a critical point).

`ProcessUnblock()` will lock the scheduler and then call `ProcessSwitch()`.  In this case, the scheduler is guaranteed to be locked to a process swap is also guaranteed to be deferred.  In this case, I think it better to simply set the `processChangePending` flag and save the call and return operations.

Finally is `ProcessStart()`, which is called to initialize a new process.  This will need to release the `schedulerLocksHeld` for whatever process just set it.  Similarly, I a termination process will need to set this counter for something else to release.

So, here is the kicker -- normal function calls must guarantee that the depth of `schedulerLocksHeld` is at most 1.  The `TimerCallBack()` may increment this lock count to 1 or to 2 only.  When it is 1, the `ProcessSwitch()` can happen immediately.  When it is 2, the swap will be deferred.

So, with this, I can clean up my code a bit.

---

I am no longer locking up.  But I am losing processes in the in the `TimerCallBack()` function.  This function is dropping all other delayed processes.

---

I have this working now.  One key call-out here: the x86 arch does not yet have a running timer while the rpi2b does.  Therefore, a request to pause for 1micro is trivial and will not actually wait.  Therefore, once "E" gets control, it does not give it up.  I am not going to change this wait because I want to get into preemption to solve this problem.

I am finally ready to commit this change.

---

### 2019-Mar-27

I am finally at the point where I can take on some preemption.  Step 11 in the tutorial is to implement the preemption.

Now, the tutorial keeps track of time, I am going to keep track of quantum (meaning the number of timer ticks that pass).  Therefore, I should not need to worry about any floating point setup for the arm cpu.  However, what I did not have before was the ability of a task to "overdraw" its quantum.  So, some changes are needed (changing the quantumLeft to an integer, which is already done).

---

For rpi2b, it appears that the timer is no longer firing!  CRAP!!  Something else to solve with this iteration.  I have the same problem with x86.  So, I am having a problem with synchronizing interrupts or something with creating a process and enabling interrupts.

After adding some debugging code, I the following sequence, where `{^0xnn}` in an increase to `nn` and `{v0xnn}` is a decrease to `nn`, `.` is a timer interrupt, and '!' is a quantum check:

```
. {^0x1} ! {v0x0} {^0x1}
. {^0x1} ! {v0x0} {v0x0}
. {^0x1} ! {v0x0} {^0x1} {v0x0}
. {^0x1} ! {v0x0}
. {^0x1} ! {v0x0}
. {^0x1} ! {v0x0} {^0x1} {v0x0} {^0x1} {v0x0} {^0x1} {v0x0}
. {^0x1} ! {v0x0} {^0x1} {v0x0}
. {^0x1} ! {v0x0}
. {^0x1} ! {v0x0} {^0x1} {v0x0}
    A {^0x1} {^0x2} {v0x1} {v0xffffffff}
    B {^0x0} {^0x1} {v0xffffffff}
    D {^0x0} {^0x1} {v0xffffffff}
    E {^0x0} {^0x1} {v0xffffffff}
    F {^0x0} {^0x1} {v0xffffffff}
 ```

So, there is definitely a problem.  In the first line, the count is increased to 1, decreased to 0, and then increased to 1 again.  Then there is a timer interrupt and the lock count is increased to 1...!!  However, I think this is a simple timing problem:

```C
    // -- interrupts are still disabled here
    if (schedulerLocksHeld == 0) {
        if (processChangePending != 0) {
            processChangePending = 0;           // need to clear this to actually perform a change
            ProcessSchedule();
        }
        EnableInterrupts();
    }

    kprintf(" {v%x} ", schedulerLocksHeld);
```

Interrupts are enabled before the debugging statement is executed.  So, it's a simple change to move that debugging statement.

That solved the first problem.  The next problem is the sequence: `A {^0x1} {^0x2} {v0x1} {v0xffffffff}` where the lock counter is increased to 1, increased to 2, decreased to 1, and then decreased to -1!!.  What happened to 0??

Now, however, I am not properly wrapping scheduler maintenance in `ProcessEnterPostpone()` ... `ProcessExitPostpone()`:

```C
        ListRemoveInit(&C->stsQueue);
        ProcessUnblock(C);
```

Now it is technically correct, but it did not fix my problem.

---

I was able to get things cleaned up for x86.  My problem was that the `Spinlock_t` functions sill updated a lock count.  I removed that code and it helped.  I also needed to remove the unlock in `ProcessStart()` and explicitly enable interrupts in the same function.  x86 is working well at this point.

However, the rpi2b timer is not firing anymore.  This broke somewhere along the way, but I am not sure when that broke.  I think I am going to have to go back in time to figure out when that broke.

---

I finally got back to a working timer.  I was able to do this by removing all code in `TimerCallBack()`.  So, I have a few issues to look at:
1. Make sure I have the frequency right.
1. Make sure I am handling the interrupt right.

I'm sure there is more; I will look more into that tomorrow.

---

### 2019-Mar-28

I am having a hell of a time with the timer on this rpi2b.  What a damned mess this is.  I'm going around in circles here and I'm not thrilled about it.

Currently, as soon as I enable IRQs for the timer, I am burying the cpu in interrupts.  Whatever is triggering it, it is not getting reset in my EOI routine.  However, again, whatever is triggering it, it is being represented (or should I say recognized) as a timer interrupt.

Adding a dump into the code to get the IRQ flags, I get the following (what the hell??!!):

```
Interrupt flags are 0x74696d65
```

OK, so hold on....  in `PicInit()`, I am writing to this register:

```C
    MmioWrite(base2 + TIMER_IRQ_SOURCE + (core * 4), 0x00000002);           // enable IRQs from the core for this CPU
```

In `PicGetIrq()`, I am reading from the same register:

```C
    archsize_t irq = MmioRead(dev->base2 + TIMER_IRQ_SOURCE + (core * 4));
```

OK, this implies to me that there is very likely a problem with my inline assembly.  Except it's not inline assembly, but MMIO reads/writes.

---

It dawns on me that `0xf8003060` might not be the correct address.  I am actually expecting this to be `0xf9003060`.  So, I need to go research that first.

---

With that corrected, I am now getting another IRQ that I am not expecting and I am not sure how to clear it.

```
Interrupt flags at 0xf9003060 are 0x00000100
Basic flags are 0x00000100
Block flags1 are 0x00008000
Block flags2 are 0x00000000
PANIC: Unhandled interrupt: 0x8
```

So, line by line:
1. I have a GPU IRQ to handle
1. I have at least 1 interrupt in IRQPEND1 register
1. I have a pending IRQ15

(oh, by the way, IRQ 15 is not documented)

This looks to be a USB IRQ.  `lk` does not have a USB implementation.  However, `rpiboot` should.  I will have to research there.

---

### 2019-Mar-29

I started with a crap-ton of research today, and a lot of reading... and some debugging code.  Here is what I know:
* Reading the USB register that should reset the interrupt pending flag did nothing
* If I do not enable the timer irq, then I get interrupts that fire, but they are all spurious
* If I enable the timer irq, then I get interrupts that fire and bury the CPU

at one point, I changed the `cpsr` to also have the `A` (I think it was) bit set for additional interrupts.  I need to double check that.  Changing that back had no effect on the behavior.

Actually, I think I need to not enable the timer IRQ and focus on eliminating the spurious IRQ.  Then I can come back to the timer.

Looking at my output, it actually looks like the timer is really firing.  Check that: it looks like *something* is triggering a (somewhat) regular interrupt:

```
Interrupt flags at 0xf9003060 are 0x00000000
Basic flags at 0xf800b200 are 0x00000000
Block flags1 at 0xf800b204 are 0x00000000
Block flags2 at 0xf800b208 are 0x00000000
+@E+@E+@E+@E+@E+@E+@E+@E+@EInterrupt flags at 0xf9003060 are 0x00000000
Basic flags at 0xf800b200 are 0x00000000
Block flags1 at 0xf800b204 are 0x00000000
Block flags2 at 0xf800b208 are 0x00000000
+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@E+@Interrupt flags at 0xf9003060 are 0x00000000
Basic flags at 0xf800b200 are 0x00000000
Block flags1 at 0xf800b204 are 0x00000000
Block flags2 at 0xf800b208 are 0x00000000
E+@E+@E+@E+@E+@E+@E+@E+@E+@Interrupt flags at 0xf9003060 are 0x00000000
Basic flags at 0xf800b200 are 0x00000000
Block flags1 at 0xf800b204 are 0x00000000
Block flags2 at 0xf800b208 are 0x00000000
```

---

OK, holy crap!!  This issue turned out to be related to MMU mapping flags.  Once I got that cleared up, the spurious IRQs went away and I am now getting a timer tick -- but more to the point I am able to get the IRQ numbers dialed in correctly.

So here is where I am at:

```
Interrupt flags at 0xf9003060 are 0x00000002
Basic flags0 at 0xf800b200 are 0x00000000 (0x00000000)
Block flags1 at 0xf800b204 are 0x00000000 (0x00000000)
Block flags2 at 0xf800b208 are 0x00000000 (0x00000000)
PANIC: Unhandled interrupt: 0x1
```

A couple of notes here:
1. the irq number is wrong and I need to get that aligned properly
1. the code to determine irq number is way inefficient

In the interest of a quick test, I am going to install the IRQ handler to 1.

---

Getting this cleaned up is done.  And the rpi works!  One last test with the x86 to make sure I did not break anything there and I should be ready to commit these changes.

---




