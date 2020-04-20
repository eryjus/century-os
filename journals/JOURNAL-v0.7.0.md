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



