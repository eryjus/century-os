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

Again, let the faults begin!!!!!

```
Data Exception:
.. Data Fault Address: 0xff404000
.. Data Fault Status Register: 0x00000085
.. Fault status 0x5: Translation fault (First level)
.. Fault occurred because of a read
At address: 0xff800f20
 R0: 0x00008000   R1: 0x000001e7   R2: 0x00000000
 R3: 0xff404000   R4: 0x00000000   R5: 0x00008000
 R6: 0x00000000   R7: 0x000001e7   R8: 0x90000614
 R9: 0x3fd01000  R10: 0x00000001  R11: 0xff409000
R12: 0x00000000   SP: 0x80806470   LR_ret: 0x8080405c
SPSR_ret: 0x200001d3     type: 0x17

Additional Data Points:
User LR: 0xfffbd6bf  User SP: 0xffefead3
Svc LR: 0xff800f30
```

This first fault is in `MmuMapToFrame()`.  This makes me wonder if I properly have the user- and kernel- `TTBRn`s set properly.

So, I think I have a few things to work out here:
1. Get the `TTBR0` populated properly
1. Set the bits to properly split the `TTBRn` registers
1. Make sure `PmmAllocAlignedFrames()` will allocate aligned frames (likely not complete from the PMM rewrite)

So, for clarification:

> If N == 0 then use `TTBR0`. Setting `TTBCR.N` to zero disables use of a second set of translation tables.
> if N > 0 then:
> — if bits[31:32-N] of the input VA are all zero then use `TTBR0`
> — otherwise use `TTBR1`

So, either the `TTBR0` is used.  Or, if the `TTBCR` has been prepared, the `TTBR0` contains the user Virtual Address and the TTBR1 contains the kernel virtual address.  So, let's check what is set up.  A quick search indicates I am not setting this value.

Well, crap!  it looks like I have confused the `TTBCR` with the TTBR0 in `ProcessSwitch()` and in `MmuGetTopUserTables()`.  How the hell was this working!!??

For the record, the registers are:
* `TTBCR`: p15,0,r?,c2,c0,2
* `TTBR0`: p15,0,r?,c2,c0,0
* `TTBR1`: p15,0,R?,c2,c0,1

OK, I take it back.....  All 3 registers are being set properly in `entry` and `entryAp`.  The other functions are correct as well.

OK, now I am deadlocking (or infinite faulting).  This is most likely because of some bad memory mapping.

---

After spending some time thinking about this, I am not certain I have the armv7 user-mode tables properly implemented in code for mapping and unmapping.  I think it is all going in the kernel tables.  This will have to be my task for tomorrow.

---

### 2020-Apr-25

OK, checking in on armv7's `MmuMapToFrame()`, there is no code to handle a split between user-space tables and kernel-space tables.  So that will need to be cleaned up.

On the other hand, `PmmAllocAlignedFrames()` appears to have the proper code to handle the alignment.  I don't recall working on that, but I must have.

So, the problem with arm is that there is no cute recursive mapping trick that can be done -- it is all done by brute force.  This means that on a process change, I need to remap the TTL1 tables for maintenance (pages `0xff404000` and `0xff405000`) and the TTL2 tables for maintenance (for addresses from `0xffc00000` to `0xffdfffff`).  That's 2MB + 8KB that all need to be maintained on every task change.  That is *not* going to be efficient!  So, the question then becomes -- Should I create the system so that the user-space TTL2 tables are *only* mapped when we are about to do maintenance?  This would certainly reduce the overhead, but I would also need to disable interrupts during this maintenance.  It would be fussy for sure.  But, that may have to be the way to go.

This will be a complete rewrite of the MMU for arm.

So, I think I can map the user-TTL1 tables on process change (only 2 pages to map).  This should not create any performance problems.  However, when I need to map an actual page, I think I will need to map the proper TTL2 table page to a work frame.  Also, since the mapping is only temporary and will be specific to the CPU (1 page per for maintenance), so there is only need to flush the specific core when unmapping.  Having 1 address per CPU will reduce contention, but will also limit the number of CPUs that can be supported in the future....  I am planning on having this in the same space as the GDT for x86 (the requirements are mututally exclusive so there will be no conflict).  If need be, I can move this up to address `0xffc00000` where there is nothing.

I am struggling (again) to be able to articulate what I functions and addresses I need to maintain and what is mapped where.  This will likely need to be done on a piece of paper.

---

So, after some time with the paper, I am left wondering if I even need to go farther back in the design....  I had taken a frame and mapped all 4 TTL2 tables in that page.  Well, perhaps I need to map more on demand than I am at the moment.  More to the point, maybe when I need a TTL2 table, I map just 1K of the frame and wait for the next request to map the next one (likely somewhere else in memory).

I asked on `freenode#osdev`:

> [20:34:10] <eryjus> i uncovered a fatal flaw with mmu code for armv7.  It requires a total rewrite -- very little can be salvaged, so everything is on the table to change.  With that (and unrelated to the fatal flaw), I am wondering how the 1K TTL2 tables are usually implemented.  are they usually kept contiguous as a 4K block and mapped into 4 consecutive aligned TTL1 entries, or are they usually scattered around and leveraged as needed?  I'm
> [20:34:10] <eryjus> considering the latter and wondering how far out there my thinking is.
> [20:43:29] <geist> yeah that's what i'd do
> [20:43:39] <geist> if you're using 4K pages then that would be my suggestion
> [20:43:50] <eryjus> and I am
> [20:43:54] <geist> note with arm64 they went to a much more straightforward 4K/4K level
> [20:44:03] <geist> the old 1K pages tables was finally retired
> [20:44:17] <geist> also double check if you enabled LPAE if it doesn't switch to the 4K model
> [20:44:32] <geist> depending on which v7 you have it may have LPAE feature, which iirc looks a lot closer to arm64 model
> [20:44:57] <eryjus> its an rpi2b, and so I'm assuming not likely
> [20:45:10] <geist> dont assume, that's a cortex-a7. may have LPAE
> [20:45:35] <geist> yah cortex-a7 has LPAE
> [20:45:53] <geist> so then look in the manual and see what that looks like. may be smipler to implement that
> [20:46:12] <geist> iirc LPAE is much like how PAE is on x86: it's a 64bit physical 32bit virtual page table extension, but it also rearranges thigns a bit
> [20:47:45] <geist> i have never actually programmed LPAE on 32bit arm, but i think at the back of my mind i remember it being different enough, and then maybe the 64bit stuff just picked it up and went with it? i forget
> [20:48:53] <eryjus> yeah, I have never done PAE either, so not quite sure what I would be getting into.  I will have some reading to do for sure.
> [20:49:15] <eryjus> s/either/at all/
> [20:50:15] <geist> trouble is even if you treat 4 consequetive 1K pages you still have to contend with the top level 16KB page
> [20:50:18] <geist> which is annoying as heck
> [20:50:24] <geist> if you have a 4K PMM
> [20:52:18] <eryjus> that how i currently do it and the PMM was written to handle aligned allocations from the start; it wasn't quite that annoying

---

### 2020-Apr-26

So, with some additional reading last night before bed, I realize that LPAE is really still 32-bit addressing, but you have access fo 40-bits of physical memory to back that.  For some reason I thought that was backwards -- larger virtual space with smaller phyiscal, but then that makes no sense at all now that I am writing it down.

So, the top level has 4 entries in it only.  Moreover, in my setup, `TTBR0` hold entries `{0, 1}` whereas `TTBR1` hold entries `{2, 3}`.  On entry, both the `TTBR0` and `TTBR1` will be set to the same level 1 table, and all 4 level 2 tables will be allocated/initialized.  I will save that for a Butler initialization task to clean up the first 2 level 2 tables (and should be able to confirm that nothing remains mapped).

Now, there is this whole thing about mapping and unmapping addresses.  Instead of keeping everything mapped (like I do for x86), I will have a block of dedicated addresses for each CPU.  These addresses will be used for temporarily mapping each table for maintenance.  This means there will be some timing and contention among processes and task swaps and the addresses will have to be protected by disabling interrupts (since there is 1 block per CPU, there is no need for a spinlock).  At the moment, I think I can get all my management into a single address, so we are talking about a block of 1 address.  To make sure I do not run out, I will dedicate 4 pages per CPU.

So....
* Addresses `0xffc00000` - `0xffc0000c` will be used for CPU0
* Addresses `0xffc00010` - `0xffc0001c` will be used for CPU1
* Addresses `0xffc00020` - `0xffc0002c` will be used for CPU2
* Addresses `0xffc00030` - `0xffc0003c` will be used for CPU3

That said, I currently support only 4 CPUs, so this is reasonable.

Finally, there has to be something I need to do to get the MMU into lpae mode....  I think that is in the `TTBCR`....  `TTBCR.EAE` is the bit to enable the lpae.

The first task here is to break everything.

I also need to set the MAIR values that will be used for my memory.  There are 2 registers which need values: `MAIR0` and `MAIR1`.  Right now, I only see the need for 2 types of memory: Device and Normal.  There are 8 buckets, so this is what I am thinking:

* `Attr0`: value `0000 0000` for device memory
* `Attr1`: value `0100 1011` for Normal memory (Normal memory, Outer Non-cacheable; Normal memory, Inner Write-Through Cacheable, Non-transient)
* `Attr2`: Unused value `0000 0000`
* `Attr3`: Unused value `0000 0000`
* `Attr4`: Unused value `0000 0000`
* `Attr5`: Unused value `0000 0000`
* `Attr6`: Unused value `0000 0000`
* `Attr7`: Unused value `0000 0000`

This, then, means that `MAIR0` is written with a value of `0x00004b00` and `MAIR1` is written with a value of `0x00000000`.  Those changes were 1-time setup which was easy to add to the `entry` and `entryAp` sources.

The next step is to map all the minimal structures in the `entry` source.  This will be a *major* overhaul of this source file.

---

### 2020-Apr-27

Today I was able to start by getting the `entry` code all cleaned up.  `entryAp` should be good as well.  I also have a couple of additional functions I was able to move back into place.

So, I can see 2 major functions I am missing: `MmuMapToFrame()` and `MmuUnmapPage()`.  I know there is at least one additional function, but let's get the big dogs out of the way.

Well, shit.  As I get into the work for any of the actual mapping, in order to map a page I need a page mapped where I can change these table entries.  In order words, I need to map a table into an address before I can map a table into an address.

So, maybe a little bit of rethinking is required.  Well, no 'maybe'.

---

OK, so I need to have a page mapped to the level 3 table for `0xffc00000`.  This page will be updated directly by the MMU functions, since this will be where the actual temporary mappings will take place.  I think I will use address `0xfffff000` for this purpose.  This also means I need to map that in `entry`.

---

### 2020-Apr-28

I'm not quite feeling this today....  Too many issues at work.

But, I am going to try to get the `MmuVirtToPhys()` function rewritten.  Let's see how I do....  Not gonna happen.  There is too much thinking required and I just do not have it in me today.

The problem is that I am mapping mappings and I cannot keep them all straight today.

---

OK, so if I want to use address `0xffc00000` to manage the MMU tables, with the intent to map some frame with some structure therein.  That address is at level 1 index `3`, level 2 index `0x1fe`, and level 3 index `0`.  Therefore, I need to have the table for level 3 (level 2 index `0x1fe`) mapped to `0xfffff000`.  At that point, all I will need to do is update the mapping at `((uint64_t *)0xfffff000)[0]` and invalidate the page for `0xffc00000` and do the maintenance.

I am saving the frame for address `0xffc00000` level 2 table in the variable `mmuMgmtTable` and also mapping that to `0xfffff000`, so that is not required.  The only thing that is necessary is to to the update to entry `0` and flush the TLB.

In writing this, I have decided I need a more generic `MmuGetTopTable()` function that is responsible on the arm to determine what that correct value should be.  Now for that to work, I will need to be able to pass in an address for that table.  That piece I will work on tormorrow.  So far, the only thing that will have problems with that logic will be `ProcessPrepareFromImage()`, which should be able to work with a user-space address -- the kernel address will not need to change.

---

### 2020-Apr-30

Well, I wasn't feeling it yesterday again.  I have a project going on for my day job that is taking a lot of my concentration and troubleshooting allotment for the day....

---

### 2020-May-01

I was able to get through a good compile last night.  There is a ton of repitition in the code.  I hace not executed this code yet.  So that will be the first order of business today, but I also need to stop before I get too deep into the code and chech my work.  The first milestone is going to be to get to `LoaderMain()`.  And, of course, nothing.  See, the thing is I had all this debugging code already built into the arm `entry.s` and I removed it rather than retrofit it.

So, I have to now go put it back in.

So, with some simple character output, I have the following output:

```
Booting...
ABC.
```

So, I get through the first `MakePageTable` call and set the global variable.  Then I get to the next `MakePageTable` call and it gets lost.  Hmmm.... am I clobbering state by overwriting `r0`?

With this code:

```S
    push    {r0}
    mov     r0,#'G'
    bl      OutputChar
    pop     {r0}

    str     r1,[r0,r3]                          @@ load the upper record bits

    push    {r0}
    mov     r0,#'H'
    bl      OutputChar
    pop     {r0}
```

I have been able to narrow this down:

```
Sending the Entry point as 100060
Waiting for the rpi to boot
Booting...
ABC....DEFG
```

to this instruction:

```S
    str     r1,[r0,r3]                          @@ load the upper record bits
```

So, what the hell is really going on here??  This looks right:


```s
1002ac:       e7801003        str     r1, [r0, r3]
```

But it's not.  `r0` is supposed to contain the value to load.  I gotta look at this again.  And I found it:

```s
    str     r1,[r5,r3]                          @@ load the upper record bits

    add     r3,#4                               @@ move to the next word
    str     r0,[r5,r3]                          @@ load the lower record bits
```

Notice the change of `r0` into `r5`...  This code was copied all over the place, so it's all suspect.

With that change, I am able to get all the way to the point where I am about to enable paging.  So the next step here is to be able to dump some tables.

```
Booting...
ABC....DEFGHIY
```

If I were getting paging properly enabled, I would also see `'Z'` printed.

---

### 2020-May-02

I need a function to dump the tables for an address, before we turn paging on.  This will be in 100% assemply, of course.

---

Haha!!!  With this code:

```s
@@ -- Print some debugging information
    push    {r0}
    ldr     r0,=.paging
    bl      OutputAddrTables
    pop     {r0}



@@ -- now we enable paging
    mrc     p15,0,r1,c1,c0,0                    @@ This gets the cp15 register 1 and puts it in r0
    orr     r1,#1                               @@ set bit 0
    mcr     p15,0,r1,c1,c0,0                    @@ Put the cp15 register 1 back, with the MMU enabled
.paging:
```

I am getting the following output:

```
Booting...
ABC....DEFGHIY

Before Paging Dump MMU Tables: Walking the page tables for address 0x001007bc

Level  Tabl-Addr     Index        Entry Addr    Next PAddr    fault
-----  ----------    ----------   ----------    ----------    ----------
  1    0x01002000    0x00000000   0x01002000    0x01003000    0x00000003
  2    0x01003000    0x00000000   0x01003000    0x01008000    0x00000003
  3    0x01008000    0x00000100   0x01008400    0x00000000    0x00000000
```

So, the page is really not mapped (or not mapped properly).

I now know what is wrong...  so, why is it wrong?  It turns out my calculations were wrong:

```s
@@ -- now we can get into the process of mapping the kernel; starting with the multiboot code/data at 1MB...
@@    Again, this can be statically calculated:
@@    level 1 index: 0x00100000 >> 30 or 0x00 (so use the table in r6)
@@    level 2 index: 0x00100000 >> 21 & 0x1ff or index 0x000 or offset 0x000 (so we want to update address [r6,#4])
@@    level 3 index: 0x00100000 >> 12 & 0x1ff or index 0x100 or offset 0x800 (so we want to update addr [r5,#0x800])
```

That offset should be `0x400`!

ok, so now:

```
Booting...


Pre-Paging MMU Tables Dump: Walking the page tables for address 0x00100714

Level  Tabl-Addr     Index        Entry Addr    Next PAddr    Attr Bits
-----  ----------    ----------   ----------    ----------    ---------------------
  1    0x01002000    0x00000000   0x01002000    0x01003000    0x00000000 0x00000003
  2    0x01003000    0x00000000   0x01003000    0x01008000    0x00000000 0x00000003
  3    0x01008000    0x00000100   0x01008400    0x00100000    0x00000000 0x00000027
```

Are the bits set correctly at level 3?  Not quite sure...   We have:
* `present`
* `pageFlag`
* `attrIndex` == `0b001`
* `ns`
* and the rest are set to `0`.  How were these flags set before?  Well, an actual mapping looked like this:

```c++
    ttl2Entry->frame = frame;
    ttl2Entry->s = ARMV7_SHARABLE_TRUE;
    ttl2Entry->apx = ARMV7_MMU_APX_FULL_ACCESS;
    ttl2Entry->ap = ARMV7_MMU_AP_FULL_ACCESS;
    ttl2Entry->tex = (flags&PG_DEVICE?ARMV7_MMU_TEX_DEVICE:ARMV7_MMU_TEX_NORMAL);
    ttl2Entry->c = (flags&PG_DEVICE?ARMV7_MMU_UNCACHED:ARMV7_MMU_CACHED);
    ttl2Entry->b = (flags&PG_DEVICE?ARMV7_MMU_UNBUFFERED:ARMV7_MMU_BUFFERED);
    ttl2Entry->nG = ARMV7_MMU_GLOBAL;
    ttl2Entry->fault = ARMV7_MMU_CODE_PAGE;
```

I know I am missing setting the `LongPageDescriptor_t.sh` to `0b11`.  Also, I believe I should be setting `LongPageDescriptor_t.ap` to `0b01`.

I believe that these bits should also be set for the page mapping.  It turns out that my bit handling was incorrect, as bits 8-11 were shifted over bits 0-3, so I was just `orr`ing the same bits.

That fixed, I am now getting all the bits set properly.  But I am still not getting my `'Z'` char to print.

It also looks like I had my `MAIR` nibbles backwards....  But that did not help either.

Hmmm....

```S
@@ -- now we enable paging
    mrc     p15,0,r1,c1,c0,0                    @@ This gets the cp15 register 1 and puts it in r0
    orr     r1,#1                               @@ set bit 0
    mcr     p15,0,r1,c1,c0,0                    @@ Put the cp15 register 1 back, with the MMU enabled
.paging:


    push    {r0}
    mov     r0,#'Z'
    bl      OutputChar
    pop     {r0}
```

Maybe I am getting there and the stack is not set right, so I am faulting on the `push` to the stack.

Well, while that was true, it was not *the* problem.  I think I am going to have to walk away for a bit and maybe resort to QEMU for some debugging.

---

```
R00=00000327 R01=ffffffff R02=00000000 R03=00000ffc
R04=0100f000 R05=0100e000 R06=01003000 R07=01004000
R08=01005000 R09=01006000 R10=0002830f R11=000254cc
R12=00000264 R13=01000ffc R14=00100100 R15=00100708
PSR=200001d3 --C- A NS svc32
Taking exception 3 [Prefetch Abort]
...from EL1 to EL1
...with ESR 0x21/0x86000005
...with IFSR 0x205 IFAR 0x100718
```

So, this tells me that I am genuinely faulting on the instruction page mapping.  Based on my prior analysis, I believe it to be mapped properly.

Let's pick apart the `IFSR`.   But `0b10101` (or before adjusting the bits, `0x205`) is not defined by the ARM ARM.

OK, I was looking at the wrong place....

So, the `2` in `0x205` indicates that I am in lpae mode.  This is good.  So what is left is the `0x5` part of the status register.

Bits 5:2 indicates that this is a Translation Fault and bit 1:0 show that this is at Level 1.  So, the Level 1 entry is not mapped.  But my output disagrees.

Wait a minute!!  Could this be related to getting to the Level 1 table?  Probably!

And in fact, with lpae enabled, the `TTBR0` and `TTBR1` registers become 64-bit (and the address I loaded into the 32-bit register is probably banked away)....

---

OK, I am wondering if the root of this is related to the `TTBCR` register....Currently, I am only setting bits 31 and 0.  Coult my problems be related to the `T0SZ` and `T1SZ` fields in the `TTBCR`???

---

### 2020-May-03

OK, so let's map out how the `TTBCR` should look:

|  Bits  |  Name  |  Desired Value |
|:------:|:------:|:---------------|
|   31   | EAE    | 1: Extended Address Enable |
|   30   | ---    | 0: Impl Defnied |
| 28-29  | SH1    | 11: Inner Sharable for `TTBR1` |
| 26-27  | ORGN1  | 01: Outer Write-back Wite-allocate |
| 24-25  | IRGN1  | 01: Inner Write-back Wite-allocate |
|   23   | EPD1   | 0: Translation Table Walk Disable |
|   22   | A1     | 0: ASID encoded in `TTBR0` |
| 19-21  | ---    | 000: Ignored |
| 16-18  | T1SZ   | 001: 2^31 sized memory region for `TTBR1` |
| 14-15  | ---    | 00: Ignored |
| 12-13  | SH0    | 11: Inner Sharable for `TTBR0` |
| 10-11  | ORGN0  | 01: Outer Write-back Wite-allocate |
|  8-9   | IRGN0  | 01: Inner Write-back Wite-allocate |
|   7    | EPD0   | 0: Translation Table Walk Disable |
|  3-6   | ---    | 0000: Ignored |
|  0-2   | T0SZ   | 001: 2^31 size memory region for `TTBR0` |

So, with this table, the bits to set are: `0b1011 0101 0000 0001 0011 0101 0000 0001`, or `0xb5013501`.

Let's give that a try.

---

OK, maybe it is working and I'm trying to print something to the serial port when that address is no longer mapped....  Nope -- still nothing...

So, let me see if I can articulate the problem for a gist before I ask for help:

---

I am getting a Pre-fetch Abort on the next instruction after I enable paging.  This pre-fetch abort reports a Level 1 Translation error for address `0x1007ac`.  That address is the instruction immediately after enabling paging.

The pre-fetch abort looks like this:

```
IN:
0x0010079c:  e49d0004  pop      {r0}
0x001007a0:  ee111f10  mrc      p15, #0, r1, c1, c0, #0
0x001007a4:  e3811001  orr      r1, r1, #1
0x001007a8:  ee011f10  mcr      p15, #0, r1, c1, c0, #0

R00=00000327 R01=ffffffff R02=00000000 R03=00000ffc
R04=0100f000 R05=0100e000 R06=01003000 R07=01004000
R08=01005000 R09=01006000 R10=0002830f R11=000254cc
R12=00000264 R13=01000ffc R14=00100100 R15=0010079c
PSR=200001d3 --C- A NS svc32
Taking exception 3 [Prefetch Abort]
...from EL1 to EL1
...with ESR 0x21/0x86000005
...with IFSR 0x205 IFAR 0x1007ac
```

The value `0x205` in the `IFSR` indicates that:
* LPAE is indeed enabled
* This is a translation fault
* The level of the translation fault is 1

I am dumping critical tables and entries just before enabling paging, with the following results:

```
Pre-Paging MMU Tables Dump: Walking the page tables for address 0x001007b8

.. TTBR0 entry is 0x00000000 0x01002000
.. TTBR1 entry is 0x00000000 0x01002000
.. TTBCR is 0xb5013501

Level  Tabl-Addr     Index        Entry Addr    Next PAddr    Attr Bits
-----  ----------    ----------   ----------    ----------    ---------------------
  1    0x01002000    0x00000000   0x01002000    0x01003000    0x00000000 0x00000003
  2    0x01003000    0x00000000   0x01003000    0x01008000    0x00000000 0x00000003
  3    0x01008000    0x00000100   0x01008400    0x00100000    0x00000000 0x00000327
```

The TTBCR bits have the following meanings:

|  Bits  |  Name  |  Desired Value |  Description  |
|:------:|:------:|:--------------:|:--------------|
|   31   | EAE    | 1 | Extended Address Enable |
|   30   | ---    | 0 | Impl Defnied |
| 28-29  | SH1    | 11 | Inner Sharable for `TTBR1` |
| 26-27  | ORGN1  | 01 | Outer Write-back Wite-allocate |
| 24-25  | IRGN1  | 01 | Inner Write-back Wite-allocate |
|   23   | EPD1   | 0 | Translation Table Walk Disable |
|   22   | A1     | 0 | ASID encoded in `TTBR0` |
| 19-21  | ---    | 000 | Ignored |
| 16-18  | T1SZ   | 001 | 2^31 sized memory region for `TTBR1` |
| 14-15  | ---    | 00 | Ignored |
| 12-13  | SH0    | 11 | Inner Sharable for `TTBR0` |
| 10-11  | ORGN0  | 01 | Outer Write-back Wite-allocate |
|  8-9   | IRGN0  | 01 | Inner Write-back Wite-allocate |
|   7    | EPD0   | 0 | Translation Table Walk Disable |
|  3-6   | ---    | 0000 | Ignored |
|  0-2   | T0SZ   | 001 | 2^31 size memory region for `TTBR0` |

This all looks reasonable to me, but I am still unable to determine a cause.

For the moment, I am only making use of `MAIR` entries 0 and 1, so the value of the MAIR0 gets set to `0x0000bb00`.

MAIR1 has the following meaning:
* `0b1011` -- Normal memory, Outer Write-Through Cacheable, Non-transient, Outer Read-Allocate, Outer Write-Allocate.
* `0b1011` -- Normal memory, Inner Write-Through Cacheable, Non-transient, Inner Read-Allocate, Inner Write-Allocate.

MAIR0 has the following meaning:
* `0b0000` -- Strongly-ordered or Device memory.
* `0b0000` -- Strongly-ordered memory.

I am not making use of `ASID`, so those values are 0.

---

Hmmm....

> For a Translation fault, the lookup level of the translation table that gave the fault. If a fault occurs because an MMU is disabled, or because the input address is outside the range specified by the appropriate base address register or registers, the fault is reported as a First level fault.

Am I setting this separation properly????

Hmmm....  section B3.6.4 of the ARM ARM may hold a clue:

| T0SZ  | T1SZ  | TTBR0 | TTBR1 |
|:-----:|:-----:|:-----:|:-----:|
| 0b000 | 0b000 | All addresses | Not used |
| M     | 0b000 | Zero to (2 ^ (32-M) - 1) | 2 ^ 32-M to maximum input address |
| 0b000 | N     | Zero to (2 ^ 32 - 2 ^ (32-N) - 1) | 2 ^ 32-2 ^ (32-N) to maximum input address |
| M     | N     | Zero to (2 ^ (32-M) - 1) | 2 ^ 32 - 2 ^ (32-N) to maximum input address |

---

### 2020-May-04

May the fourth be with you!

So, I am getting a little farther along here.  I am seeing that the low 32-bits of the 64-bit sturcture are actually in the low 4 bytes of memory, not the top 4 bytes of memory.  It was out of sheer depsiarion that I tried flipping them and I got farther.

So, this conversation covers the problem pretty clearly:

> [15:57:06] <eryjus> ok, what the actual @&^%!&$@
> [15:57:54] <eryjus> the long descriptors for lpae are not 64-bit structures but 2 married 32-bit structures, where the low 32-bits are in the low addresses.
> [15:58:11] <eryjus> i cannot find anywhere in the documentation where it states that fact.
> [16:03:56] <geist> well, it might just be kinda implicit in how it's written out
> [16:05:28] <geist> hmm, i dont see that. the v7 manual pretty clearly just points it out as a 64bit thing
> [16:05:39] <geist> but what you described actually sounds exactly like a little endian 64bit value
> [16:05:49] <eryjus> The TTBR0 and R1 registers are the only explicit thing I could find
> [16:06:13] <geist> oh i'm looking at ir gith here. page B3-1340 in the armv7 manual
> [16:06:18] <geist> ARM DDI 0406C.c
> [16:06:35] <eryjus> where the mrcc instruction is explicit that it is counter-intuitive...
> [16:06:44] <geist> B3.6 is all about the Long-descriptor translation table format
> [16:06:57] <geist> well, mrcc is a different thing all togetierh
> [16:07:04] <geist> what you were describing was the translation table structures
> [16:07:11] <eryjus> that is true
> [16:07:15] <geist> and i see no reason to believe what you said is true at all
> [16:07:28] <eryjus> so, I was expecting a 64-bit little endian structure
> [16:07:39] <geist> and that sounds right
> [16:07:43] <eryjus> not 2 X 32-bit little endian structures
> [16:07:52] <geist> how are the two different?
> [16:07:57] <eryjus> so my lowest 8 bits are in offset 4
> [16:08:12] <geist> thats simply not right
> [16:08:13] <eryjus> sorry offset3
> [16:08:18] <eryjus> then 2, 1, 0, 7!
> [16:08:23] <geist> incorrect
> [16:09:08] <eryjus> well...  you are expecting what I am expecting then...  i flipped them out of desperation and got better results
> [16:09:19] <geist> well, then you probably have something else wrong
> [16:09:26] <geist> and you're correcting it
> [16:09:32] <geist> by undoing what you did wrong in the first place
> [16:10:03] <geist> i'm lookin right at the manual and it clearly describes it as a 64bit value, with even bit numbers
> [16:10:13] <eryjus> it's possible im too close to it, and I will triple triple quadruple check....
> [16:10:43] <geist> and sure enough that's the same format as arm64. so LPAE is very much like x86 PAE
> [16:10:43] <eryjus> i yeah, its up on my screen too
> [16:13:35] <eryjus> geist, this works: 1002000:       01003003;     1002004:       00000000
> [16:13:48] <eryjus> the low 32-bits that contain the frame are in the low 4 bytes.
> [16:14:15] <eryjus> thus, the expletive
> [16:23:52] <geist> yeah what's wrong with that?
> [16:23:58] <geist> that's precisely little endian
> [16:24:34] <geist> 03 30 00 01 00 00 00 00
> [16:25:12] * eryjus was expecting the values to be swapped so 0x01003003 at address 0x01002004
> [16:26:04] <geist> nope. think of little endian as streaming from lower bytes to higher
> [16:26:19] <eryjus> yeah...  getting my head around that...
> [16:26:26] <geist> depending on how you visualize it it can be confusing, but if you go back to that thinking it is clear
> [16:26:46] <geist> your printf showing the two 32bit values is also little endian swapping within it, so you're getting a half swizzle
> [16:27:00] <geist> if you print it as a bunch of 8 bit values i think it'll be more obvious
> [16:27:52] <eryjus> well, now that I understand my flaw....
> [16:27:59] <eryjus> may the fourth be with you!

So, I need to redo my table print a little bit, since I need to be able to print this is a better order to be clear about it.

This is something I need to get my head around better.....  When I write the 32-bit values the perspective is different than when I dump the values.  I need to be clear about each.

---

OK, so let's recap a bit here....
* I had the 64-bit entry backward by 32-bit word.
* I was multiplying by 4 to adjust an index to offset, rather than multiplying by 8.
* I was not setting the access flag.  This should be hardware managed, not software managed.  [`SCTLR.HA` needs to be set to allow hardware-managed access flags](http://eryjus.ddns.net:3000/issues/476).

That said, I am now getting past into the loader (`0x80000000`) before faulting.

OK, I am finally getting somewhere.  But I'm exhausted.  I'm off to bed and I will continue tomorrow.

---

### 2020-May-05

So, it looks like the stack has some mapping problem:

```
IN:
0x800007b0:  e92d4010  push     {r4, lr}
0x800007b4:  ebffffeb  bl       #0x80000768

R00=01002000 R01=00c5187d R02=00000000 R03=00000028
R04=0100f000 R05=0100e000 R06=01003000 R07=01004000
R08=01005000 R09=01006000 R10=0002830f R11=000254cc
R12=00000264 R13=ff801000 R14=001001b4 R15=800007b0
PSR=200001d3 --C- A NS svc32
Taking exception 4 [Data Abort]
...from EL1 to EL1
...with ESR 0x25/0x96000047
...with DFSR 0xa07 DFAR 0xff800ff8
```

Aith a `DFSR` of `0xa07`, this means the following:

* A Level 3 translation fault
* LPAE is enabled
* The problem occurred on a write

Ahhhh...  backwards still:

```
Pre-Paging MMU Tables Dump: Walking the page tables for address 0xff80 0000

.. TTBR0 entry is 0x0000 0000 0100 2000
.. TTBR1 entry is 0x0000 0000 0100 2000
.. TTBCR is 0xb500 3501

Level  Tabl-Addr      Index         Entry Addr     Next PAddr     Attr Bits
-----  -----------    -----------   -----------    -----------    ---------------------
  1    0x0100 2000    0x0000 0003   0x0100 2018    0x0100 6000    0x0000 0000 0000 0003
  2    0x0100 6000    0x0000 01fc   0x0100 6fe0    0x0100 d000    0x0000 0000 0000 0003
  3    0x0100 d000    0x0000 0000   0x0100 d000    0x0000 0000    0x0100 0747 0000 0000
```

Still getting a data fault, but.... from the execution log:

```s
0x800007f8:  e34f6904  movt     r6, #0xf904
0x800007fc:  eb200df8  bl       #0x80803fe4
```

That branch/link instruction should land here:

```s
80803fe4 <MmuMapToFrame>:
80803fe4:       e3500000        cmp     r0, #0
80803fe8:       13510000        cmpne   r1, #0
80803fec:       012fff1e        bxeq    lr
80803ff0:       e92d47f0        push    {r4, r5, r6, r7, r8, r9, sl, lr}
```

But instead, the execution log shows:

```s
0x80803fe4:  e3091030  movw     r1, #0x9030
0x80803fe8:  e3000768  movw     r0, #0x768
0x80803fec:  e3481080  movt     r1, #0x8080
0x80803ff0:  e3480100  movt     r0, #0x8100
0x80803ff4:  eb001c3c  bl       #0x8080b0ec
```

That's not right, and the page is mapped to the wrong physical frame.

And in fact, I find this code here in the binary:

```s
80800fe4:       e3091030        movw    r1, #36912      ; 0x9030
80800fe8:       e3000768        movw    r0, #1896       ; 0x768
80800fec:       e3481080        movt    r1, #32896      ; 0x8080
80800ff0:       e3480100        movt    r0, #33024      ; 0x8100
80800ff4:       eb001c3c        bl      808080ec <kStrCmp>
```

So, I am off by 3 frames.  Why?  To check a theory, I added the following:

```s
    mov     r0,#0
    movt    r0,#0x8080
    bl      OutputAddrTables

    mov     r0,#0x1000
    movt    r0,#0x8080
    bl      OutputAddrTables

    mov     r0,#0x2000
    movt    r0,#0x8080
    bl      OutputAddrTables

    mov     r0,#0x3000
    movt    r0,#0x8080
    bl      OutputAddrTables
```

and it is confirmed....

```
Pre-Paging MMU Tables Dump: Walking the page tables for address 0xff80 0000

.. TTBR0 entry is 0x0000 0000 0100 2000
.. TTBR1 entry is 0x0000 0000 0100 2000
.. TTBCR is 0xb500 3501

Level  Tabl-Addr      Index         Entry Addr     Next PAddr     Attr Bits
-----  -----------    -----------   -----------    -----------    ---------------------
  1    0x0100 2000    0x0000 0003   0x0100 2018    0x0100 6000    0x0000 0000 0000 0003
  2    0x0100 6000    0x0000 01fc   0x0100 6fe0    0x0100 d000    0x0000 0000 0000 0003
  3    0x0100 d000    0x0000 0000   0x0100 d000    0x0100 0000    0x0000 0000 0000 0747


Pre-Paging MMU Tables Dump: Walking the page tables for address 0x8100 0000

.. TTBR0 entry is 0x0000 0000 0100 2000
.. TTBR1 entry is 0x0000 0000 0100 2000
.. TTBCR is 0xb500 3501

Level  Tabl-Addr      Index         Entry Addr     Next PAddr     Attr Bits
-----  -----------    -----------   -----------    -----------    ---------------------
  1    0x0100 2000    0x0000 0002   0x0100 2010    0x0100 5000    0x0000 0000 0000 0003
  2    0x0100 5000    0x0000 0008   0x0100 5040    0x0100 c000    0x0000 0000 0000 0003
  3    0x0100 c000    0x0000 0000   0x0100 c000    0x0011 2000    0x0000 0000 0000 0747


Pre-Paging MMU Tables Dump: Walking the page tables for address 0x8080 0000

.. TTBR0 entry is 0x0000 0000 0100 2000
.. TTBR1 entry is 0x0000 0000 0100 2000
.. TTBCR is 0xb500 3501

Level  Tabl-Addr      Index         Entry Addr     Next PAddr     Attr Bits
-----  -----------    -----------   -----------    -----------    ---------------------
  1    0x0100 2000    0x0000 0002   0x0100 2010    0x0100 5000    0x0000 0000 0000 0003
  2    0x0100 5000    0x0000 0004   0x0100 5020    0x0100 b000    0x0000 0000 0000 0003
  3    0x0100 b000    0x0000 0000   0x0100 b000    0x0010 5000    0x0000 0000 0000 0747


Pre-Paging MMU Tables Dump: Walking the page tables for address 0x8080 1000

.. TTBR0 entry is 0x0000 0000 0100 2000
.. TTBR1 entry is 0x0000 0000 0100 2000
.. TTBCR is 0xb500 3501

Level  Tabl-Addr      Index         Entry Addr     Next PAddr     Attr Bits
-----  -----------    -----------   -----------    -----------    ---------------------
  1    0x0100 2000    0x0000 0002   0x0100 2010    0x0100 5000    0x0000 0000 0000 0003
  2    0x0100 5000    0x0000 0004   0x0100 5020    0x0100 b000    0x0000 0000 0000 0003
  3    0x0100 b000    0x0000 0001   0x0100 b008    0x0010 5000    0x0000 0000 0000 0747


Pre-Paging MMU Tables Dump: Walking the page tables for address 0x8080 2000

.. TTBR0 entry is 0x0000 0000 0100 2000
.. TTBR1 entry is 0x0000 0000 0100 2000
.. TTBCR is 0xb500 3501

Level  Tabl-Addr      Index         Entry Addr     Next PAddr     Attr Bits
-----  -----------    -----------   -----------    -----------    ---------------------
  1    0x0100 2000    0x0000 0002   0x0100 2010    0x0100 5000    0x0000 0000 0000 0003
  2    0x0100 5000    0x0000 0004   0x0100 5020    0x0100 b000    0x0000 0000 0000 0003
  3    0x0100 b000    0x0000 0002   0x0100 b010    0x0010 5000    0x0000 0000 0000 0747


Pre-Paging MMU Tables Dump: Walking the page tables for address 0x8080 3000

.. TTBR0 entry is 0x0000 0000 0100 2000
.. TTBR1 entry is 0x0000 0000 0100 2000
.. TTBCR is 0xb500 3501

Level  Tabl-Addr      Index         Entry Addr     Next PAddr     Attr Bits
-----  -----------    -----------   -----------    -----------    ---------------------
  1    0x0100 2000    0x0000 0002   0x0100 2010    0x0100 5000    0x0000 0000 0000 0003
  2    0x0100 5000    0x0000 0004   0x0100 5020    0x0100 b000    0x0000 0000 0000 0003
  3    0x0100 b000    0x0000 0003   0x0100 b018    0x0010 5000    0x0000 0000 0000 0747
```

Notice that all 4 pages map to the same frame.  A quick change cleaned that up, but I am still faulting with something.  Back to QEMU....

Hmmm....

```
IN:
0x80804000:  e1a0af26  lsr      sl, r6, #0x1e
0x80804004:  e1a05000  mov      r5, r0
0x80804008:  ee1d3f90  mrc      p15, #0, r3, c13, c0, #4
0x8080400c:  e5934000  ldr      r4, [r3]
0x80804010:  e7e84454  ubfx     r4, r4, #8, #9
0x80804014:  ee1d3f90  mrc      p15, #0, r3, c13, c0, #4
0x80804018:  e5939000  ldr      sb, [r3]
0x8080401c:  e3560000  cmp      r6, #0
0x80804020:  e1a09209  lsl      sb, sb, #4
0x80804024:  e2499501  sub      sb, sb, #0x400000
0x80804028:  ba000062  blt      #0x808041b8

R00=000000c0 R01=01001000 R02=00000003 R03=00100024
R04=f8000000 R05=0003f000 R06=ff401000 R07=01004000
R08=01001000 R09=01006000 R10=0002830f R11=000254cc
R12=00000264 R13=ff800fc8 R14=80804000 R15=80804000
PSR=200001d3 --C- A NS svc32
Taking exception 4 [Data Abort]
...from EL1 to EL1
...with ESR 0x25/0x96000007
...with DFSR 0x207 DFAR 0x0
```

Could this be a straight-up NULL pointer dereference?  It certainly could be: this exception starts from `MmuMapToFrame()`.  However, I think I am more concerned with the fact I am not getting any fault dumps.  And, in fact, that fault looks like this:

```
Taking exception 3 [Prefetch Abort]
...from EL1 to EL1
...with ESR 0x21/0x86000006
...with IFSR 0x206 IFAR 0x1001010
```

So, I expect that 'exception 4' refers to offset `0x10` into the VBAR.  I was mapping the wrong address into the MMU, and I was setting the VBAR incorrectly.  Now, I am getting a fault and the handler is taking it, but there is no output.  This is because the MMIO addresses are not mapped yet.

It turns out that the first call is from `MmuInit()`.  Now, that is the VBAR mapping, which I now have taken care of in `entry.s`.

OK, this fault is because the `TPIDRPRW` register is not populated when I am executing `MmuInit()`.  A quick check for a NULL address fixed that.  But now, address `0xfffff000` is not mapped.

```
IN:
0x808040ac:  e3800003  orr      r0, r0, #3
0x808040b0:  e3a03000  mov      r3, #0
0x808040b4:  e8840009  stm      r4, {r0, r3}
0x808040b8:  f57ff05b  dmb      ish
0x808040bc:  ee08af77  mcr      p15, #0, sl, c8, c7, #3

R00=01002000 R01=0003f000 R02=80000003 R03=00000000
R04=fffff000 R05=00000003 R06=f8000000 R07=ffc00000
R08=000000c0 R09=0003f000 R10=ffc00000 R11=000254cc
R12=00000264 R13=ff800fc8 R14=80804234 R15=808040ac
PSR=a00001d3 N-C- A NS svc32
Taking exception 4 [Data Abort]
...from EL1 to EL1
...with ESR 0x25/0x96000047
...with DFSR 0xa07 DFAR 0xfffff000
```

This needs to be handled in `entry.s`.  And this is where it gets complicated since this is a double mapping.  So, what I need is the physical address for the level 2 table for address `0xffc00000`.  I think I have that already....  I do, and I should have already taken care of this mapping.  So, I need to dump the tables for that address, as well as for `0xffc00000`.

So, the physical address I am looking for is `0x0f00e000`:

```
Pre-Paging MMU Tables Dump: Walking the page tables for address 0xffc0 0000

.. TTBR0 entry is 0x0000 0000 0100 2000
.. TTBR1 entry is 0x0000 0000 0100 2000
.. TTBCR is 0xb500 3501

Level  Tabl-Addr      Index         Entry Addr     Next PAddr     Attr Bits
-----  -----------    -----------   -----------    -----------    ---------------------
  1    0x0100 2000    0x0000 0003   0x0100 2018    0x0100 6000    0x0000 0000 0000 0003
  2    0x0100 6000    0x0000 01fe   0x0100 6ff0    0x0100 e000    0x0000 0000 0000 0003
  3    0x0100 e000    0x0000 0000   0x0100 e000    0x0000 0000    0x0000 0000 0000 0000
```

And it is almost completely mapped:

```
Pre-Paging MMU Tables Dump: Walking the page tables for address 0xffff f000

.. TTBR0 entry is 0x0000 0000 0100 2000
.. TTBR1 entry is 0x0000 0000 0100 2000
.. TTBCR is 0xb500 3501

Level  Tabl-Addr      Index         Entry Addr     Next PAddr     Attr Bits
-----  -----------    -----------   -----------    -----------    ---------------------
  1    0x0100 2000    0x0000 0003   0x0100 2018    0x0100 6000    0x0000 0000 0000 0003
  2    0x0100 6000    0x0000 01ff   0x0100 6ff8    0x0100 f000    0x0000 0000 0000 0003
  3    0x0100 f000    0x0000 01ff   0x0100 fff8    0x0100 e000    0x0000 0000 0000 0000
```

The fault looks like this:

```
Taking exception 4 [Data Abort]
...from EL1 to EL1
...with ESR 0x25/0x9600000b
...with DFSR 0x20b DFAR 0xffc00024
```

This is an access problem, not a mapping problem.

---

### 2020-May-06

OK, back into debugging....  I am currently working on this:

```
IN:
0x80804134:  e7e82ad6  ubfx     r2, r6, #0x15, #9
0x80804138:  e3a0300c  mov      r3, #0xc
0x8080413c:  e0030293  mul      r3, r3, r2
0x80804140:  e7d32007  ldrb     r2, [r3, r7]
0x80804144:  e0835007  add      r5, r3, r7
0x80804148:  e2023003  and      r3, r2, #3
0x8080414c:  e3530003  cmp      r3, #3
0x80804150:  1a00003e  bne      #0x80804250

R00=01010403 R01=0010000c R02=00000000 R03=00000403
R04=fffff000 R05=ffc00024 R06=f8000000 R07=ffc00000
R08=000000c0 R09=0003f000 R10=ffc00000 R11=000254cc
R12=00000264 R13=ff800fc8 R14=808053b4 R15=80804134
PSR=200001d3 --C- A NS svc32
Taking exception 4 [Data Abort]
...from EL1 to EL1
...with ESR 0x25/0x96000007
...with DFSR 0x207 DFAR 0xffc01500
```

The `DFAR` is indicating an address that is not expected.  Let me see if I can trace this down....

Well, a well (lucky) placed call to `kprintf("")` revealed that the problem happens when dealing with the level 3 table.

OK, with some well-places calls, I have been able to narrow this down a bit (the problem occurs between the 2 calls to `kprintf("")`):

```s
8080414c:       eb00105e        bl      808082cc <kprintf>
80804150:       e7d53007        ldrb    r3, [r5, r7]
80804154:       e2033003        and     r3, r3, #3
80804158:       e3530003        cmp     r3, #3
8080415c:       059b0004        ldreq   r0, [fp, #4]
80804160:       07fb0050        ubfxeq  r0, r0, #0, #28
80804164:       0a000007        beq     80804188 <MmuMapToFrame+0x15c>
80804168:       e3080abc        movw    r0, #35516      ; 0x8abc
8080416c:       e3480080        movt    r0, #32896      ; 0x8080
80804170:       eb001055        bl      808082cc <kprintf>
```

Now, from the debug log, I can narrow this down a little further:

```s
0x80804150:  e7d53007  ldrb     r3, [r5, r7]
0x80804154:  e2033003  and      r3, r3, #3
0x80804158:  e3530003  cmp      r3, #3
0x8080415c:  059b0004  ldreq    r0, [fp, #4]
0x80804160:  07fb0050  ubfxeq   r0, r0, #0, #0x1c
0x80804164:  0a000007  beq      #0x80804188
```

And it is not happening on the branch.  In fact, this will happen on either of the 2 load statements.  So the offending line of code?

```c++
    if (likely(!table->present || !table->tableFlag)) {
```
(and the `likely` tag was added to locate the lines in source)

So, the problem is that `table` is not mapped.

---

### 2020-May-09

I have a couple of hours this morning to work on the debugging.

So, let's start by confirming that I am having the problems where I think I am having the problems...  I believe I am having an issue with `MmuInit()`.  So the following loop should not have a problem:

```c++
//
// -- The actual loader main function
//    -------------------------------
EXTERN_C EXPORT LOADER NORETURN
void LoaderMain(archsize_t arg0, archsize_t arg1, archsize_t arg2)
{
    LoaderFunctionInit();               // go and initialize all the function locations
while (true){}
    MmuInit();                          // Complete the MMU initialization for the loader
    PlatformEarlyInit();
    kprintf("Welcome\n");
while (true){}
```

and that confirms that `LoaderFunctionInit()` does not generate the fault (meaning it is not calling `MmuMapToPage()` and I forgot).

So, moving that breakpoint past `MmuInit()`, I expect to have problems.  And I do.

So, what am I trying to do in `MmuInit()`?  Well, I am trying to map virtual address `0xf8000000` to physical address `0x3f000000`.  I do believe that this is failing on the first call.  Let's confirm that.  Confirmed.

So, let's break this down....  Level 1 Entry is `0xf8000000 >> 30` or entry #3.  This table will be mapped as I guarnteed it during `entry.s` initialization.  This table from entry 3 is at `0x0100 6000` based on the debugging output above.

The Level 2 entry is `(0xf8000000 >> 21) & 0x1ff` or entry `0x1c0`.  This would be at offset `0xe00`, and this entry would not exist.

I'm a dumbass!!!  I was supposed to be mapping a new table here:

```c++
    if (!table->present || !table->tableFlag) {
        table->tableAddress = PmmAllocateFrame();
        uint64_t ent = *((uint64_t *)table);
        ent |= tableBits;
    }
```

but the `ent` variable is a value, not a reference.  So, I really am never mapping this table.  A quick test shows a change (not sure if this is good or bad):

```
IN:
0x808041f4:  f2c00010  vmov.i32 d16, #0

R00=00000403 R01=ffc00024 R02=00000403 R03=fffff000
R04=fffff000 R05=f8000000 R06=0003f000 R07=000000c0
R08=ffc00000 R09=ffc00000 R10=00000403 R11=00000000
R12=00000264 R13=ff800fb8 R14=808053f0 R15=808041f4
PSR=600001d3 -ZC- A NS svc32
Taking exception 1 [Undefined Instruction]
...from EL1 to EL1
...with ESR 0x7/0x1fe00020
```

So, I am executing off the face of the earth....  Or am I??

```S
808041f4:       f2c00010        vmov.i32        d16, #0 ; 0x00000000
```

I'm betting this will work on real hardware, but I am not willing to test that.  I need to figure out what in creating that insruction -- it appears to be using a FPU register.

So I asked on chat:

> [07:27:52] <eryjus> some time ago, someone (zid, i think) offered a gcc command line options that would force the codegen to use only core cpu registers...  what was that option?
> [07:28:09] <zid> -mgeneral-regs-only
> [07:28:48] <eryjus> ahhh..., zid, thank you

Crap, but it is not available for this cross-.  OK, I guess I need to check real hardware...  `vmov` is used all other the place.

OK, maybe there is a parameter I can pass into qemu...??

---

### 2020-May-13

Hmmmm...

```c++
/* If adding a feature bit which corresponds to a Linux ELF
 * HWCAP bit, remember to update the feature-bit-to-hwcap
 * mapping in linux-user/elfload.c:get_elf_hwcap().
 */
```

I found this looking at the QEMU for arm source....

And checking:

```
[adam@adamlt2 century-os]$ readelf -A targets/rpi2b/bin/boot/kernel.elf
Attribute Section: aeabi
File Attributes
  Tag_CPU_name: "7VE"
  Tag_CPU_arch: v7
  Tag_CPU_arch_profile: Application
  Tag_ARM_ISA_use: Yes
  Tag_THUMB_ISA_use: Thumb-2
  Tag_FP_arch: VFPv4
  Tag_Advanced_SIMD_arch: NEONv1 with Fused-MAC
  Tag_ABI_PCS_wchar_t: 4
  Tag_ABI_FP_denormal: Needed
  Tag_ABI_FP_exceptions: Needed
  Tag_ABI_FP_number_model: IEEE 754
  Tag_ABI_align_needed: 8-byte
  Tag_ABI_enum_size: small
  Tag_ABI_VFP_args: VFP registers
  Tag_ABI_optimization_goals: Aggressive Speed
  Tag_CPU_unaligned_access: v6
  Tag_MPextension_use: Allowed
  Tag_DIV_use: Allowed in v7-A with integer division extension
  Tag_Virtualization_use: TrustZone and Virtualization Extensions
```

Perhaps this will work?

```
##
## -- Add some options to the build
##    -----------------------------
CFLAGS += -mlittle-endian
CFLAGS += -mcpu=cortex-a7
CFLAGS += -mfpu=vfpv2                   ## dumb this down for qemu?
CFLAGS += -mfloat-abi=hard
```

It appears to be better for qemu.  For the real hardware?  Well, it boots, but the loader is still not getting control.  But, I am still getting a data fault in qemu:

```
Taking exception 4 [Data Abort]
...from EL1 to EL1
...with ESR 0x25/0x96000046
...with DFSR 0xa06 DFAR 0xf8215004
```

OK, so `0xf82150004` I believe is the new MMIO address for the serial port.  And it is:

```c++
#define KRN_SERIAL_BASE (MMIO_VADDR+0x215000)
```

So, I am getting to where I should be outputting a character, but there is something wrong with the mapping.

Let's dig into the `DFSR` meaning:
* This was a fault on a Write
* LPAE is enabled
* This was a level 2 translation fault

So, I end up back at `MmuMapToFrame()`....

---

### 2020-May-15

OK, I was thinking about this: https://wiki.osdev.org/ARM_Paging#Recursive_Table_Mapping

I was wondering what it would take to make that work.  I was close, but forgot I really needed to map 2 or 4 entries at the end.

So, what are we really talking about here?  Let's talk about the kernel space first:
* The Level-1 table will be fixed and allocated from the start, so there will be no need to touch the Level-1 table after the MMU is up and running.  So, there is no need to map it -- I just need to know the physical address of this table.
* There are 4 Level-2 tables and each will need to be mapped.  This means that the final 4 entries will need to be allocated to this space:
    * Index `0x1fc` to entry 0 (offset `0xfe0`)
    * Index `0x1fd` to entry 1 (offset `0xfe8`)
    * Index `0x1fe` to entry 2 (offset `0xff0`)
    * Index `0x1ff` to entry 3 (offset `0xff8`)
* This, then, means that virtual addresses from `((3<<30) | 1fc<<21)` or `(c0000000 | 3f800000)` or `0xff800000` will be lost to recursive mappings.  This is 8MB (compared to x86 4MB at `0xffc00000`) and conflicts currently with the MMIO addresses.
* That said, the kernel space should only be concerned about the top 2 pages/frames, so this can really be cut in half (and results in `0xffc00000` and up)
* That said, the user space will also need a block of memory for recursive mapping (thinking `0x7fc00000` up to `0x80000000`), which would also be 4MB.  The key for the initial kernel memory map would be to map these addresses as well.

This leaves me at 8MB of virtual address space used total for arm, and a separation between kernel and user address space.

What sucks is that I have to scrap all the code I've written (and re-written) to write it all again.  Isn't this iteration 4 for arm??

So, let's start with the entry code....  I currently have 4 frames allocated and set into the registers `r6` to `r9`.  To complete the recurisve mappings, I only need to execute the following instuctions (cleaning up the immediate sizes of course):

```s
    str     r6,[r7,#0xff0]
    str     r7,[r7,#0xff8]
    str     r8,[r9,#0xff0]
    str     r9,[r9,#0xff8]
```

... of course, there are some additional bit twiddling to do, so the actual implementation will not quite be that simple.

---

So, let's talk about what the addresses are for each "side".  First the kernel.  We have an address range from `0xffc00000` and up.  2 frames are set aside for the recursive mappings.  These are going to end up being 2 2MB blocks and they will be at the top of the address range.  But here is where things get a bit trciky: address `0xffdff000` is going to map to the lower table (in index 2) whereas address `0xfffff000` is going to map to the upper table (the one in index 3).  This (not so) simple fact makes the management a bit tricky.  What we really need to do is break the address space up into 4 blocks, not 2.

To manage the Level-2 table for index 2, the math looks something like this:
* `0xc0000000 + (0x1ff << 21) + (0x1fe << 12)`
* `0xc0000000 + 0x3fe00000 + 0x001fe000`
* `0xffffe000`

To contrast, the address to manage the Level-2 table for index 3, the math looks like this:
* `0xc0000000 + (0x1ff << 21) + (0x1ff << 12)`
* `0xc0000000 + 0x3fe00000 + 0x001ff000`
* `0xfffff000`

So, I guess it's not as tricky as I thought....

I have a diagram in front of me and I will work on some ASCII art tomorrow.

---

### 2020-May-16

OK, for that ASCII art:

```

                             (Table A)
                +-----+-----+-----+-----+--------------+
                |  0  |  1  |  2  |  3  |  ...         |
                +-----+-----+-----+-----+--------------+
                               |     |
+------------------------------+     +--- +
|                                         |                     +----------+
V         (Table B)                       V    (Table C)        V          |
+--+--+--+--+--+--+----------+--+--+      +--+--+--+--+--+--+----------+--+--+
|  |  |  |  |  |  |  ...     |  |  |      |  |  |  |  |  |  |  ...     |  |  |
+--+--+--+--+--+--+----------+--+--+      +--+--+--+--+--+--+----------+--+--+
                      ^                                                 |
                      +-------------------------------------------------+

```

So, if I wanted to maintain the Level-2 Table located at Index Level-1[2] (meaning TableB), I would take TableA[3], TableC[0x1ff], TableC[0x1fe].  This would turn into address `0xffffe000`.

So, if I have a base address for managing the kernel space (`0xffffe000`) Level-2 table and the same for the user spcace (`0x7fffe000`) Level-2 table, how to I decide which entry to look at?  Well, each entry looks at 2MB of address space....  My first thought is that it the mask should be `0x7fe00000`.  Shifting these bits to the right, we end up with `0x3ff`, or 1024 possible entries.  This is accurate.  Notice, I am trimming off the most significant digit here, which is used to determine user vs. kernel space.  So, the macro to get this index would be `(addr & 0x7fe00000) >> 21`.

Now for the Level-3 table, these addresses start at `0xffc00000` and `0x7fc00000`.  This mask should be `0x001ff000`.  This results in 512 entries and is alignment with a single table.  This marcro would look like `(addr & 0x001ff000) >> 12`.

Note that both macros work perfectly well for user and kernel space.  The difference there is that the address of the table changes.

---

### 2020-May-17

I was able to figure out the bug for real hardware what was preventing the boot.  Having that fixed, I am now able to work on getting the mappings correct before getting out of `entry.s`.  At the moment, I am seeing some problems.

So, let's start with the fundamentals first: I'm going to debug the mapping for address `0xfffff000`.  This should be the top table, the value in `r9` for level-2, and the value in `r9` in level 3, and then the mapping to the value in `r9` yet again for the final physical address.  Lots of looping here.  I do not know the value of `r9`, but that should not really matter much.  It should be the value of the level 1 table plus 4 pages.

My starting point for this work is:

```
Pre-Paging MMU Tables Dump: Walking the page tables for address 0xffff f000

.. TTBR0 entry is 0x0000 0000 0100 2000
.. TTBR1 entry is 0x0000 0000 0100 2000
.. TTBCR is 0xb500 3501

Level  Tabl-Addr      Index         Entry Addr     Next PAddr     Attr Bits
-----  -----------    -----------   -----------    -----------    ---------------------
  1    0x0100 2000    0x0000 0003   0x0100 2018    0x0100 6000    0x0000 0000 0000 0003
  2    0x0100 6000    0x0000 01ff   0x0100 6ff8    0x0100 f000    0x0000 0000 0000 0003
  3    0x0100 f000    0x0000 01ff   0x0100 fff8    0x0100 e000    0x0000 0000 0000 0747
```

And, of course, this is wrong.  Level 1 does appear to be correct.  But, I expect level 2 next address to be `0x01006000` and I expect the level 3 next address to be `0x01006000`.

So, the first question to be answered is whether this is set right to begin with and updated to be wrong or if it is set wrong from the beginning.  A simple jump should be enough to determine....

```
Pre-Paging MMU Tables Dump: Walking the page tables for address 0xffff f000

.. TTBR0 entry is 0x0094 0097 bb98 3b52
.. TTBR1 entry is 0x00f1 0064 dfed 60c1
.. TTBCR is 0x0000 0000

Level  Tabl-Addr      Index         Entry Addr     Next PAddr     Attr Bits
-----  -----------    -----------   -----------    -----------    ---------------------
  1    0x0100 2000    0x0000 0003   0x0100 2018    0x0100 6000    0x0000 0000 0000 0003
  2    0x0100 6000    0x0000 01ff   0x0100 6ff8    0x0100 6000    0x0000 0000 0000 0003
  3    0x0100 6000    0x0000 01ff   0x0100 6ff8    0x0100 6000    0x0000 0000 0000 0003
```

So this is correct originally.  The only problem here is that the page flags are not correct.  Ok, let's look at address `0xffffe000`:

```
Pre-Paging MMU Tables Dump: Walking the page tables for address 0xffff e000

.. TTBR0 entry is 0x008d 00dd 79f4 295b
.. TTBR1 entry is 0x008a 00c1 beda 5cb0
.. TTBCR is 0x0000 0000

Level  Tabl-Addr      Index         Entry Addr     Next PAddr     Attr Bits
-----  -----------    -----------   -----------    -----------    ---------------------
  1    0x0100 2000    0x0000 0003   0x0100 2018    0x0100 6000    0x0000 0000 0000 0003
  2    0x0100 6000    0x0000 01ff   0x0100 6ff8    0x0100 6000    0x0000 0000 0000 0003
  3    0x0100 6000    0x0000 01fe   0x0100 6ff0    0x0100 5000    0x0000 0000 0000 0003
```

Again, this looks correct, except for the page flags.

So, the next step is to clean up the page flags:

```
Pre-Paging MMU Tables Dump: Walking the page tables for address 0xffff f000

.. TTBR0 entry is 0x008c 00dd f9f6 295b
.. TTBR1 entry is 0x008a 00c1 befa 4cb0
.. TTBCR is 0x0000 0000

Level  Tabl-Addr      Index         Entry Addr     Next PAddr     Attr Bits
-----  -----------    -----------   -----------    -----------    ---------------------
  1    0x0100 2000    0x0000 0003   0x0100 2018    0x0100 6000    0x0000 0000 0000 0747
  2    0x0100 6000    0x0000 01ff   0x0100 6ff8    0x0100 6000    0x0000 0000 0000 0747
  3    0x0100 6000    0x0000 01ff   0x0100 6ff8    0x0100 6000    0x0000 0000 0000 0747


Pre-Paging MMU Tables Dump: Walking the page tables for address 0xffff e000

.. TTBR0 entry is 0x008c 00dd f9f6 295b
.. TTBR1 entry is 0x008a 00c1 befa 4cb0
.. TTBCR is 0x0000 0000

Level  Tabl-Addr      Index         Entry Addr     Next PAddr     Attr Bits
-----  -----------    -----------   -----------    -----------    ---------------------
  1    0x0100 2000    0x0000 0003   0x0100 2018    0x0100 6000    0x0000 0000 0000 0747
  2    0x0100 6000    0x0000 01ff   0x0100 6ff8    0x0100 6000    0x0000 0000 0000 0747
  3    0x0100 6000    0x0000 01fe   0x0100 6ff0    0x0100 5000    0x0000 0000 0000 0747
```

This looks better.  Now to check the user space mappings:

```
Pre-Paging MMU Tables Dump: Walking the page tables for address 0x7fff f000

.. TTBR0 entry is 0x008d 00dd f9f6 295b
.. TTBR1 entry is 0x008e 00c1 beda 4c90
.. TTBCR is 0x0000 0000

Level  Tabl-Addr      Index         Entry Addr     Next PAddr     Attr Bits
-----  -----------    -----------   -----------    -----------    ---------------------
  1    0x0100 2000    0x0000 0001   0x0100 2008    0x0100 4000    0x0000 0000 0000 0747
  2    0x0100 4000    0x0000 01ff   0x0100 4ff8    0x0100 4000    0x0000 0000 0000 0747
  3    0x0100 4000    0x0000 01ff   0x0100 4ff8    0x0100 4000    0x0000 0000 0000 0747


Pre-Paging MMU Tables Dump: Walking the page tables for address 0x7fff e000

.. TTBR0 entry is 0x008d 00dd f9f6 295b
.. TTBR1 entry is 0x008e 00c1 beda 4c90
.. TTBCR is 0x0000 0000

Level  Tabl-Addr      Index         Entry Addr     Next PAddr     Attr Bits
-----  -----------    -----------   -----------    -----------    ---------------------
  1    0x0100 2000    0x0000 0001   0x0100 2008    0x0100 4000    0x0000 0000 0000 0747
  2    0x0100 4000    0x0000 01ff   0x0100 4ff8    0x0100 4000    0x0000 0000 0000 0747
  3    0x0100 4000    0x0000 01fe   0x0100 4ff0    0x0100 3000    0x0000 0000 0000 0747
```

This looks right.

---

### 2020-May-18

The next address mapped in sequence is `0xff400000`, which is used for the interrupt vector table.

This looks reasonable, except for the flags:

```
Pre-Paging MMU Tables Dump: Walking the page tables for address 0xff40 1000

.. TTBR0 entry is 0x008c 00dd f9f4 295b
.. TTBR1 entry is 0x008a 00c1 beda 4cb2
.. TTBCR is 0x0000 0000

Level  Tabl-Addr      Index         Entry Addr     Next PAddr     Attr Bits
-----  -----------    -----------   -----------    -----------    ---------------------
  1    0x0100 2000    0x0000 0003   0x0100 2018    0x0100 6000    0x0000 0000 0000 0747
  2    0x0100 6000    0x0000 01fa   0x0100 6fd0    0x0100 7000    0x0000 0000 0000 0003
  3    0x0100 7000    0x0000 0001   0x0100 7008    0x0100 1000    0x0000 0000 0000 0747
```

And the other mappings appear to not have changed as well.

Next up is the `.mboot` section, which is mapped to 1MB....  Again, this looks good except for the page attributes.

```
Pre-Paging MMU Tables Dump: Walking the page tables for address 0x0010 08fc

.. TTBR0 entry is 0x008c 00dd f9f6 295b
.. TTBR1 entry is 0x008a 00c1 bed2 4c90
.. TTBCR is 0x0000 0000

Level  Tabl-Addr      Index         Entry Addr     Next PAddr     Attr Bits
-----  -----------    -----------   -----------    -----------    ---------------------
  1    0x0100 2000    0x0000 0000   0x0100 2000    0x0100 3000    0x0000 0000 0000 0747
  2    0x0100 3000    0x0000 0000   0x0100 3000    0x0100 8000    0x0000 0000 0000 0003
  3    0x0100 8000    0x0000 0100   0x0100 8800    0x0010 0000    0x0000 0000 0000 0747
```

Next up is the loader code at `0x80000000`:

```
Pre-Paging MMU Tables Dump: Walking the page tables for address 0x8000 0000

.. TTBR0 entry is 0x008c 00dd f9f4 295b
.. TTBR1 entry is 0x008a 00c1 bed2 4c90
.. TTBCR is 0x0000 0000

Level  Tabl-Addr      Index         Entry Addr     Next PAddr     Attr Bits
-----  -----------    -----------   -----------    -----------    ---------------------
  1    0x0100 2000    0x0000 0002   0x0100 2010    0x0100 5000    0x0000 0000 0000 0747
  2    0x0100 5000    0x0000 0000   0x0100 5000    0x0100 9000    0x0000 0000 0000 0747
  3    0x0100 9000    0x0000 0000   0x0100 9000    0x0010 1000    0x0000 0000 0000 0747
```

This also looks reasonable, and the earlier mappings are still clean.  Now for the syscall locations, at `0x80400000`:

```
Pre-Paging MMU Tables Dump: Walking the page tables for address 0x8040 0000

.. TTBR0 entry is 0x008c 00dd f9f6 295b
.. TTBR1 entry is 0x009e 00c1 beda 4c90
.. TTBCR is 0x0000 0000

Level  Tabl-Addr      Index         Entry Addr     Next PAddr     Attr Bits
-----  -----------    -----------   -----------    -----------    ---------------------
  1    0x0100 2000    0x0000 0002   0x0100 2010    0x0100 5000    0x0000 0000 0000 0747
  2    0x0100 5000    0x0000 0002   0x0100 5010    0x0100 a000    0x0000 0000 0000 0747
  3    0x0100 a000    0x0000 0000   0x0100 a000    0x0010 4000    0x0000 0000 0000 0747
```

Again, reasonable.  Still not stepping on anything.  Next is the kernel proper, at `0x80800000`:

```
Pre-Paging MMU Tables Dump: Walking the page tables for address 0x8080 0000

.. TTBR0 entry is 0x008c 00dd f9f4 295b
.. TTBR1 entry is 0x009a 00c1 bed2 4c90
.. TTBCR is 0x0000 0000

Level  Tabl-Addr      Index         Entry Addr     Next PAddr     Attr Bits
-----  -----------    -----------   -----------    -----------    ---------------------
  1    0x0100 2000    0x0000 0002   0x0100 2010    0x0100 5000    0x0000 0000 0000 0747
  2    0x0100 5000    0x0000 0004   0x0100 5020    0x0100 b000    0x0000 0000 0000 0747
  3    0x0100 b000    0x0000 0000   0x0100 b000    0x0010 5000    0x0000 0000 0000 0747
```

Now, checking data:

```
Pre-Paging MMU Tables Dump: Walking the page tables for address 0x8100 0000

.. TTBR0 entry is 0x008c 00dd f9f4 295b
.. TTBR1 entry is 0x008a 00c1 bed2 4890
.. TTBCR is 0x0000 0000

Level  Tabl-Addr      Index         Entry Addr     Next PAddr     Attr Bits
-----  -----------    -----------   -----------    -----------    ---------------------
  1    0x0100 2000    0x0000 0002   0x0100 2010    0x0100 5000    0x0000 0000 0000 0747
  2    0x0100 5000    0x0000 0008   0x0100 5040    0x0100 c000    0x0000 0000 0000 0747
  3    0x0100 c000    0x0000 0000   0x0100 c000    0x0011 2000    0x0000 0000 0000 0747
```

This looks reasonable, but I need to check the frame to be sure.  It's good:

```
Program Headers:
  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align
  LOAD           0x001000 0x00100000 0x00100000 0x01000 0x01000 R E 0x1000
  LOAD           0x002000 0x80000000 0x00101000 0x03000 0x03000 RWE 0x1000
  LOAD           0x005000 0x80400000 0x00104000 0x01000 0x01000 RWE 0x1000
  LOAD           0x006000 0x80800000 0x00105000 0x0d000 0x0d000 R E 0x1000
  LOAD           0x013000 0x81000000 0x00112000 0xd0000 0xd0000 RW  0x1000
```

Next up is the stack at `0xff800000`, which was good as well.

So, now back to qemu for the logs, and I get the following:

```
IN:
0x80803d9c:  e92d47f0  push     {r4, r5, r6, r7, r8, sb, sl, lr}
0x80803da0:  e3a05a0e  mov      r5, #0xe000
0x80803da4:  e7e92ad0  ubfx     r2, r0, #0x15, #0xa
0x80803da8:  e1a03005  mov      r3, r5
0x80803dac:  e34f3fff  movt     r3, #0xffff
0x80803db0:  e1a06001  mov      r6, r1
0x80803db4:  e3475fff  movt     r5, #0x7fff
0x80803db8:  e2001502  and      r1, r0, #0x800000
0x80803dbc:  e3510000  cmp      r1, #0
0x80803dc0:  e3a0700c  mov      r7, #0xc
0x80803dc4:  01a05003  moveq    r5, r3
0x80803dc8:  e3a03000  mov      r3, #0
0x80803dcc:  e0070297  mul      r7, r7, r2
0x80803dd0:  e34f3fc0  movt     r3, #0xffc0
0x80803dd4:  e3a02000  mov      r2, #0
0x80803dd8:  e7d51007  ldrb     r1, [r5, r7]
0x80803ddc:  01a08003  moveq    r8, r3
0x80803de0:  e3472fc0  movt     r2, #0x7fc0
0x80803de4:  e1a04000  mov      r4, r0
0x80803de8:  e0859007  add      sb, r5, r7
0x80803dec:  11a08002  movne    r8, r2
0x80803df0:  e3110001  tst      r1, #1
0x80803df4:  0a00001f  beq      #0x80803e78

R00=f8000000 R01=0003f000 R02=80000003 R03=80001628
R04=f8001000 R05=0003f001 R06=f9040000 R07=01004000
R08=01005000 R09=01006000 R10=0002830f R11=000254cc
R12=00000264 R13=ff800fe8 R14=800007fc R15=80803d9c
PSR=200001d3 --C- A NS svc32
Taking exception 4 [Data Abort]
...from EL1 to EL1
...with ESR 0x25/0x96000007
...with DFSR 0x207 DFAR 0xd00
```

But this is a straight bug: `DFAR 0xd00`.

---

OK, now I am getting the first letter of `"Welcome"`.  After that, the next letter does not write.  This is most likely because I do not have the memory set to device memory for the MAIR settings.  Let me work on setting that up.

qemu says I am getting a data fault, on some odd address.  This is likely in the `kprintf()` function, or even in the `SerialPutChar()`.  I do get 1 character to write....  but that's it.

---

### 2020-May-19

So, I have been debugging a ghost!!!  Here is what is going on:
* I boot
* I print a welcome message
* I start into `PlatformEarlyInit()`, which in turn maps and unmaps pages in to memory space
* Unmapping pages is not supported yet

When I loop indefinitely after printing the welcome message, it works properly.  This is only as far as I can reasonably expect the kernel to work so far since the rest of the functions are not implemented.

Sooooo....., now that I am no longer debugging the wrong problem....  the correct problem is:

```
Welcome to CenturyOS -- a hobby operating system
    (initializing...)
Attempting the clear the monitor screen at address 0xfb000000
.. Done!
Data Exception:
.. Data Fault Address: 0x90000020
.. Data Fault Status Register: 0x00001210
.. Fault status 0x0: Unknown
.. Fault occurred because of a read
At address: 0xff800f88
 R0: 0x90000088   R1: 0x8080b232   R2: 0x0000001e
 R3: 0x90000020   R4: 0x9000000c   R5: 0x81000530
 R6: 0x00000000   R7: 0xff804000   R8: 0x01005000
 R9: 0x01006000  R10: 0x440e359f  R11: 0xf580abfe
R12: 0x0000ff50   SP: 0x80000bac   LR_ret: 0x80000bc4
SPSR_ret: 0x600001d3     type: 0x17

Additional Data Points:
User LR: 0xafefcdfe  User SP: 0xe7fb68f9
Svc LR: 0xff800f98
```

A couple of things to take away from this fault:
1. The DFSR is no longer being interpreted properly
1. The fault is in `ProcessInit()`
1. The faulting address appears to be a heap address

I started by cleaning up the DFSR interpretation.

I missed catching the updates, but.... This turned out to be related to the attr settings.  Changing from Strongly Ordered memory to Normal memory helped.

The conversation about this:
> [17:17:35] <eryjus> hmmm....  this is from my first heap allocation and worked properly until I changed mmu to the long descriptors...  no change in login with the heap init.
> [17:17:42] <geist> probably not, ARM is in UK
> [17:18:08] <geist> eryjus: possibly bad physical address in a page table
> [17:17:35] <eryjus> hmmm....  this is from my first heap allocation and worked properly until I changed mmu to the long descriptors...  no change in login with the heap init.
> [17:17:42] <geist> probably not, ARM is in UK
> [17:18:08] <geist> eryjus: possibly bad physical address in a page table
> [18:04:18] <eryjus> geist, any thoughts for option b?  0x0101b000 should be a valid address.
> [18:09:15] <geist> eryjus: what is option b?
> [18:09:18] <geist> also are you on real hardware?
> [18:09:36] <geist> also dunno, is that a valid address?
> [18:09:58] <eryjus> i am on real hardware...  i'm curious if something more than an invalid address would cause the problem -- it is valid
> [18:10:28] <eryjus> or i cannot imagine is it invalid -- it's an rpi2b
> [18:12:48] <eryjus> even more interesting is that this is happening after i have initialized my heap and I have already written to that page.
> [18:41:30] <geist> eryjus: is that within the usual dram range on a rpi2?
> [18:42:02] <geist> it's also possible the memory is marked off limits in secure mode. fairly common on many arm platforms, though i dunno if any of the raspberry pis do that
> [18:44:14] <eryjus> 1GB dram....
> [18:48:11] <geist> eryjus: indeed. i guess what i was asking was dram on rpi2 goes from 0 + right? not all do
> [18:49:22] <eryjus> turned out that when I changed the Attr from strongly ordered memory to normal memory it worked...  though I cannot imagine it should have made a difference, or there is something else wrong
> [18:53:19] <geist> yah possible that it'll just fail a bit later
> [18:53:35] <geist> because if it's normal memory then it's in the cache and it's only until the cache is written out (if it was a write that triggered it)
> [19:05:36] <eryjus> anyway, changed the attributes on the memory and I am getting much farther along.  you think there may still be an underlying problem?
> [19:07:15] <geist> yeah i bet there is
> [19:07:25] <geist> also what kind of memory is it? is that dram? io?
> [19:07:40] <eryjus> dram
> [19:07:45] <geist> was there a reason ou had it set to strongly ordered before?
> [19:07:56] <eryjus> not really
> [19:08:10] <geist> that's pretty strange to have dram mapped SO, so it's possible the memory controller balked at you
> [19:08:30] <geist> SO is above and beyond usual uncached modes, since it basically forces a full sync every access
> [19:10:00] <eryjus> ok....  will keep it in mind...  gonna bookmark this conversation in case something comes back up again.
> [19:10:18] <geist> butr yeah, general rule map dram as normal memory. that's full cached, what you generally want
> [19:10:39] <geist> mmio regions you should map as 'device memory' which is uncached, but allows posted writes (ie, write and then move on)
> [19:11:20] <eryjus> which was my earlier questions which turned out to be me looking in the wrong spot
> [19:11:28] <geist> SO is mega synchronized, since it does what gungoman said and runs the cpu one load/store at a time
> [19:12:00] <eryjus> missed that...

Anyway, I am now getting to where the other cores are trying to start.  This means that `entryAp.s` is in play.

That cleaned up, I am back to where I was beore I started trying to set up a user process.  So, I am going to commit this code now (and keep the same version). since I have such a good spot to stop.  I still need to get a user-space process running on the rpi2b target.

---

That done, I address the user-space process.

---

### 2020-May-20

To start my day, I need to prepare a new user-process paging table.  So, this is controlled from `ProcessPrepareFromImage()`, which is very x86-centric.

That cleaned up, I am still getting a data fault when I try to map a user-space page.

```
Data Exception:
.. Data Fault Address: 0x7fc00040
.. Fault occurred because of a read
.. LPAE is enabled
.. Data Fault Status Register: 0x00000207
.. Fault status 0x7: Translation Fault -- Level 3

MMU Tables Dump: Walking the page tables for address 0x7fc00040

.. TTBR0 entry is 0x3eff3000
.. TTBR1 entry is 0x01002000

Level  Tabl-Addr     Index        Entry Addr    Next Frame    Attr Bits
-----  ----------    ----------   ----------    ----------    ---------------------
  2    0x7ffff000    0x000003fe   0x7ffffff0    0x00001003    0x00000000 0x00000743
  3    0x7fffe000    0x00000000   0x7fffe000    0x00001008    0x00000000 0x00000743

At address: 0xff800f10
 R0: 0x00008000   R1: 0x000001e6   R2: 0x00000000
 R3: 0x00000008   R4: 0x7fc00040   R5: 0x00008000
 R6: 0x00000000   R7: 0x000001e6   R8: 0x7fc00000
 R9: 0x00000001  R10: 0x00000000  R11: 0x00000000
R12: 0xf8215000   SP: 0x80806568   LR_ret: 0x80804144
SPSR_ret: 0x200001d3     type: 0x17

Additional Data Points:
User LR: 0xafef8dff  User SP: 0xe7fa68f9
Svc LR: 0xff800f20
```

One thing that stands out is that the frames are not from the newly allocated ones for the user space.  In other words, I am still looking at the previous TTBR0 register -- the user space has not been recognized (it exists properly in the TTBR0 register).

Shooting down the entire TLB worked (half the address space makes this reasonable).  I also did the same thing with `ProcessSwitch()`, but I still have problems:

```
+---------------------------+--------+----------+----------+------------+-----------------------+--------------+
| Command                   | PID    | Priority | Status   | Address    | Time Used             | Kernel Stack |
+---------------------------+--------+----------+----------+------------+-----------------------+--------------+
| Butler                    | 0      | LOW      | MSGW     | 0x9000000c | 0x00000000 0x0044aba2 | 0xff805000   |
| Idle Process              | 1      | IDLE     | RUNNING  | 0x900000bc | 0x00000000 0x00c858de | 0xff806000   |
| Idle Process              | 2      | IDLE     | RUNNING  | 0x9000016c | 0x00000000 0x00c6271f | 0xff808000   |
| Idle Process              | 3      | IDLE     | READY    | 0x9000021c | 0x00000000 0x00c3ee7e | 0xff80a000   |
| Idle Process              | 4      | IDLE     | RUNNING  | 0x900002cc | 0x00000000 0x00c83293 | 0xff80c000   |
| test.elf                  | 8      | NORMAL   | INIT     | 0x90000644 | 0x00000000 0x00000000 | 0x00007000   |
| Process A                 | 9      | OS       | MSGW     | 0x900007fc | 0x00000000 0x0000c84a | 0xff811000   |
| Process B                 | 10     | OS       | DLYW     | 0x900008ac | 0x00000000 0x0000f971 | 0xff813000   |
| Kernel Debugger           | 11     | OS       | RUNNING  | 0x9000095c | 0x00000000 0x011e7a8e | 0xff815000   |
+---------------------------+--------+----------+----------+------------+-----------------------+--------------+
sched :>
 (allowed: show,status,run,ready,list,exit)
```

The process is still in an `INIT` state.

---

### 2020-May-21

Today I take on why the new `test.elf` process is still in the `INIT` state.  Turns out I had a line commented out which readies the process.  Now, once the process is ready to be scheduled, the rpi2b locks.  So, something is not working properly with the user process.  I am going to start by commenting out the extra bits compared to the kernel process start.  This process should, then, run at privilege.

```
Welcome to the Century-OS kernel debugger
- :> Prefetch Abort:
At address: 0x00002f70imer,msgq)
 R0: 0x00000001   R1: 0x00000000   R2: 0x00000000
 R3: 0x00000000   R4: 0x00000000   R5: 0x00000000
 R6: 0x00000000   R7: 0x000080e4   R8: 0x00000000
 R9: 0x00000000  R10: 0x00000000  R11: 0x00000000
R12: 0x00000000   SP: 0x00008030   LR_ret: 0x010d8e18
SPSR_ret: 0x800001d3     type: 0x17

Additional Data Points:
User LR: 0xcf99df8d  User SP: 0xba5defbd
Svc LR: 0x00002f80
```

Even cleaned up, I am not sure what is really going on here.  And I am going to need to code some more details:

```c++
void PrefetchHandler(isrRegs_t *regs)
{
    kprintf("Prefetch Abort:\n");
    IsrDumpState(regs);
}
```

Making that more like the `DataAbortHandler()` and re-running -- I am back to a lock-up.  So I have some damned race condition.  OK, now I have a Data Abort!

```
Data Exception:
.. Data Fault Address: 0x7fc086c0
.. Fault occurred because of a read
.. LPAE is enabled
.. Data Fault Status Register: 0x00000207
.. Fault status 0x7: Translation Fault -- Level 3

MMU Tables Dump: Walking the page tables for address 0x7fc086c0

.. TTBR0 entry is 0x3eff3000
.. TTBR1 entry is 0x01002000

Level  Tabl-Addr     Index        Entry Addr    Next Frame    Attr Bits
-----  ----------    ----------   ----------    ----------    ---------------------
  2    0x7ffff000    0x000003fe   0x7ffffff0    0x0003eff2    0x00600000 0x00000763
  3    0x7fffe000    0x00000008   0x7fffe040    0x00000000    0x00000000 0x00000000

At address: 0x00002ee0
 R0: 0x8080b064   R1: 0x00000000   R2: 0x000010d8
 R3: 0x000010d8   R4: 0x7fc086c0   R5: 0xfffff000
 R6: 0x7fc00000   R7: 0x00000000   R8: 0xffffffff
 R9: 0xffffffff  R10: 0x00000000  R11: 0x00000000
R12: 0xf8215000   SP: 0x80803ee4   LR_ret: 0x80803ef8
SPSR_ret: 0x600001d3     type: 0x17

Additional Data Points:
User LR: 0x9e5ffe3e  User SP: 0xe7f6e7bf
Svc LR: 0x00002ef0
```

All the mapping should be completed, and this is occurring in `MmuMapToFrame()`.  So, I am going to turn off the `ProcessReady()` again and try to get to the bottom of this problem.  It's obviously in the MMU code.

OK, that does not appear to be the problem....  Maybe comment the cleanup?  That had no effect.

---

### 2020-May-22

OK, so let's see here....  How to debug this issue?

I guess I first need to see if it can be duplicated on qemu....  No, that is not going to help.

So, I guess the next step here will be to comment out the debugger for the moment (it steps on debug output) and see if I can narrow down the problem in the elf loader.

---

OK, so after some debugging with the rpi2b (which will not emit a complete boot!) and a reboot of this host computer, I think I need to hard-power the computer to force a hard-reset on the USB hardware.  No matter what I do, I cannot get a full and complete boot on either rpi2b or rpi3b hardware even thought I should.

So, I am taking updates and I am going to hard reboot the PC.  Should take an hour to get that complete.  Fun, fun, fun!!!

---

OK, I think I see what is going on here...  I am not jumping to the `LoaderMain()` code and getting the memory mapped properly.  This should be easy enough to check.  And that is confirmed:

```
Level  Tabl-Addr      Index         Entry Addr     Next PAddr     Attr Bits
-----  -----------    -----------   -----------    -----------    ---------------------
  1    0x0100 2000    0x0000 0003   0x0100 2018    0x0100 6000    0x0000 0000 0000 0743
  2    0x0100 6000    0x0000 01fe   0x0100 6ff0    0x0100 5000    0x0000 0000 0000 0743
  3    0x0100 5000    0x0000 0000   0x0100 5000    0x0100 9000    0x0000 0000 0000 0743
Enabling Paging...
```

No clue what happened.  This was working reasonably well last night...  well 2 nights ago.  Let me change from the pi3 back to the pi2 and confirm I am in the same spot.  And I am faulting in the same spots.

So, where am I faulting really?  This is clearly happening between where I enable paging and I am able to map the MMIO memory space (so `kprintf()` will work again).  So, I am going to have to go back to QEMU and do some checking.

QEMU confirms:

```
IN:
0x001008e4:  e59fd0d8  ldr      sp, [pc, #0xd8]
0x001008e8:  ee070fd5  mcr      p15, #0, r0, c7, c5, #6
0x001008ec:  ee070f11  mcr      p15, #0, r0, c7, c1, #0
0x001008f0:  e59f00d0  ldr      r0, [pc, #0xd0]
0x001008f4:  ebfffe19  bl       #0x100160

R00=01002000 R01=00c5187d R02=00000000 R03=00000028
R04=00000040 R05=0100d000 R06=01003000 R07=01004000
R08=01005000 R09=01006000 R10=0002830f R11=000254cc
R12=00000264 R13=01001000 R14=001001b4 R15=001008e4
PSR=200001d3 --C- A NS svc32
Taking exception 4 [Data Abort]
...from EL1 to EL1
...with ESR 0x25/0x96000006
...with DFSR 0x206 DFAR 0x3f215054
```

So for some reason, I have lost the mapping of the MMIO mappings...  wait!  This is the extra output I added after enabling paging.  I need to clean that up first, obviously.

---

### 2020-May-23

At this point, I believe I have a bad FTDI cable, or I am losing the USB controller on the laptop.  I am going to order a new cable (or 3).  However, in the meantime, I am completely unable to debug the rpi code.  So, let's see...  what are my options?
* refit for a qemi-rpi2b target so I can debug using qemu
* start an x86_64 target to start to support 64-bit
* change gears altogether and work on an emulator of my own
* walk away until this is resolved

Well, I think the first option is the best.  Or at least figure out how to get the serial working for qemu.

---

OK, I think I have qemu working now.  I am at least getting my serial debug output!!  I'm happy about that.

The problem was that I have 2 available serial devices, and I was only mapping one.  I needed to redirect the UART0 (the PL011 UART) to `/dev/null`, and then use a second `-serial` option for the AUX UART.  I can now see the output.

And I now have much better visibility into where the system is failing.  And it is failing trying to read the Multiboot structure.  If I recall correctly, this is mapped prior to trying to read it.

I'm wondering: did I finally reach a critical kernel size?  Nope -- it just turns out that the multiboot information in the MB1 structure for qemu is not like the standard states -- and the bootloader name is in an invalid location.  A quick change to validate that is all that was required.

So, I feel like I am back on track here.

I have now made it to the point where the other CPUs are trying to start.  It looks like CPU0 is able to start CPU1 and CPU1 confirmed it is started, but it appears that CPU0 does not recognize that the data has changed.  This means I have problems with the atomics again.

I have been able to trace the lock-up down to the `MmuUnmapPage()` function where the function tries to obtain the spinlock for the TLB Flush IPI.

```
Preparing to unmap address 0xff40a000
.. kernel-space
Locking the TLB Flush IPI mechanism; its current state is unlocked at 0x81004d18
```

Let me confirm I am not getting this lock.  No, that is good:

```
Locking the TLB Flush IPI mechanism; its current state is unlocked at 0x81004d18
.. lock obtained
```

```c++
    archsize_t flags = SPINLOCK_BLOCK_NO_INT(tlbFlush.lock) {
        kprintf(".. lock obtained\n");
        tlbFlush.addr = -1;
        PicBroadcastIpi(picControl, IPI_TLB_FLUSH);

        kprintf("Completed IPI Broadcast\n");
```

So, the problem is in `PicBroadcastIpi()`....  And this goes back to a problem with the atomics.

I believe the problem is going to boil down to the new MMU implementation and the caching that I have set up versus the caching that used to be set up using the old short-descriptor.

So, let me go collect the settings us used to use with the old short-descriptor.
* s (sharable) which was set to `1` for all pages
* apx (access permissions extension bit) is `0` (and indicated for all access)
* ap (access permissions) is `11` and also indivated for all access
    * the first `01` means all access from any privilege
    * the final `1` means the page can be accessed
* tex is going to be either classified as a device or normal memory, depending on the `PG_DEVICE` flag
    * tex device is defined as `010`
    * tex normal is defined as `000`
* c is for cached and is defined depending on the `PG_DEVICE` flag
    * c device is defined as `0`
    * c normal is defined as `1`
* b is for buffered memory and is dependent on the `PG_DEVICE` flag
    * b device is defined as `0`
    * b normal is defined as `1`
* nG is defined as global or a value of `0` for all pages

Now, mapping this to the new long-descriptor format, the following should be set:
* attrIndex will be set to an `MAIR` index based on the `PG_DEVICE` flag (it replaces the existing "c b tex s" combinations)
    * `MAIR` value `0000 0100` when the `PG-DEVICE` flag is  (MAIR index 2)
    * `MAIR` value `1011 1011` for normal memory (may want to change this to `1000 1000`...) -- MAIR index 0
* ns is set to `1`, meaning that this page is not secured
* tblNs is set to `1` meaning that this table is not secured
* ap is set to `01`, in alignment to the short-descriptor
* TblAp will be set to `00` to drive access permissions to the page
* sh is set to `11` to indicate that the page is inner sharable
* af will be set to `1` to allow access
* nG will be set to `0` in alignment to the short-descriptor
* pxn will be set:
    * `0` when PG_WRT is clear
    * `1` when PG_WRT is set
* tblPxn will be set to `0` to push the lookups to the page level
* xn will be set:
    * `0` when PG_WRT is clear
    * `1` when PG_WRT is set
* tblXn will be set to `0` to push the lookups to the page level

So this now results in the following bits for the long-descriptor flags `1000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0111 0110 0011` or in hex `8000 0000 0000 0763` for code.  The value becomes `8040 0000 0000 0763` for data.

---

OK, I am now working on an MMU problems with `SerialPutChar()` where it is looping waiting for room to be available.  I need to see if I can check the mappings.  So lets see here.... `0xf8215040` is the address we check for the status.  That is level 2 table `0x00106000` so that would be offset `0xe08` into that table.

All fixed up, I am back where I was with no change in behavior.  OK, looking at this page mapping, the bits are `8060 0000 3f21 576b`.  And this looks correct.  In particular, `6b` turns into `0110 1011` and masking out the attr bits, we are looking at `010` or `MAIR` index 2.  These bits are `04` and is device memory.  Let's try `00` for strongly ordered memory.

---

### 2020-May-25

OK, getting into `gdb` and attaching to `qemu`...  for CPU0 (that's a lot of shit in the CPU!):

```
(gdb) info reg
r0             0xc0                192
r1             0x81000664          -2130704796
r2             0xf8215054          -132034476
r3             0x60                96
r4             0x81000660          -2130704800
r5             0x20                32
r6             0x1                 1
r7             0x2                 2
r8             0xff800f74          -8384652
r9             0xff40a000          -12541952
r10            0xffc00000          -4194304
r11            0x8080c788          -2139043960
r12            0xf8215000          -132034560
sp             0xff800ef8          0xff800ef8
lr             0x808070bc          -2139066180
pc             0x808070fc          0x808070fc
cpsr           0x200001d3          536871379
fpscr          0x0                 0
fpsid          0x41023075          1090662517
fpexc          0x40000000          1073741824
MAIR0_S        0x0                 0
MAIR1_S        0x0                 0
CNTP_CTL_S     0x0                 0
MAIR0          0x400bb             262331
MAIR1          0x0                 0
CNTP_CTL       0x5                 5
DBGBVR_S       0x0                 0
DBGBCR_S       0x0                 0
DBGBVR         0x0                 0
DBGBCR         0x0                 0
DUMMY          0x0                 0
DUMMY_S        0x0                 0
MAIR_EL2_S     0x0                 0
HMAIR1_S       0x0                 0
CNTHP_CTL_EL2_S 0x0                0
MAIR_EL2       0x0                 0
HMAIR1         0x0                 0
CNTHP_CTL_EL2  0x0                 0
OSLSR_EL1_S    0xa                 10
OSLSR_EL1      0xa                 10
SCR_S          0x131               305
SDER_S         0x0                 0
AFSR0_EL1_S    0x0                 0
NSACR_S        0x0                 0
AFSR1_EL1_S    0x0                 0
SCR            0x131               305
AFSR0_EL1      0x0                 0
SDER           0x0                 0
NSACR          0x0                 0
AFSR1_EL1      0x0                 0
PAR_S          0x0                 0
PAR            0x0                 0
HCR_S          0x0                 0
MDCR_EL2_S     0x0                 0
AFSR0_EL2_S    0x0                 0
AFSR1_EL2_S    0x0                 0
CPTR_EL2_S     0x0                 0
HCR            0x0                 0
HSTR_EL2_S     0x0                 0
MDCR_EL2       0x0                 0
AFSR0_EL2      0x0                 0
CPTR_EL2       0x0                 0
AFSR1_EL2      0x0                 0
HSTR_EL2       0x0                 0
PMCCNTR_S      0x0                 0
AFSR0_EL3_S    0x0                 0
DBGDIDR_S      0x3515f005          890630149
AFSR1_EL3_S    0x0                 0
PMXEVCNTR_S    0x0                 0
PMCCNTR        0x0                 0
AFSR0_EL3      0x0                 0
DBGDIDR        0x3515f005          890630149
AFSR1_EL3      0x0                 0
PMXEVCNTR      0x0                 0
DBGBVR_S       0x0                 0
DBGBCR_S       0x0                 0
DBGWVR_S       0x0                 0
DBGBVR         0x0                 0
DBGWCR_S       0x0                 0
DBGBCR         0x0                 0
MIDR_S         0x410fc075          1091551349
DBGWVR         0x0                 0
CTR_S          0x84448003          -2075885565
DBGWCR         0x0                 0
TCMTR_S        0x0                 0
MIDR           0x410fc075          1091551349
TLBTR_S        0x0                 0
VBAR_S         0x0                 0
MVBAR_S        0x400               1024
CTR            0x84448003          -2075885565
TCMTR          0x0                 0
TLBTR          0x0                 0
MVBAR          0x400               1024
CLIDR_S        0xa200023           169869347
CLIDR          0xa200023           169869347
AMAIR0_S       0x0                 0
AIDR_S         0x0                 0
CSSELR_S       0x0                 0
AMAIR1_S       0x0                 0
CNTV_CTL_S     0x0                 0
AIDR           0x0                 0
CSSELR         0x0                 0
CNTV_CTL       0x0                 0
VBAR           0xff401000          -12578816
AMAIR0         0x0                 0
AMAIR1         0x0                 0
DUMMY_S        0x0                 0
DUMMY          0x0                 0
VPIDR_S        0x410fc075          1091551349
VPIDR          0x410fc075          1091551349
VBAR_EL2_S     0x0                 0
VMPIDR_S       0x80000f00          -2147479808
VBAR_EL2       0x0                 0
VMPIDR         0x80000f00          -2147479808
TEECR_S        0x0                 0
TEECR          0x0                 0
AMAIR_EL2_S    0x0                 0
HAMAIR1_S      0x0                 0
AMAIR_EL2      0x0                 0
HAMAIR1        0x0                 0
ESR_EL2_S      0x0                 0
ESR_EL2        0x0                 0
PMUSERENR_S    0x0                 0
PMINTENSET_S   0x0                 0
MDCCSR_EL0_S   0x0                 0
PMINTENCLR_S   0x0                 0
PMUSERENR      0x0                 0
PMINTENSET     0x0                 0
MDCCSR_EL0     0x0                 0
PMINTENCLR     0x0                 0
DBGBVR_S       0x0                 0
DBGDRAR_S      0x0                 0
DBGBCR_S       0x0                 0
DBGWVR_S       0x0                 0
DBGBVR         0x0                 0
DBGDRAR        0x0                 0
DBGWCR_S       0x0                 0
DBGBCR         0x0                 0
ID_PFR0_S      0x1131              4401
DBGWVR         0x0                 0
DBGWCR         0x0                 0
ID_DFR0_S      0x2010555           33621333
ID_PFR0        0x1131              4401
ID_AFR0_S      0x0                 0
ID_MMFR0_S     0x10101105          269488389
ID_DFR0        0x2010555           33621333
ID_MMFR1_S     0x40000000          1073741824
ID_AFR0        0x0                 0
ID_MMFR2_S     0x1240000           19136512
ID_MMFR0       0x10101105          269488389
ID_MMFR3_S     0x2102211           34611729
ID_MMFR1       0x40000000          1073741824
ID_MMFR2       0x1240000           19136512
ID_MMFR3       0x2102211           34611729
DUMMY_S        0x0                 0
DUMMY          0x0                 0
PAR_S          0x0                 0
PAR            0x0                 0
DACR_S         0x0                 0
DACR           0xffffffff          -1
CBAR_S         0x3f000000          1056964608
CBAR           0x3f000000          1056964608
MDSCR_EL1_S    0x0                 0
DBGBVR_S       0x0                 0
MDSCR_EL1      0x0                 0
DBGDSAR_S      0x0                 0
DBGBCR_S       0x0                 0
DBGWVR_S       0x0                 0
DBGBVR         0x0                 0
DBGDSAR        0x0                 0
DBGWCR_S       0x0                 0
DBGBCR         0x0                 0
ID_ISAR0_S     0x2101110           34607376
DBGWVR         0x0                 0
ID_ISAR1_S     0x13112111          319889681
DBGWCR         0x0                 0
ID_ISAR2_S     0x21232041          555950145
ID_ISAR0       0x2101110           34607376
ID_ISAR3_S     0x11112131          286335281
ID_ISAR1       0x13112111          319889681
ID_ISAR4_S     0x10011142          268505410
ID_ISAR2       0x21232041          555950145
ID_ISAR5_S     0x0                 0
ID_ISAR3       0x11112131          286335281
ID_MMFR4_S     0x0                 0
ID_ISAR4       0x10011142          268505410
ID_ISAR6_S     0x0                 0
ID_ISAR5       0x0                 0
ID_MMFR4       0x0                 0
ID_ISAR6       0x0                 0
TTBR0          0x1002000           16785408
TTBR1_S        0x0                 0
TTBR1          0x1002000           16785408
TTBR0_S        0x0                 0
HTTBR_S        0x0                 0
HTTBR          0x0                 0
VTTBR_S        0x0                 0
VTTBR          0x0                 0
CNTP_CVAL_S    0x0                 0
CNTP_CVAL      0x167aa25c          377135708
CNTV_CVAL_S    0x0                 0
CNTV_CVAL      0x0                 0
CNTVOFF_S      0x0                 0
CNTVOFF        0x0                 0
CNTHP_CVAL_S   0x0                 0
CNTHP_CVAL     0x0                 0
DBGDSAR_S      0x0                 0
DBGDSAR        0x0                 0
TTBR0_EL1_S    0x0                 0
TTBR1_EL1_S    0x0                 0
DFAR_S         0x0                 0
TTBR0_EL1      0x1002000           16785408
WFAR_S         0x0                 0
CNTFRQ_S       0x3b9aca0           62500000
TTBR1_EL1      0x1002000           16785408
DFAR           0x0                 0
IFAR_S         0x0                 0
WFAR           0x0                 0
TTBCR          0xb5003501          -1258277631
IFAR           0x0                 0
CNTFRQ         0x3b9aca0           62500000
DBGBVR_S       0x0                 0
DBGBCR_S       0x0                 0
DBGBVR         0x0                 0
DBGWVR_S       0x0                 0
DBGBCR         0x0                 0
DBGWCR_S       0x0                 0
DBGWVR         0x0                 0
TTBCR_S        0x0                 0
DBGWCR         0x0                 0
DUMMY          0x0                 0
FAR_EL2_S      0x0                 0
TCR_EL2_S      0x0                 0
HIFAR_S        0x0                 0
FAR_EL2        0x0                 0
TCR_EL2        0x0                 0
HPFAR_S        0x0                 0
HIFAR          0x0                 0
DUMMY_S        0x0                 0
HPFAR          0x0                 0
CNTKCTL_S      0x0                 0
DBGBVR_S       0x0                 0
DBGBCR_S       0x0                 0
DBGBVR         0x0                 0
DBGBCR         0x0                 0
CNTKCTL        0x0                 0
DUMMY          0x0                 0
VTCR_S         0x0                 0
CNTHCTL_EL2_S  0x3                 3
VTCR           0x0                 0
CNTHCTL_EL2    0x3                 3
DUMMY_S        0x0                 0
DBGDRAR_S      0x0                 0
DBGDRAR        0x0                 0
SCTLR_S        0xc50078            12910712
DFSR_S         0x0                 0
ACTLR_EL1_S    0x0                 0
CPACR_S        0xf00000            15728640
IFSR_S         0x0                 0
SCTLR          0xc5187d            12916861
FCSEIDR_S      0x0                 0
DFSR           0x0                 0
ACTLR_EL1      0x0                 0
CONTEXTIDR_S   0x0                 0
CPACR          0xf00000            15728640
IFSR           0x0                 0
TPIDRURW_S     0x0                 0
FCSEIDR        0x0                 0
TPIDRURO_S     0x0                 0
CONTEXTIDR_EL1 0x0                 0
TPIDRPRW_S     0x0                 0
TPIDRURW       0x0                 0
TPIDRURO       0x9000000c          -1879048180
TPIDRPRW       0x81000018          -2130706408
L2CTLR_S       0x3800000           58720256
L2ECTLR_S      0x0                 0
L2CTLR         0x3800000           58720256
L2ECTLR        0x0                 0
SCTLR_EL2_S    0x0                 0
ACTLR_EL2_S    0x0                 0
SCTLR_EL2      0x0                 0
ACTLR_EL2      0x0                 0
TPIDR_EL2_S    0x0                 0
TEEHBR_S       0x0                 0
TPIDR_EL2      0x0                 0
TEEHBR         0x0                 0
PMCR_S         0x41000000          1090519040
PMCNTENSET_S   0x0                 0
PMCNTENCLR_S   0x0                 0
PMCR           0x41000000          1090519040
PMOVSR_S       0x0                 0
PMCNTENSET     0x0                 0
PMCNTENCLR     0x0                 0
PMSELR_S       0x0                 0
PMOVSR         0x0                 0
PMSELR         0x0                 0
```

But, in particular, `cpsr 0x200001d3` shows that AIF are all masked.

For CPU1:

```
(gdb) thread 1
[Switching to thread 1 (Thread 1)]
#0  0x808070fc in ?? ()
(gdb) info reg
[snip]
sp             0xff800ef8          0xff800ef8
lr             0x808070bc          -2139066180
pc             0x808070fc          0x808070fc
cpsr           0x200001d3          536871379
```

This CPU is trying to output a character to the serial port.  This is done with interrupts disabled.

```
sp             0xff801fe0          0xff801fe0
lr             0x80807f08          -2139062520
pc             0x80807f10          0x80807f10
cpsr           0x60000113          1610613011
```

CPU2 is waiting at the synchronization barrier, and interrupts are enabled!  And CPU3 is in the same state as CPU2.

So, OK, where is CPU0?  I thought this was running on CPU0, but I may be wrong.  That CPU is in the same state as CPU2 and CPU3.

After disabling the other CPUs, it looks like the timer is not being programmed properly and it is not generating a signal.

I think the best thing to do at this point is to go back to the beginning and confirm that all my code is working.  Something is happening before the kernel that is preventing the arch from working.  This is likely to involved a large dump of registers and MMIO addresses.

---

OK, I think I am going to have to write a purpose-built system to check out what is going on with the system.  Before I go there, I want to commit my changes and try v0.6.1 again to see if I can get it to boot (I need to rule out hardware as a problem).

---

So, I had problems with that version for a while as well.  I ended up reloading a different microSD card with the same `pi-bootloader` software and I was able to get it to load far more regularly.  I was also having problems with the `ftdi-sio` module on my host system.  So, that said, I still do not have a root cause for all my problems.  The only thing I think I can do regularly to resolve my problems is to exit the shell and launch a new one.  And even at that, it is not consistent.  I honestly believe it is the rpi at this point.

---

### 2020-Jun-06

I have been away for a bit.  My dedicated test worked very well, and I have no problems with it.  It is working as expected.

I ordered and received a new rpi2b.  The one I have been developing against was `'Rev 1.1'` whereas the new one that was delivered was `'Rev 1.2'`.  I looked it up and the difference is the SoC, where the 1.2 version has the BCM2837 SoC which is a Cortex-A53 CPU -- a 64-bit CPU.

I think, with this, I am no longer going to be able to develop the ARM 32-bit OS and be able to test it on real hardware.  I believe it is going to be time to move on to the 64-bit OS for both ARM and x86.  I may come back around to these archs later, but for now I think it is time to move on.  I will, however, make an attempt to keep the 32-bit archs up to date if I find anything that would impact them.

So, with this version being dedicated to revisiting User Mode (which I believe I have working on x86-pc), I am going to have to bookmark this and tickle the micro-version for the new archs.
