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
* the timeslice is set to be the priority -- so the higher the priority the longer it gets to stay on the CPU.

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


