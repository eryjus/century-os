# The Century OS Development Journal - v0.7.0

I am currently debating what this version really should be.  The top contenders are:
* Revisit user mode processes
* Start the VFS
* Take on basic drivers for KVM (Keyboard, Video, Mouse)

### 2020-Apr-17

So, as much as I want to get into something more material, I think I need to get user-mode processes running again.  The reason I believe this is that I still think that is foundational and the other 2 things are at least partially dependent on user-mode.

So, I think I have decided on revisiting User-Mode processes.  This, then, brings up: *what user mode process?*  I currently have nothing that would qualify.


## Version 0.7.0a -- Revisit User mode processes

So, with this version, I am going to revisit the user mode.  I had that working at one point and then abandoned the code in favor of stabilizing several other things.  Now that I feel those are reasonably stabilized, it's time to take on user processes again.  This will include several components:
* Exception handling (I do not want a divide by 0 to crash everything)
* Virtual Memory management (3 spaces of memory -- user, syscall, and kernel), with setup and teardown
* System Calls (I have to be able to perform some system calls to make this all work) and the start of a runtime library
* Multiboot modules (or some other feature to get a user process loaded!)
* ELF loader

---

So, all of that said for the ideal end state....  The ELF loader is optional and the Multiboot module addition is also optional at the moment.

From a minimalist perspective, I need a system call to do something measurable repeatedly but does not interfere with the debugger.  I'm thinking a simple counter, but I'm not totally convinced at this point.  Ideally, I would like something that is not a total loss, but that may be a fantasy.

After a lot of looking, I cannot really find a syscall to implement.  The best thing I might be able to do is `nice(2)`, which will allow me to manage the priority of the process.

---

### 2020-Apr-18

Looking at the Posix standard for `nice`, this can change the how favorably a process is scheduled, where a negative number makes the scheduling more favorable and a positive number is less favorable.  For Century, this can impact in 2 ways:
* the priority can change (I only have 3 priorities available for a user process -- HIGH, NORMAL, LOW -- as OS and IDLE are reserved for OS processes and device drivers)
* the time slice is set to be the priority -- so the higher the priority the longer it gets to stay on the CPU.

Now, on the other hand, `sched_yield()` might be an option....  I can write this such that the quantum remaining is not reset and I should be able to see this value grow.  However, I would only see this when the process gets CPU time.  On the other hand, the total time used should increase very slowly.  I will start with that.

So, to start, I am going to start to build out the system calls side of things.  For this to work, I am going to need several things:
1. A system called function to actually perform the yield.
1. A landing point for the system call to change out the stack, and manage the virtual memory.
2. Start a runtime library module that starts to handle the system calls for other modules.

Now, on the other hand, I think I may want to create a process who's function is to sleep for a few seconds and then launch a new user process.  This user process is responsible for self-terminating only (as in an `exit` system call).  The point of this would also be to be able to monitor the PMM to see if the amount of memory was decreasing -- indicating a memory leak.  I think I like this test better.

---

OK, I think I have the syscall foundation written for both archs.  Both support up to 6 arguments to the system calls.  I hope I do not need more.  The arm arch is a bit fussier as I can encode the system call function number into the instruction, but that is an immediate value.  As a result, I am dealing with a jump table for each different system call.  I may regret that decision.

Now, I need to write the kernel-side of the `exit` system call.  This should really be a rather trivial call to `ProcessEnd()`, except that I have no way of handling the return code.  For now, I am going to ignore it and [this Redmine](http://eryjus.ddns.net:3000/issues/467) has been created to track this task.

---

OK, so I now have the start of a C runtime library and the first system call defined.  I am not able use this from the kernel (address space issues), so I will need to develop a module for this test.

I now have a test module building.  I did opt for now to allow the default locations for the binary executable to be used.  These are different by architecture.  I may or may not keep this setup.

At this point, I am going to push my code.  I do not expect a change in behavior when running since all the kernel changes are in the syscall section.

---

### 2020-Apr-19

Today I am going to get the new `test.elf` object to load and then determine what I need to clean up in order to preserve the code.

OK, the module loads.  For x86, it is reported to be here:

```
Module information present
   Found Module:
    .. Name is at : 0x000100ac
    .. Start: 0x00219000
    .. End: 0x0021b9bc
```

I'm certain that this is being cleaned up by the butler.  Checking the rpi, I get the following:

```
Module information present
   Found Module: test.elf
    .. Name is at : 0x000fffbc
    .. Start: 0x001da000
    .. End: 0x001de000
```

Now there is a difference: the x86 is not outputting the name; I think I need to put that in the `grub.cfg` file.  That works.

Now, what to do with the module?  There are a couple of things I can do:
1. leave the module where it is and work around it
2. copy the module from its load location to a better permanent location

One issue I have here is how the elf is organized:

```
Program Headers:
  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align
  LOAD           0x000000 0x08048000 0x08048000 0x00178 0x00178 R E 0x1000
  LOAD           0x000178 0x08049178 0x08049178 0x00004 0x00004 RW  0x1000
```

The actual size of the interesting parts is < 1 page (0x178 + 0x4 == 0x17c bytes long).  But, the addresses are to map 2 different pages to the same frame.  Moreover, in my environment, I am assuming a 1:1 relationship between the pages and frames unless it is a kernel page.  Well, that will have to be [addressed at some point](http://eryjus.ddns.net:3000/issues/468).  That said, for now I am not going to be freeing this memory, so it will not be relevant for this iteration.

So, for now, I am going to map-in-place (not going to move the frames).  Later, I do expect that I will have several additional modules to load and I will want to replace this with some sort of ramdisk containing all the files required to actually get disk access so I can complete the boot.  I also expect that this will be compressed in some way.  When that happens, I will have to relocate all kinds of stuff, and then map in place.

So, the next thing to do is to revisit the butler clean-up to make sure I am keeping the relevant module memory.  That was actually rather simple since I am keeping track of `localHwDisc->modHighestFrame`.  All I had to do was replace the check for the end of the kernel with this value.

So, the last thing to do will be to create the new Process and start it.  This will be the fussiest work since I need to also drop the process into user mode.  To do this, I need to (at least conceptually) have the `ProcessStart()` function simulate a return from a system call to the kernel.  The reason for this is that the kernel functions `ProcessUnlockScheduler()` and `EnableInterrupts()` are both called before handing control over to the actual process.  These are kernel functions that would be unmapped for a user process.  So, after both of those are completed (well, maybe not the `EnableInterrupts()` call as flags would be restored later) I will need to drop into user mode.  A return across privileges is required and this is different than a kernel process.  Hmmm...  could `longjmp()` work??  Well, that's part of libc, but maybe I can duplicate it a bit...

Well, I gotta start somewhere.  I am going to start by creating the virtual memory space for the new user-process.  This should be relatively simple as I should only need to allocate a frame, map it to a temporary location, and copy the top-level table entries for the kernel space.  This will be arch-specific work to complete.  Then, I can replace the current top-level MMU structure with the new one (all the kernel space is mapped!) and map in the user process memory.  Then, I can finally put the old MMU top level structure back.  For arm, this should be a bit easier as the kernel space structure is separated from the user space structure.

Hmmm....  as I am getting into this, I am left wondering if I do not create a new kernel process that has the responsibility to build itself out....  For this to work, I will need a kernel stack and a user stack, where I can build out the user stack from the kernel stack.  Right now, I do not have the second kernel stack for interrupts and kernel calls.  I need to get that set up and running first.

So, what are we talking about?
* Each process has 2 stacks -- 1 for the normal process and 2 for interrupts and kernel calls
* Some processes can be started on the process stack (since they run at privilege) and others need to start on the kernel stack and return as if they just returned from a syscall

So, for this to work, I am going to need to get a second stack set up on all processes and handle that cleanup in the butler.  For this step 1, we will not use this stack.

OK, so I have the structures created.  Now to populate them and clean them up (again, still not making use of them).  Ok, now that I have the structures created and cleaned up, I am  now ready to start to use them.... This will be in an interrupt or exception for all archs.  For this, I have some research to do....

So, when an interrupt or service call occurs, the following need to take place:
* Save the state (SPSR & LR)
* Change the state to supervisor mode
* Save the current stack to the Process structure
* Set the new SP from the kernel stack in the Process structure
* save all the registers

And, on the way back out:
* restore all the saved registers
* get the SP from the process stack
* restore the state (CPSR & PC)

The challenge here for arm is that there are 3 registers that are not going to be saved: `sp`, `lr`, and `pc`.  `pc` is out of the question, so that leaves `sp` and `lr`.  So, I can use those as work registers to set the new `sp`.  On the way back out, the same registers are not restored.  And, again, `pc` is out of the question.  So, I can use `sp` and `lr` as work registers to get this accomplished.  For x86, I have direct access to the memory, so these constraints are not a concern.

The other concern is that I need a place to save the `sp` register.  I would consider using `tosProcess` for this, but it is used for process changes.  However, a timer callback will also require that same field.  So, I think I need another save location for the process stack.  The problem I am having at this point is how to get through the preemptive process swap and the voluntary process swap both to get to the same result.

So, let's think about this again for a timer preemption:
* Process gets interrupted
  * state is pushed onto the stack
  * stack location is saved to process (stack member 1)
  * stack is replaced with kernel stack (stack member 2)
* Timer callback is execute and preemption is determined
  * state is saved to the current stack
  * stack location is saved to process (stack member 3)
  * stack is replaced with new process stack (stack member 3 of new process)

So, I need 3 stack saved location in each process:
1. The interrupted stack location
2. The kernel starting stack location
3. The save stack location on process change

OK, I have the 3 locations in the `Process_t` structure.  I have also updated the process changes to use the proper structure member.  The next step will be to replace the process stack with the proper kernel stack, properly saving the state.  Before I get into these changes, since the system properly runs at the moment, I am going to commit these changes.

---

### 2020-Apr-20

So, now it is time for some more intrusive changes.  I will start with x86.  On an interrupt, I will need to perform the following tasks:
* Save the current stack to the `Process_t` structure
* Replace the stack with the kernel stack
* Complete the rest of the interrupt handler
* Restore the process (current) stack from the `Process_t` structure

---

Hmmmm:

> The processor handles calls to exception- and interrupt-handlers similar to the way it handles calls with a CALL instruction to a procedure or a task. When responding to an exception or interrupt, the processor uses the exception or interrupt vector as an index to a descriptor in the IDT. If the index points to an interrupt gate or trap gate, the processor calls the exception or interrupt handler in a manner similar to a CALL to a call gate (see Section 5.8.2, “Gate Descriptors,” through Section 5.8.6, “Returning from a Called Procedure”). If index points to a task gate, the processor executes a task switch to the exception- or interrupt-handler task in a manner similar to a CALL to a task gate (see Section 7.3, “Task Switching”).
>
> When the processor performs a call to the exception- or interrupt-handler procedure:
> * If the handler procedure is going to be executed at a numerically lower privilege level, a stack switch occurs.  When the stack switch occurs:
>   1. The segment selector and stack pointer for the stack to be used by the handler are obtained from the TSS for the currently executing task. On this new stack, the processor pushes the stack segment selector and stack pointer of the interrupted procedure.
>   2. The processor then saves the current state of the EFLAGS, CS, and EIP registers on the new stack (see Figure 6-4).
>   3. If an exception causes an error code to be saved, it is pushed on the new stack after the EIP value.
> * If the handler procedure is going to be executed at the same privilege level as the interrupted procedure:
>   1. The processor saves the current state of the EFLAGS, CS, and EIP registers on the current stack (see Figure 6-4).
>   2. If an exception causes an error code to be saved, it is pushed on the current stack after the EIP value.

This leads me to want to make all processes run at ring 1 to force a push of the stack and replacement with that from the TSS.

And I think I got talked out of it:

```
[15:33:17] <eryjus_> hmmm....  "If the [exception- or interrupt-]handler procedure is going to be executed at a numerically lower privilege level, a stack switch occurs."  This kinda makes me want to have all my "privileged" processes run in ring 1 and save ring 0 for the exceptions and kernel calls.
[15:34:39] <doug16k> I'd say don't because most machines don't have that
[15:35:35] <heat> yeah
[15:35:44] <heat> it's mostly ring-3 and ring-0 only
[15:36:04] <geist> eryjus_: yeah it's a neat thing to do, but it's super deprecated and removed in modern stuff
[15:36:04] <heat> and ring -1 and ring -2 too, but those are for different things ;)
[15:36:12] <geist> so really ring 1 and ring 2 are just historical oddifies
[15:36:16] <geist> oddities
[15:36:48] <heat> huh, just noticed
[15:36:53] <geist> and if you want to have actually proper and interesting 4 ring systems, VAX is a fun architecture
[15:37:06] <heat> I wonder when do you start to be a user vs supervisor in the paging system
[15:37:08] <geist> since it's historical at this point anyway
[15:37:19] <doug16k> heat, 2
[15:37:28] <geist> heat: 386 basically started deprecating it, honestly, since it statred with out user v supervisor
[15:37:38] <doug16k> supervisor is cpl != 3
[15:37:44] <geist> essentially the 4 ring stuff was added for 286, as was the fancy descriptor based segments
[15:37:45] <heat> ohhh
[15:37:50] <geist> and then 386 went in a different direction
[15:37:53] <doug16k> some things are cpl <= iopl
[15:37:57] <zid> so cpl1 can do everything, except read memory of cpl1?
[15:38:01] <zid> of cpl0*
[15:38:17] <geist> well you can still use segmentation to keep it from reading kernel stuff
[15:38:21] <heat> looks like cpl1 can also read memory of cpl0
[15:38:26] <zid> I never bothered to find out which things were ring0 only, and which things were != 3
[15:38:33] <zid> so I have no idea what 1 or 2might be useful for
[15:38:36] <geist> but yes, if they wanted to they could have just added 2 bits to each page table entry but they didn
[15:38:49] <geist> that's exactly what VAX does, which clearly 286 was trying to copy from
[15:39:43] <geist> like a lot of vestigial stuff, the designers of 286 had a vision that they were going for, clearlyg interested in VAX and supposedly heavily influenced by iAXP 432 which was very descriptor based
[15:40:07] <geist> but then 3 or 4 years later when 432 was canned and 68K was turning into a proper user/supervisor w/paging system they tossed together 386 which was far more 'regular' looking
[15:40:28] <eryjus_> so, I am mostly after the stack-swapping feature.  Am I remembering correctly that the x86_64 will swap stacks for you on all interrupt- and exception- calls?
[15:40:50] <geist> it *can* using the ISTs but there are only 7 of them so it's not a generally useful feature
[15:41:09] <geist> x86_64 works more or less the same way as x86-32, there are just only two rings
[15:41:22] <doug16k> there are some instructions that only work if cpl == 0, some that work only if cpl <= eflags.iopl, and some that work only if cpl < 3
[15:41:30] <geist> and in that case the cpu will switch stacks automatically if its transitioning from user to supervisor
[15:41:43] <geist> but if supervisor to supervisor it wont, *unless* you use an IST
[15:42:37] <geist> and the IST was clearly added because of the loss of a TSS task gate, which was the canonical way to trap a #DF or #NMI
[15:42:58] <geist> so they removed the task gate stuff but added a simple supervisor stack swapping thing to let you at least do what you were already doing
[15:43:15] <eryjus_> geist, what does the aarch64 have?
[15:43:17] <geist> anyway it's pretty advanced shit, point is yes you can swap stacks
[15:43:27] <doug16k> (because the whole reason for using the task gate was to switch stacks)
[15:43:32] <geist> eryjus_: each run level (EL0, EL1, EL2, etc) has a banked SP
[15:43:48] <geist> so when you switch from EL0 (user space) to EL1 (kernel space) it simply starts using the banked stack pointer
[15:44:01] <geist> that's what most 'sane' architectures do
[15:44:42] <heat> are NMIs used for anything these days?
[15:44:46] <eryjus_> that's backed by core, not by process...  so there is still the managing of the stack to get the one I want -- or manage the banks on process change
[15:44:56] <geist> eryjus_: correct
[15:44:57] <doug16k> sure, performance counter sampling
[15:44:59] <eryjus_> s/backed/banked/
[15:45:14] <doug16k> can't profile locks with an IRQ tick
[15:45:31] <heat> oh right right
[15:45:37] <geist> eryjus_: so when in EL1 (supervisor mode) the SP is intrinsically pointing at SP_EL1, but you can directly write to SP_EL0 (user space) so before you swap back to user space you can fill it in
[15:46:12] <eryjus_> geist, is that the same with armv7?
[15:46:13] <geist> and on irq entry you probably just save it on the stack and restore it on the way back, like other registesr you save from user space
[15:46:39] <geist> eryjus_: basically, though it looks a little different, and it's actually more complicated (v8-64bit actually simplifies it) but yes
[15:46:44] <geist> there are banked SPs (r13) in arm32
[15:47:18] <eryjus_> but you can reach across the banks and update the other stack?
[15:47:57] <geist> on armv7? it's complicated, but yes.
[15:48:34] <geist> on v8 it's much more logically sane: 4 levels of cpu (EL0 - EL3) and each higher level can access everything in the lower level, but not the other way around
[15:48:58] <geist> FWIW riscv works basically identically
[15:50:33] <eryjus_> thx
[15:51:06] <geist> key is that on most risc machines, ARM, riscv, mips, etc. nothing is automatically pushed on the stack during an exceptino or syscall by hardware
[15:51:26] <geist> so if hardware doesn't need to atomically access a stack like x86 does, then there's no need for there to be a mechanism to swap stacks automatically like x86 is
[15:51:38] <geist> but even if you did, then having banked copies of the stack is a simple and convenient way to do it
```

So, the key problem here for x86 is that the CPU *may* change privilege levels on an interrupt, and when it does the stack is replaced.  If the stack is replaced, I am already on the proper stack whereas if the interrupt is to the same privilege level the same stack is used.

> Voice of reason: Ok!!!  STOP!!!  What is the real problem here????!!!  You are struggling because you are not able to properly articulate the problem statement.
>
> Confused: I need to be able to start a process in user space.  For that to work, I need to be able to return from privilege using an `iret` instruction.  When an `iret` executes and there is a privilege level changes, the `ss` and `esp` registers are also popped off the stack, effectively replacing the stack.
>
> Voice of reason: So what?  Just because the `ss` and `esp` registers are replaced, it does not mean that we need 2 stacks to start a process.  Just use the same stack and the proper values.
>
> Confused: .....
>
> Voice of reason: So, on all other interrupts and exceptions, the user stack can still be used for the kernel-to-kernel call whereas the user-to-kernel stack replacement will happen automatically by updating the TSS.  There is no need to get fancy in the interrupt handler.

So, that was an epiphany!  D'oh!!  All I really need to do is construct one stack properly.

OK, so now what to do about the binary image and getting it mapped.

---

### 2020-Apr-21

Today I completed all the coding I needed to get into user mode on both archs.  Tomorrow, time willing, I will test.

---

### 220-Apr-22

Let the faults begin!

```
Page Fault
CPU: 0
EAX: 0x00000000  EBX: 0x00000000  ECX: 0x00000000
EDX: 0x00000001  ESI: 0x900005f0  EDI: 0x00200292
EBP: 0x800041d8  ESP: 0xff800f9c  SS: 0x10
EIP: 0x8080210d  EFLAGS: 0x00200082
CS: 0x8  DS: 0x28  ES: 0x28  FS: 0x28  GS: 0x48
CR0: 0xe0000011  CR2: 0xff409000  CR3: 0x01001000
Trap: 0xe  Error: 0x0
```

OK, `cr2` is `ELF_TEMP_HEADER`.  This is not getting mapped.  The `ElfImage_t` structure was not mapped....  Next:

```
Heap Error!!! HeapAlloc() - Invalid Heap Header Magic Number

CPU: 0
EAX: 0x0000003d  EBX: 0x0022700b  ECX: 0x00000000
EDX: 0x8080a730  ESI: 0x00227008  EDI: 0x81000a00
EBP: 0x00200292  ESP: 0xff800ef0  SS: 0x10
EIP: 0x00200096  EFLAGS: 0x80809210
CS: 0x80802e59  DS: 0x28  ES: 0x28  FS: 0x28  GS: 0x48
CR0: 0xe0000011  CR2: 0x00000000  CR3: 0x01001000
Trap: 0x0  Error: 0x8
```

Ok, *that* I was not expecting!  I had to clean up the registers..  but now it is:

```
Invalid ELF Signature
Heap Error!!! HeapAlloc() - Invalid Heap Header Magic Number



CPU: 0
EAX: 0x0000003d  EBX: 0x0022700b  ECX: 0x00000000
EDX: 0x8080a730  ESI: 0x00227008  EDI: 0x81000a00
EBP: 0x00200292  ESP: 0xff800ee8  SS: 0x10
EIP: 0x80802e59  EFLAGS: 0x00200096
CS: 0x8  DS: 0x28  ES: 0x28  FS: 0x28  GS: 0x48
CR0: 0xe0000011  CR2: 0x00000000  CR3: 0x01001000
Trap: 0x0  Error: 0x0
```

I forgot to convert the addresses returned by multiboot to frame.  Now I'm at triple fault.  I forgot to adjust the new top-level table to its address.  Fixing that, I have:

```
Page Fault
CPU: 0
EAX: 0x7df61007  EBX: 0x0017df62  ECX: 0x00000000
EDX: 0x08048080  ESI: 0x90000624  EDI: 0x08048080
EBP: 0xff800f8c  ESP: 0xff800edc  SS: 0x10
EIP: 0x80805ecf  EFLAGS: 0x00200096
CS: 0x8  DS: 0x28  ES: 0x28  FS: 0x28  GS: 0x48
CR0: 0xe0000011  CR2: 0x0803cffc  CR3: 0x01024000
Trap: 0xe  Error: 0x2
```

This time, I do not believe that the page is not mapped, but has the incorrect attributes.  In particular, the following statement is being used to create the mapping:

```c++
MmuMapToFrame(USER_STACK, rv, PG_WRT);
```

This does not have the `PG_KRN` attribute.  However, adding that attribute in as a test does not work.  I switch to Bochs for its debugger.

```
<bochs:2> page 0x803cff8
 PDE: 0x0000000000000000    ps         a pcd pwt S R p
physical address not available for linear 0x000000000803c000
```

So the Page Directory is not being created.

```
<bochs:3> creg
CR0=0xe0000011: PG CD NW ac wp ne ET ts em mp PE
CR2=page fault laddr=0x000000000803cff8
CR3=0x000001024000
    PCD=page-level cache disable=0
    PWT=page-level write-through=0
CR4=0x00000000: cet pke smap smep osxsave pcid fsgsbase smx vmx osxmmexcpt umip osfxsr pce pge mce pae pse de tsd pvi vme
CR8: 0x0
EFER=0x00000000: ffxsr nxe lma lme sce
```

But `cr3` is valid.

OK, I am not properly checking the Page Directory:

```
Mapping page 0x0803c000 to frame 0x17df61
... Kernel: no
... Device: no
... Write.: yes
.. MmuMapToFrame sanity checks passed
... The contents of the PTE is at 0xffc200f0: 0x7df61007

Page Fault
CPU: 0
EAX: 0x08048080  EBX: 0x0017df61  ECX: 0x00000000
EDX: 0x8080b7a5  ESI: 0x90000624  EDI: 0x08048080
EBP: 0xff800f8c  ESP: 0xff800eec  SS: 0x10
EIP: 0x80805fb5  EFLAGS: 0x00200096
CS: 0x8  DS: 0x28  ES: 0x28  FS: 0x28  GS: 0x48
CR0: 0xe0000011  CR2: 0x0803cff8  CR3: 0x01024000
Trap: 0xe  Error: 0x2
```

OK, I see the problem here:

```
<bochs:5> page 0xfffff080
 PDE: 0x0000000001001063    ps         A pcd pwt S W P
 PTE: 0x0000000001001063       g pat D A pcd pwt S W P
linear page 0x00000000fffff000 maps to physical page 0x000001001000
```

I changed the top-level table, but did not replace the recursive mapping.  This needs to be done in the call to `MmuMakeTopUserTable()`.

This now brings me back to a triple fault -- but the debugger is getting started.  So, back to Bochs:

```
00230515543i[CPU3  ] CPU is in protected mode (active)
00230515543i[CPU3  ] CS.mode = 32 bit
00230515543i[CPU3  ] SS.mode = 32 bit
00230515543i[CPU3  ] EFER   = 0x00000000
00230515543i[CPU3  ] | EAX=00001024  EBX=90000544  ECX=01001000  EDX=00000000
00230515543i[CPU3  ] | ESP=0803cfd4  EBP=00000000  ESI=90000544  EDI=90000624
00230515543i[CPU3  ] | IOPL=0 id vip vif ac vm RF nt of df if tf SF zf af PF CF
00230515543i[CPU3  ] | SEG sltr(index|ti|rpl)     base    limit G D
00230515543i[CPU3  ] |  CS:0008( 0001| 0|  0) 00000000 ffffffff 1 1
00230515543i[CPU3  ] |  DS:0028( 0005| 0|  0) 00000000 ffffffff 1 1
00230515543i[CPU3  ] |  SS:0010( 0002| 0|  0) 00000000 ffffffff 1 1
00230515543i[CPU3  ] |  ES:0028( 0005| 0|  0) 00000000 ffffffff 1 1
00230515543i[CPU3  ] |  FS:0028( 0005| 0|  0) 00000000 ffffffff 1 1
00230515543i[CPU3  ] |  GS:0090( 0012| 0|  0) 810003cc 00000007 0 1
00230515543i[CPU3  ] | EIP=808067fc (808067fc)
00230515543i[CPU3  ] | CR0=0xe0000011 CR2=0xff401040
00230515543i[CPU3  ] | CR3=0x00001024 CR4=0x00000000
00230515543p[CPU3  ] >>PANIC<< exception(): 3rd (14) exception with no resolution
```

Ok, this gets me to the faulting instruction:

```s
808067f9:       0f 22 d8                mov    %eax,%cr3
```

`eax` is not a valid value.  Back to a page fault:

```
Page Fault scheduler,timer,msgq)
CPU: 2
EAX: 0x00000086  EBX: 0x00000000  ECX: 0x01001000
EDX: 0x00000000  ESI: 0x00000000  EDI: 0x00000000
EBP: 0x00000000  ESP: 0x0803cfc4  SS: 0x10
EIP: 0x80803cfd  EFLAGS: 0x00000216
CS: 0x8  DS: 0x28  ES: 0x28  FS: 0x28  GS: 0x78
CR0: 0xe0000011  CR2: 0x0803d000  CR3: 0x01024000
Trap: 0xe  Error: 0x0
```

which is:

```s
80803cfd <ProcessStartEpilogue>:
80803cfd:       cf                      iret
```

So, something on the stack is a problem.  Back to Bocks:

```
<bochs:5> x /20 0x0803cfc4
[bochs]:
0x000000000803cfc4 <bogus+       0>:    0x00000000      0x00000000      0x01001000      0x00200086
0x000000000803cfd4 <bogus+      16>:    0x0000000e      0x00000000      0x00000000      0x80803cfd
0x000000000803cfe4 <bogus+      32>:    0x00000008      0x00210216      0x0000001b      0x00000202
0x000000000803cff4 <bogus+      48>:    0x0803cffc      0x00000023      0x08048080bx_dbg_read_linear: physical address not available for linear 0x000000000803d000
```

I may have a bad choice here as the `eip` value and `esp` value look like they overlap.  They do not.

So, the `iret` instruction pops the following off the stack:
* `cs`, which I am expecting `0x1b` and should be located at `0x0803cfec`

... and since I think I have a stack alignment problem, I am going to reset and try again.

OK, not I am working with a General Protection Fault:

```
General Protection Faulter,msgq)
CPU: 2
EAX: 0x00000282  EBX: 0x00000000  ECX: 0x01001000
EDX: 0x00000000  ESI: 0x00000000  EDI: 0x00000000
EBP: 0x00000000  ESP: 0x0803cfc4  SS: 0x10
EIP: 0x80803cfd  EFLAGS: 0x00000216
CS: 0x8  DS: 0x28  ES: 0x28  FS: 0x28  GS: 0x78
CR0: 0xe0000011  CR2: 0x00000000  CR3: 0x01024000
Trap: 0xd  Error: 0x18
```

Back to Bochs for more information.

```
<bochs:3> x /20 0x0803cfc4
[bochs]:
0x000000000803cfc4 <bogus+       0>:    0x00000000      0x00000000      0x01001000      0x00200086
0x000000000803cfd4 <bogus+      16>:    0x0000000d      0x00000000      0x00000018      0x80803cfd
0x000000000803cfe4 <bogus+      32>:    0x00000008      0x00210216      0x08048080      0x0000001b
0x000000000803cff4 <bogus+      48>:    0x00000202      0x0803d000      0x00000023
```

So, this looks like the previous stack.  Now, having said that, the error code is that of the User Code segment selector.  I need to figure out why that is an error.

The possible conditions are:

> #GP(selector)
> * If a segment selector index is outside its descriptor table limits.
> * If the return code segment selector RPL is less than the CPL.
> * If the DPL of a conforming-code segment is greater than the return code segment selector RPL.
> * If the DPL for a nonconforming-code segment is not equal to the RPL of the code segment selector.
> * If the stack segment descriptor DPL is not equal to the RPL of the return code segment selector.
> * If the stack segment is not a writable data segment.
> * If the stack segment selector RPL is not equal to the RPL of the return code segment selector.
> * If the segment descriptor for a code segment does not indicate it is a code segment.
> * If the segment selector for a TSS has its local/global bit set for local.
> * If a TSS segment descriptor specifies that the TSS is not busy.
> * If a TSS segment descriptor specifies that the TSS is not available.

So, let's go through these in order.

```
<bochs:4> info gdt
Global Descriptor Table (base=0x00000000ff410000, limit=167):
GDT[0x0000]=??? descriptor hi=0x00000000, lo=0x00000000
GDT[0x0008]=Code segment, base=0x00000000, limit=0xffffffff, Execute/Read, Non-Conforming, Accessed, 32-bit
GDT[0x0010]=Data segment, base=0x00000000, limit=0xffffffff, Read/Write, Accessed
GDT[0x0018]=Code segment, base=0x00000000, limit=0xffffffff, Execute/Read, Non-Conforming, 32-bit
GDT[0x0020]=Data segment, base=0x00000000, limit=0xffffffff, Read/Write
GDT[0x0028]=Data segment, base=0x00000000, limit=0xffffffff, Read/Write, Accessed
GDT[0x0030]=Data segment, base=0x00000000, limit=0xffffffff, Read/Write
```

`0x18` is within the proper limits.

However, the segment `dpl` value was 0.  Made a change and will test now again....  Back to a triple fault (which means we are probably executing the code).

Immediately, in Bochs, I can see I am trying to execute a system call:

```
<bochs:1> c
(3).[230230113] [0x000000228103] 001b:0000000008048103 (unk. ctxt): int 0x64                  ; cd64
Next at t=230230118
(0) [0x00000010d61b] 0008:000000008080661b (unk. ctxt): call .-17840 (0x80802070) ; e850baffff
(1) [0x000000107442] 0008:0000000080800442 (unk. ctxt): call .+16649 (0x80804550) ; e809410000
(2) [0x00000010d61b] 0008:000000008080661b (unk. ctxt): call .-17840 (0x80802070) ; e850baffff
(3) [0x000000228103] 001b:0000000008048103 (unk. ctxt): int 0x64                  ; cd64
```

And more information:

```
00230230113e[CPU3  ] interrupt(): SS selector null
00230230113e[CPU3  ] interrupt(): SS selector null
00230230113e[CPU3  ] interrupt(): SS selector null
00230230113i[CPU3  ] CPU is in protected mode (active)
00230230113i[CPU3  ] CS.mode = 32 bit
00230230113i[CPU3  ] SS.mode = 32 bit
00230230113i[CPU3  ] EFER   = 0x00000000
00230230113i[CPU3  ] | EAX=00000000  EBX=00000000  ECX=00000000  EDX=00000000
00230230113i[CPU3  ] | ESP=0803cfa4  EBP=00000000  ESI=00000000  EDI=00000000
00230230113i[CPU3  ] | IOPL=0 id vip vif ac vm RF nt of df IF tf SF zf AF PF CF
00230230113i[CPU3  ] | SEG sltr(index|ti|rpl)     base    limit G D
00230230113i[CPU3  ] |  CS:001b( 0003| 0|  3) 00000000 ffffffff 1 1
00230230113i[CPU3  ] |  DS:0000( 0005| 0|  0) 00000000 ffffffff 1 1
00230230113i[CPU3  ] |  SS:0023( 0004| 0|  3) 00000000 ffffffff 1 1
00230230113i[CPU3  ] |  ES:0000( 0005| 0|  0) 00000000 ffffffff 1 1
00230230113i[CPU3  ] |  FS:0000( 0005| 0|  0) 00000000 ffffffff 1 1
00230230113i[CPU3  ] |  GS:0000( 0012| 0|  0) 810003cc 00000007 0 1
00230230113i[CPU3  ] | EIP=08048103 (08048103)
00230230113i[CPU3  ] | CR0=0xe0000011 CR2=0x00000000
00230230113i[CPU3  ] | CR3=0x01024000 CR4=0x00000000
00230230113p[CPU3  ] >>PANIC<< exception(): 3rd (10) exception with no resolution
```

So, the TSS is probably not decorated properly, and sure enough:

```
<bochs:3> info tss
tr:s=0x98, base=0x00000000810003e8, valid=1
ss:esp(0): 0x0000:0x00000000
ss:esp(1): 0x0000:0x00000000
ss:esp(2): 0x0000:0x00000000
cr3: 0x00000000
eip: 0x00000000
eflags: 0x00000000
cs: 0x0000 ds: 0x0000 ss: 0x0000
es: 0x0000 fs: 0x0000 gs: 0x0000
eax: 0x00000000  ebx: 0x00000000  ecx: 0x00000000  edx: 0x00000000
esi: 0x00000000  edi: 0x00000000  ebp: 0x00000000  esp: 0x00000000
ldt: 0x0000
i/o map: 0x0000
```

This decoration needs to happen when `ProcessSwitch()`, making use of the process kernel stack.

Some quick debugging code reveals the location of the offset for `esp0`:

```
!!>> [CPU0]: The offset of the esp0 is 60
```

Still faulting..., but now I am getting several `#GP` faults before a triple fault!  The triple fault looks like this:

```
00188246901d[CPU1  ] page fault for address 00000000ffffeffc @ 00000000808069f4
00188246901d[CPU1  ] exception(0x0e): error_code=0002
00188246901d[CPU1  ] exception(0x08): error_code=0000
00188246901d[CPU1  ] interrupt(): vector = 08, TYPE = 3, EXT = 1
00188246901d[CPU1  ] interrupt(): INTERRUPT TO SAME PRIVILEGE
00188246901d[CPU1  ] page walk for address 0x00000000ffffeffc
00188246901d[CPU1  ] PTE: entry not present
00188246901d[CPU1  ] page fault for address 00000000ffffeffc @ 00000000808069f4
00188246901d[CPU1  ] exception(0x0e): error_code=0002
00188246901i[CPU1  ] CPU is in protected mode (active)
00188246901i[CPU1  ] CS.mode = 32 bit
00188246901i[CPU1  ] SS.mode = 32 bit
00188246901i[CPU1  ] EFER   = 0x00000000
00188246901i[CPU1  ] | EAX=0000000d  EBX=810006c0  ECX=00000000  EDX=8080b0c4
00188246901i[CPU1  ] | ESP=fffff000  EBP=00000000  ESI=00000001  EDI=0000000d
00188246901i[CPU1  ] | IOPL=0 id vip vif ac vm RF nt of df if tf sf zf af PF cf
00188246901i[CPU1  ] | SEG sltr(index|ti|rpl)     base    limit G D
00188246901i[CPU1  ] |  CS:0008( 0001| 0|  0) 00000000 ffffffff 1 1
00188246901i[CPU1  ] |  DS:0028( 0005| 0|  0) 00000000 ffffffff 1 1
00188246901i[CPU1  ] |  SS:0010( 0002| 0|  0) 00000000 ffffffff 1 1
00188246901i[CPU1  ] |  ES:0028( 0005| 0|  0) 00000000 ffffffff 1 1
00188246901i[CPU1  ] |  FS:0028( 0005| 0|  0) 00000000 ffffffff 1 1
00188246901i[CPU1  ] |  GS:0000( 000c| 0|  0) 81000274 00000007 0 1
00188246901i[CPU1  ] | EIP=808069f4 (808069f4)
00188246901i[CPU1  ] | CR0=0xe0000011 CR2=0xffffeffc
00188246901i[CPU1  ] | CR3=0x01024000 CR4=0x00000000
00188246901p[CPU1  ] >>PANIC<< exception(): 3rd (14) exception with no resolution
```

Tracing back, I see the following:

```
00188044906d[CPU1  ] interrupt(): vector = 64, TYPE = 4, EXT = 0
00188044906d[CPU1  ] page walk for address 0x00000000ff401320
00188044906d[CPU1  ] interrupt(): INTERRUPT TO INNER PRIVILEGE
00188044906d[CPU1  ] page walk for address 0x00000000fffffffc
00188044946d[CPU1  ] inhibit interrupts mask = 3
00188044946d[CPU1  ] page walk for address 0x0000000081004bb0
00188044956d[CPU1  ] page walk for address 0x0000000080400020
00188044961d[CPU1  ] page walk for address 0x0000000080400074
00188044966d[CPU1  ] page walk for address 0x0000000080805b40
00188044986d[CPU1  ] read_virtual_checks(): segment descriptor not valid
00188044986d[CPU1  ] exception(0x0d): error_code=0000
00188044986d[CPU1  ] interrupt(): vector = 0d, TYPE = 3, EXT = 1
00188044986d[CPU1  ] interrupt(): INTERRUPT TO SAME PRIVILEGE
```

In particular, the `esp0` address is 0 and the stack is wrapping.

More info on the triple fault:

```
Next at t=188246906
(0) [0x00000010b846] 0008:0000000080804846 (unk. ctxt): jnz .-24 (0x80804830)     ; 75e8
(1).[188246906] ??? (physical address not available)
(2) [0x00000010cc95] 0008:0000000080805c95 (unk. ctxt): mov eax, ecx              ; 89c8
(3) [0x00000010cc95] 0008:0000000080805c95 (unk. ctxt): mov eax, ecx              ; 89c8
<bochs:2> set $cpu=1
<bochs:3> sreg
es:0x0028, dh=0x00cf9300, dl=0x0000ffff, valid=1
        Data segment, base=0x00000000, limit=0xffffffff, Read/Write, Accessed
cs:0x0008, dh=0x00cf9b00, dl=0x0000ffff, valid=1
        Code segment, base=0x00000000, limit=0xffffffff, Execute/Read, Non-Conforming, Accessed, 32-bit
ss:0x0010, dh=0x00cf9300, dl=0x0000ffff, valid=31
        Data segment, base=0x00000000, limit=0xffffffff, Read/Write, Accessed
ds:0x0028, dh=0x00cf9300, dl=0x0000ffff, valid=31
        Data segment, base=0x00000000, limit=0xffffffff, Read/Write, Accessed
fs:0x0028, dh=0x00cf9300, dl=0x0000ffff, valid=1
        Data segment, base=0x00000000, limit=0xffffffff, Read/Write, Accessed
gs:0x0000, dh=0x81409300, dl=0x02740007, valid=0
ldtr:0x0000, dh=0x00008200, dl=0x0000ffff, valid=1
tr:0x0068, dh=0x81008b00, dl=0x0290006b, valid=1
gdtr:base=0x00000000ff410000, limit=0xa7
idtr:base=0x00000000ff401000, limit=0x7f
```

I still have issues, but I am exhausted today....

---

### 2020-Apr-23

OK, where did I leave off....  I know I am having problems with the system call (`int 100`) and getting a good stack picked up.

To figure out where I stand, I will add that into the process dump -- where is the stack location?  This will be under `sched show`.  I will also need to temporarily disable the Butler to get a good proper look.

Hmmm.....  Side note, this is not right:

```
+------------------------+-------------------+-------------------+-------------------+-------------------+
| CPU                    |        CPU0       |        CPU1       |        CPU2       |        CPU3       |
+------------------------+-------------------+-------------------+-------------------+-------------------+
| Process Address:       | 0x9000000c        | 0x900002bc        | 0x90000164        | 0x90000964        |
| Process ID:            | 0                 | 4                 | 2                 | 11                |
| Command:               | Butler            | Idle Process      | Idle Process      | Kernel Debugger   |
| Virtual Address Space: | 0x1001000         | 0x1001000         | 0x1001000         | 0x1001000         |
| Base Stack Frame:      | 0xff801           | 0xbffd6           | 0xbffda           | 0x17df5b          |
| Status:                | RUNNING           | RUNNING           | RUNNING           | RUNNING           |
| Priority:              | LOW               | IDLE              | IDLE              | OS                |
| Quantum Left:          | 4                 | 0                 | 0                 | 21                |
| Time Used:             | 166993000         | 76736000          | 85074000          | 161319000         |
| Wake At:               | 0                 | 0                 | 0                 | 0                 |
| Queue Status:          | Not on a queue    | Not on a queue    | Not on a queue    | Not on a queue    |
+------------------------+-------------------+-------------------+-------------------+-------------------+
```

The Base Stack Frame is not correct.  This is not readily evident so I will [file it for later](http://eryjus.ddns.net:3000/issues/471).

So, this all looks good.  So I need to get a look at the TSS again.

I also found [this bug about Interrupt landing points are located in the kernel, not the syscall section](http://eryjus.ddns.net:3000/issues/472) which needs to get fixed.

Also, with the instrumentation, I found the following originating exception:

```
00188122991i[      ] CPU 3 at 0x80805c8f: jnz .-12 (0xfffffff6)   (reg results):
00188122991i[      ] LEN 2	BYTES: 75f4
00188122991i[      ]   EAX: 0x00000000; EBX: 0x00000001; ECX 0x00000000; EDX: 0x00000001
00188122991i[      ]   ESP: 0xff80ff20; EBP: 0x00000000; ESI 0x00000082; EDI: 0x00000000
00188122991i[      ]   EFLAGS: 0x00000082 (id vip vif ac vm rf nt IOPL=0 of df if tf SF zf af pf cf)
00188122991i[      ] 	BRANCH (NOT TAKEN)
00188122991i[      ] CPU 3: exception 0dh error_code=0
00188122991i[CPU3  ] CPU is in protected mode (active)
00188122991i[CPU3  ] CS.mode = 32 bit
00188122991i[CPU3  ] SS.mode = 32 bit
00188122991i[CPU3  ] EFER   = 0x00000000
00188122991i[CPU3  ] | EAX=00000000  EBX=00000001  ECX=00000000  EDX=00000001
00188122991i[CPU3  ] | ESP=ff80ff20  EBP=00000000  ESI=00000082  EDI=00000000
00188122991i[CPU3  ] | IOPL=0 id vip vif ac vm rf nt of df if tf sf ZF af PF cf
00188122991i[CPU3  ] | SEG sltr(index|ti|rpl)     base    limit G D
00188122991i[CPU3  ] |  CS:0008( 0001| 0|  0) 00000000 ffffffff 1 1
00188122991i[CPU3  ] |  DS:0028( 0005| 0|  0) 00000000 ffffffff 1 1
00188122991i[CPU3  ] |  SS:0010( 0002| 0|  0) 00000000 ffffffff 1 1
00188122991i[CPU3  ] |  ES:0028( 0005| 0|  0) 00000000 ffffffff 1 1
00188122991i[CPU3  ] |  FS:0028( 0005| 0|  0) 00000000 ffffffff 1 1
00188122991i[CPU3  ] |  GS:0000( 0012| 0|  0) 810003cc 00000007 0 1
00188122991i[CPU3  ] | EIP=80805c97 (80805c91)
00188122991i[CPU3  ] | CR0=0xe0000011 CR2=0x00000000
00188122991i[CPU3  ] | CR3=0x01024000 CR4=0x00000000
00188122991i[      ] CPU 3: interrupt 0dh
```

and the assembly is this:

```s
80805c70 <ProcessLockScheduler>:
80805c70:       56                      push   %esi
80805c71:       53                      push   %ebx
80805c72:       52                      push   %edx
80805c73:       8b 5c 24 10             mov    0x10(%esp),%ebx
80805c77:       e8 e4 c3 ff ff          call   80802060 <DisableInterrupts>
80805c7c:       31 c9                   xor    %ecx,%ecx
80805c7e:       89 c6                   mov    %eax,%esi
80805c80:       ba 01 00 00 00          mov    $0x1,%edx
80805c85:       89 c8                   mov    %ecx,%eax
80805c87:       f0 0f b1 15 e0 05 00    lock cmpxchg %edx,0x810005e0
80805c8e:       81
80805c8f:       75 f4                   jne    80805c85 <ProcessLockScheduler+0x15>
80805c91:       65 a1 00 00 00 00       mov    %gs:0x0,%eax
80805c97:       8b 00                   mov    (%eax),%eax
80805c99:       a3 1c 06 00 81          mov    %eax,0x8100061c
80805c9e:       a1 14 06 00 81          mov    0x81000614,%eax
80805ca3:       85 c0                   test   %eax,%eax
```

or more specifically, `mov %gs:0x0,%eax`...  where `gs` is currently `0x0000`.  So, something is resetting `gs` back to `0`.  Is that the system call?  Well the instrumentation is not outputting the segment registers.

OK, so the instrumentation shows that the registers are being replaced with the system call:

```
00188122916i[      ] CPU 3 at 0x8048103: int 0x64   (reg results):
00188122916i[      ] LEN 2	BYTES: cd64
00188122916i[      ]   EAX: 0x00000000; EBX: 0x00000000; ECX 0x00000000; EDX: 0x00000000
00188122916i[      ]   ESP: 0xff80ffec; EBP: 0x00000000; ESI 0x00000000; EDI: 0x00000000
00188122916i[      ]   CS: 0x0008; DS: 0x0000; ES: 0x0000; FS: 0x0000; GS: 0x0000; SS: 0x0010;
00188122916i[      ]   EFLAGS: 0x00000097 (id vip vif ac vm rf nt IOPL=0 of df if tf SF zf AF PF CF)
00188122916i[      ] 	BRANCH TARGET 0000000080803b84 (TAKEN)
00188122916i[      ] MEM ACCESS[0]: 0x00000000ff401320 (linear) 0x000003000320 (physical) RD SIZE: 8
00188122916i[      ] MEM ACCESS[1]: 0x00000000ff410008 (linear) 0x000000010008 (physical) RD SIZE: 8
00188122916i[      ] MEM ACCESS[2]: 0x00000000810003f0 (linear) 0x0000001163f0 (physical) RD SIZE: 2
00188122916i[      ] MEM ACCESS[3]: 0x00000000810003ec (linear) 0x0000001163ec (physical) RD SIZE: 4
00188122916i[      ] MEM ACCESS[4]: 0x00000000ff410010 (linear) 0x000000010010 (physical) RD SIZE: 8
00188122916i[      ] MEM ACCESS[5]: 0x00000000ff80fffc (linear) 0x0000bffe3ffc (physical) WR SIZE: 4
00188122916i[      ] MEM ACCESS[6]: 0x00000000ff80fff8 (linear) 0x0000bffe3ff8 (physical) WR SIZE: 4
00188122916i[      ] MEM ACCESS[7]: 0x00000000ff80fff4 (linear) 0x0000bffe3ff4 (physical) WR SIZE: 4
00188122916i[      ] MEM ACCESS[8]: 0x00000000ff80fff0 (linear) 0x0000bffe3ff0 (physical) WR SIZE: 4
00188122916i[      ] MEM ACCESS[9]: 0x00000000ff80ffec (linear) 0x0000bffe3fec (physical) WR SIZE: 4
```

I believe that these are coming from the tss, and so a quick check will be to add them into the structure.  Also, if this is the case, then the save of the segment registers is flawed and will need to be replaced with something else -- and I may lose visibility to the real values and have to settle for the intended values.

Hmmmm... is also looks like my IOPL is also set wrong when I create the user stack.  I will clean that up first.

OK, now, the segment registers are zeroed out before I ever get to the interrupt.  So, I need to figure out where that might be coming from....  The problem is I have no clue on that one!!

Hmmmm... is it possible that I am not setting up the segment selectors when I init the APs?  No, I am getting them:

```s
tgt_32_bit:
    mov     eax,0x10                    ;; this is the segment selector for the data/stack
    mov     ds,ax
    mov     es,ax
    mov     fs,ax
    mov     ss,ax
```

OK, what the....????!!!!!!:

```
00188122766i[      ] CPU 3 at 0x80802261: ret    (reg results):
00188122766i[      ] LEN 1	BYTES: c3
00188122766i[      ]   EAX: 0x00000086; EBX: 0x00000000; ECX 0x01001000; EDX: 0x00000000
00188122766i[      ]   ESP: 0x0803cfec; EBP: 0x00000000; ESI 0x00000000; EDI: 0x00000000
00188122766i[      ]   CS: 0x0008; DS: 0x0028; ES: 0x0028; FS: 0x0028; GS: 0x0090; SS: 0x0010;
00188122766i[      ]   EFLAGS: 0x00000286 (id vip vif ac vm rf nt IOPL=0 of df IF tf SF zf af PF cf)
00188122766i[      ] 	BRANCH TARGET 0000000080803ced (TAKEN)
00188122766i[      ] MEM ACCESS[0]: 0x000000000803cfe8 (linear) 0x00007df81fe8 (physical) RD SIZE: 4
00188122766i[      ] ----------------------------------------------------------
00188122766i[      ] CPU 3 at 0x80803ced: iret    (reg results):
00188122766i[      ] LEN 1	BYTES: cf
00188122766i[      ]   EAX: 0x00000086; EBX: 0x00000000; ECX 0x01001000; EDX: 0x00000000
00188122766i[      ]   ESP: 0x0803d000; EBP: 0x00000000; ESI 0x00000000; EDI: 0x00000000
00188122766i[      ]   CS: 0x001b; DS: 0x0000; ES: 0x0000; FS: 0x0000; GS: 0x0000; SS: 0x0023;
00188122766i[      ]   EFLAGS: 0x00003202 (id vip vif ac vm rf nt IOPL=3 of df IF tf sf zf af pf cf)
00188122766i[      ] 	BRANCH TARGET 0000000008048080 (TAKEN)
00188122766i[      ] MEM ACCESS[0]: 0x000000000803cff4 (linear) 0x00007df81ff4 (physical) RD SIZE: 4
00188122766i[      ] MEM ACCESS[1]: 0x000000000803cff0 (linear) 0x00007df81ff0 (physical) RD SIZE: 4
00188122766i[      ] MEM ACCESS[2]: 0x000000000803cfec (linear) 0x00007df81fec (physical) RD SIZE: 4
00188122766i[      ] MEM ACCESS[3]: 0x00000000ff410018 (linear) 0x000000010018 (physical) RD SIZE: 8
00188122766i[      ] MEM ACCESS[4]: 0x000000000803cffc (linear) 0x00007df81ffc (physical) RD SIZE: 2
00188122766i[      ] MEM ACCESS[5]: 0x00000000ff410020 (linear) 0x000000010020 (physical) RD SIZE: 8
00188122766i[      ] MEM ACCESS[6]: 0x000000000803cff8 (linear) 0x00007df81ff8 (physical) RD SIZE: 4
00188122766i[      ] MEM ACCESS[7]: 0x00000000ff41001d (linear) 0x00000001001d (physical) WR SIZE: 1
00188122766i[      ] MEM ACCESS[8]: 0x00000000ff410025 (linear) 0x000000010025 (physical) WR SIZE: 1
```

an `iret` is blanking out my segment selectors??  Well, shit:

>   FOR each SegReg in (ES, FS, GS, and DS)
>       DO
>           tempDesc ← descriptor cache for SegReg (* hidden part of segment register *)
>           IF (SegmentSelector == NULL) OR (tempDesc(DPL) < CPL AND tempDesc(Type) is (data or non-conforming code)))
>           THEN (* Segment register invalid *)
>               SegmentSelector ← 0; (*Segment selector becomes null*)
>           FI;
>       OD;

OK, so what to do about it???  So, the question is: can I change out the segment selectors before the `iret` and get the proper results?  The `DPL < CPL` condition will fail, so the data segment type will not matter any more.  I'm gonna give it a try -- what can it hurt?

OK, this really needs to happen once we are into user mode:

```
00188123246i[      ] CPU 3 at 0x80803cfc: or ax, 0x0003   (reg results):
00188123246i[      ] LEN 4	BYTES: 6683c803
00188123246i[      ]   EAX: 0x00000093; EBX: 0x00000000; ECX 0x01001000; EDX: 0x00000000
00188123246i[      ]   ESP: 0x0803cfec; EBP: 0x00000000; ESI 0x00000000; EDI: 0x00000000
00188123246i[      ]   CS: 0x0008; DS: 0x0023; ES: 0x0023; FS: 0x0023; GS: 0x0090; SS: 0x0010;
00188123246i[      ]   EFLAGS: 0x00000286 (id vip vif ac vm rf nt IOPL=0 of df IF tf SF zf af PF cf)
00188123246e[CPU3  ] load_seg_reg(GS, 0x0093): RPL & CPL must be <= DPL
00188123246i[      ] CPU 3: exception 0dh error_code=90
```

What is interesing though is that the *other* segment selectors are set properly.  And this change in `GS_GDT` should fix it:

```c
    .dpl = 3,                           \
```

and it did!!:

```
+---------------------------+--------+----------+----------+------------+-----------------------+--------------+
| Command                   | PID    | Priority | Status   | Address    | Time Used             | Kernel Stack |
+---------------------------+--------+----------+----------+------------+-----------------------+--------------+
| Butler                    | 0      | LOW      | RUNNING  | 0x9000000c | 0x00000000 0x008387e0 | 0xff805000   |
| Idle Process              | 1      | IDLE     | RUNNING  | 0x900000b8 | 0x00000000 0x003c41c8 | 0xff806000   |
| Idle Process              | 2      | IDLE     | READY    | 0x90000164 | 0x00000000 0x003cbec8 | 0xff808000   |
| Idle Process              | 3      | IDLE     | RUNNING  | 0x90000210 | 0x00000000 0x00319b38 | 0xff80a000   |
| Idle Process              | 4      | IDLE     | READY    | 0x900002bc | 0x00000000 0x00339708 | 0xff80c000   |
| kInitAp(1)                | 5      | OS       | TERM     | 0x900003ec | 0x00000000 0x00001388 | 0xff80e000   |
| kInitAp(2)                | 6      | OS       | TERM     | 0x90000498 | 0x00000000 0x00000bb8 | 0xff80f000   |
| kInitAp(3)                | 7      | OS       | TERM     | 0x90000544 | 0x00000000 0x00000bb8 | 0xff810000   |
| test.elf                  | 8      | NORMAL   | TERM     | 0x90000624 | 0x00000000 0x00000000 | 0x08035000   |
| Process A                 | 9      | OS       | DLYW     | 0x900007d8 | 0x00000000 0x00000fa0 | 0xff811000   |
| Process B                 | 10     | OS       | MSGW     | 0x90000884 | 0x00000000 0x000003e8 | 0xff813000   |
| Kernel Debugger           | 11     | OS       | RUNNING  | 0x90000930 | 0x00000000 0x006dfc40 | 0xff815000   |
+---------------------------+--------+----------+----------+------------+-----------------------+--------------+
```

Notice the status of `test.elf`!  I am going to commit this code now (lots of files have changes).  The rpi code has not been tested so it does not work.

OK, after uncommenting the butler cleanup, it is having a fit:

```
Page Fault scheduler,timer,msgq)
CPU: 2
EAX: 0xff40f000  EBX: 0x00000000  ECX: 0x00000000
EDX: 0xff40f000  ESI: 0xff40d000  EDI: 0x00000001
EBP: 0xff80d000  ESP: 0xff800efc  SS: 0x10
EIP: 0x80805537  EFLAGS: 0x00200092
CS: 0x8  DS: 0x28  ES: 0x28  FS: 0x28  GS: 0x78
CR0: 0xe0000011  CR2: 0xff40f000  CR3: 0x01001000
Trap: 0xe  Error: 0x2
```

And I'm betting it's related to the mappings....  as in the process mappings are not available for the kernel to unmap.  No, the problem is in `PmmPush()`.

---

### 2020-Apr-24

OK, the last step here it so clean up the Butler process to destroy a process's resources.  Right now, this cleanup functions is working with a heavy hand -- cleaning up everything whether it needs it or not.  Some things may not need it.  I believe this is why I am getting an attempt to free frame 0 -- I am trying to free frame 0 which is attempting to be mapped by the PMM.

Yup...

```
Starting to clean a process up
.. Removing the process from the terminated list
.. Cleaning up any referenced resources
.. Process Stack

!!! ASSERT FAILURE !!!
/home/adam/workspace/century-os/arch/x86/mmu/MmuMapToFrame.cc(46) frame
```

So the problem here is that I need to clean up a user process, but I am using the kernel mmu structure -- there is no access to the actual structure where the stack is mapped.  Really, I do not need to unmap the memory unless it is in kernel space....  I just need to free the frame.

This completes the setup for x86.  I am now able to launch a user-space process.  Now to debug the rpi2b.   This ought to be fun, since I do not have the ability to get execution logs like I do for x86!

