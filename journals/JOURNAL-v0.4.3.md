# The Century OS Development Journal - v0.4.3

## Version 0.4.3

The 0.4.x series of versions will enable SMP on both architectures.  This is all new things for me, so I do expect this to take a lot of work to get dialled in.

There is a roadmap I have developed:
* v0.4.0 -- enable caches and branch prediction on the arm cpu; and debug the resulting mess
* v0.4.1 -- replace the 8259 PIC with the Local APIC on x86-pc arch
* v0.4.2 -- implement atomic integers and replace the foundation of `Spinlock_t` implementation with `AtomicInt_t`
* v0.4.3 -- implement inter-process communication (IPC) in near-compliance with Posix -- these will be synchronization primitives
* v0.4.4 -- implement messaging in near-compliance with Posix
* v0.4.5 -- implement inter-processor interrupts and CPU-specific memory/structures
* v0.4.6 -- finally enable the APs; cleanup the resulting mess

---

### 2019-Apr-29

To start with, this is going to turn into a rabbit hole initially.  I have quite a bit of work to do to build out the basic Posix compliance.  Here are my notes as I read about Semaphores:
* The basic functionality of a semaphore will be provided by the prototypes defined [here](http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/semaphore.h.html).  This defines the API implementation.
* I believe the System calls for Semaphores will be based on what is defined [here](http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/sys_sem.h.html).  This defines the system implementation.
    * Therefore, the kernel will implement the functions from this header at the core of the semaphore implementation, but may support the calls from the prior header to aid on the readability of code.
* It appears that I will be able to define `sem_t` on my own, but the API functions will return a pointer to this structure -- but these should be assembled by the user-space side of the system call.
* The [Linux syscalls(2) page](http://man7.org/linux/man-pages/man2/syscalls.2.html) shows that the calls from `sys/sem.h` are indeed the system calls.

So, it is going to be the system calls that I am targeting at this point.  These will be:
* semget (see `man semget`)
* semop (see `man semop`)
* semctl (see `man semctl`)

Now, I also installed the `moreutils` package on my host which provides the `errno` command.  This command gives a description of any particular error number (very brief) ot lists the error codes.

This task is also going to begin to build out the user runtime library (well, the required include files at least).  This means that the bin build will now have an `usr/include` folder and will copy the files from the source tree.

One final note here: the actual system call name is from the user-space perspective.  I am going to implement semaphores functions that match `Semaphore.*()`, but will conform to the API definition.  This will allow me to bind the API to the functions without name conflicts.

Ultimately, I will need to start with the header files to get all the definitions settled.

I am going to start with `sys/sem.h`.  However, I still expect that rabbit hole and will likely end up adding additional header files very quickly.

---

### 2019-Apr-30

> Flags recognized in sem_flg are `IPC_NOWAIT` and `SEM_UNDO`.  If an operation specifies `SEM_UNDO`, it will be automatically undone when the process terminates.

Well, `IPC_NOWAIT` is defined in `sys/ipc.h`, so I think I better get that file completed first, since the `IPC_NOWAIT` and `SEM_UNDO` values need to mesh with each other.

---

OK, I have both those files written for now.  The next step is for me to develop the kernel's `semaphore.h` file, which will bring in only the symbolic constants from these tables.

---

I have the kernel header ready to go plus I have the initialization function written and integrated.  One thing I am considering is whether should create a special purpose key for the kernel-related semaphores.  Not sure what those would be at the moment, but it might be a good idea.  I think I will table that for now.

---

I was able to get the `SempahoreGet()` function written and it compiles.  No testing completed yet....  That will have to wait until tomorrow.

---

### 2019-May-01

At some point last night, I thought I could improve performance by releasing the lock a little early with a placeholder for the semSlot I was going to populate.  I may yet be able to do that, but I will have to check for a NULL value and re-acquire the lock if I need to change.  In short, at this point I am not going to do that.

So, I am now left with `SemaphoreControl()` and `SemaphoreOperations()`.  Well, `SemaphoreOperations()` has some atomicy requirements that I am not sure how I am going to handle yet.  So, I go with `SemaphoreControl()` first.

Both need to be able to determine user permissions in order to operate.  Currently, in the implementation thus far, all users have all permissions.  That is not to say that this will not change at some point.  To build this out, I will include a function to check the effective permissions for a user given the uid and the permissions map.  This function will simply return that the user has permissions.  However, with this function, I should have the API built to change that in the future.  This function (well, macro) is named `UserHasPermission()`.

So now to `SemaphoreControl()`....

So, `SemaphoreControl()` has a return code if the address of the data structure is not mapped -- EFAULT.  This means I need to be able to check for a valid mapping (and writable).  Now, this should theoretically be in user space, but may not be if I use this from the kernel.  I will forego this check for now.  In the meantime, I will divert to write `MmuIsMapped()`.

---

### 2019-May-02

Well, I have a new host system I am building out.  One of the things I want to be able to do is to script the tools build.  There are some minimum things that will need to be installed ahead of time, but after that I want the rest to be automated.

For now, I will only really need to worry about the `i686-elf` and `armv7-rpi2-linux-gnueabihf` toolchains.

---

As usual, I am having a fun time with dependency hell.  This should make things much smoother for everyone else.  I hope anyway.

---

Well, I am running FC30 on this new laptop and it has a problem with the foundational tools.  So, I abandoned the script and decided instead to copy the tools over.

This was the whole of my evening.  I am going to work on getting to a commit point on this system and then the next time I check my work in I am going to move to the other system.

---

### 2019-May-03

OK, I am at the point where I need to begin to think about the `SEM_UNDO` flag.  There is a `semadj` value that is stored between the process and the semaphore.  When the process ends, the undo needs to be adjusted; when the semaphore is destroyed, the `semadj` needs to be cleaned up.

Now, this is a many-to-many relationship.

To deal with this, it does not make a ton of sense to associate this `semadj` value with the process and it makes even less sense to associate this `semadj` value with the semaphore.  I can see problems with both implementations.  I might be able to get away with the process association if I get cute.

The problem I am working through is not about what happens when a process terminates, but what happens when a semaphore set is removed out from under a process....  The `semid` might be replaced later with a new set.

Now, on the other hand, I might be able to generate a (somewhat) unique `semid` and store that rather than as the index into an array.  This will likely be a better choice -- but requires some rework.  But it will solve the problem with the process undoing the semaphore actions after `IPC_RMID` -- if the `semid` cannot be found, there is nothing to undo.

Now, with that said, when I have operations like `SETALL` and `SETVAL`, these need to be able to associate with the semaphore, and I cannot go on a hunt for all the processes to clean up the lists.

Sounds like I am really going to need a list and a separate structure.  Wow!  Full circle!

I think the best staring point is going to be to encapsulate the entire semaphore implementation into a structure, like the `scheduler`.

---

I was able to get `SemaphoreControl()` completely written, so it is time to commit this code (and switch laptops).

---

OK, I am moved over, but the cross compilers are not working.  It looks like I am going to be forced to rebuild them.

---

### 2019-May-04

May the fourth be with you!

OK, I have the tools chain built.  It is not perfect, so I am going to have to clean it up and do it again -- several of the filesystem attributes for the toolchain were not set right.  However, I am able to do a `make toolchain` and end up with a all the different tools I need prepared.  There are a few things missing and I will end up testing this on a clean VM in the coming weeks....

I believe that there are some optimizations that I can do with the `SemaphoreCommand()` function -- particularly when managing the undo list.  In short, I really need to create an API for that which will allow me to update it if I come across a better method to update it.

---

I am now into the `semop()` work.  This will be a bit more complicated.  It will have to be implemented in 2 parts -- part 1 will get the lock on the semaphore set and check to see if all the operations can be done, returning the proper return value if it is not possible.  Part 2 will actually complete the actions and will block if not all the conditions are met and will check all the conditions again when the process unblocks.

The key attribute here it that all of these actions are to be done atomically -- it's an all or nothing update -- and all the conditions need to be met at the same time.

I think I am going to need a lower-level function to iterate the semaphore operations list... with a flag to indicate if we execute the changes or not.  We would execute this function once to check (with the lock held) if everything is good, and if it is all good excxcute the function call again (without releaing the lock) to actually execute the changes.  If we are not able to execute them all, then we release the lock and return with the reason why.

When I boil this down, there are really only 3 conditions that can happen:
1. We are able to execute all the operations and we do not block, so we execute the operations and return.
1. We are able to execute all the operations but we will block, so we execute none of them so we queue and block.
1. We are not able to execute all the operations or would block and the `IPC_NOWAIT` flag is set, so we return with proper error code.

Now, this is also going to bring up a point -- when a process blocks, we need to put it on the blocked list, which is different than the list of processes waiting for something to happen.  I have been using `Process_t.stsQueue` to hold this info, but that is not going to work.  I am going to need to come up with another structure to hold this data, likely with a pointer to the `Process_t` structure.  I am also going to have to mimic the scheduler functions by postponing the scheduling until all this work is completed.

---

### 2019-May-05

Work in a top-down design pattern, I was able to get the bulk of `SempahoreOperations()` and the `SemIterateOps()`.  There are several functions that are called but have not been defined.  These are:
* `SemBlock()` -- This function will create entries on the proper wait lists and add them to the proper semaphores.  It will not actually block the running process.
* `SemReadyWaiting()` -- This function will ready all processes in the list waiting for 0 or for the `semval` to increase.  A parameter will determine which to wake.  This function will need to be close the a scheduler function -- Postponing a Reschedule until all the work is complete.
* `SemCreateUndo()` -- This function will find and update an undo element or create one if needed and inserting it into the list.

To get started, I need to put together a structure to hold the processes that are waiting on a Semaphore.  Well, more generic for an IPC I think.

---

I think I have my kernel semaphore implementation complete.  I need to figure out how to test it, but it other than a few variances so far, I have a POSIX compliant semaphore implementation.

Time to commit.

---

POSIX states about semaphores:

> A minimum synchronization primitive to serve as a basis for more complex synchronization mechanisms to be defined by the application program.

This means that there is no required mutex implementation required by POSIX.  I just need to figure out how to test and then this will be done.

---

OK, so thinking about this, I am not going to implement mutexes -- I will leave that as an exercise for application space.  Therefore, a status of MTXW is not relevant.  I may need to change the process status to `PROC_IPCW` for waiting for some IPC element.

As for testing, I really am not feeling it today.  I am having to force myself to do it.  I think the first test is going to be to whittle things down to `ProcessA` and `ProcessB` and set up an up/down scenario where `A` will increase a semaphore to 1, write an 'A', and wait for 0; and `B` will write a 'B', decrease the same semaphore to 0, and wait for 1.  This way, I should have synchronized A's and B's across the screen.

---

OK, so I have nothing that is swapping properly.  I get a few interrupts and then I am never getting to process A.  So, I certainly need to get to the bottom of that....

I am getting timer interrupts regularly because the `kInit()` task is continuing.  However, I am not getting timer IRQs since the `TimerCallBack()` function is not getting control.  I actually get 2 of them and the nothing.  Perhaps I am getting spurious IRQs?  No, that does not appear to be it.

The next thing to look at is whether I am issuing an EOI for every IRQ.  This is working right as well....

```
@;@;@;@;@;@;Semapho@;@;reGet() offered semid 0x0
@;@;............
```

So, the `'@'` characters are a timer IRQ and the `';'` characters are the EOI; the `'.'` characters are the `kInit()` loop.  All of a sudden the IRQs are no longer firing and only the `kInit()` loop has control.  But something is waking up the CPU.

I have been able to determine I am not making it into `ProcessSchedule()`....

So, I am wondering if my problem is that I am not issuing an EOI in `ProcessStart()`.  I believe I had a change in condition where before I was voluntarily relinquishing control for the first task swap and now I am expecting the timer interrupt to handle this for me.  This test did not pan out either.

So....  The timer is firing a few times and then quitting.  I think I am going to have to put this through the qemu logs and see what I can come up with....  Not tonight.

---

### 2019-May-06

I am having trouble with swapping tasks again -- and more than likely with issuing a proper EOI.  At any rate, my scheduler seems exceptionally fragile.

Before I can do anything with the semaphores, I really need to get this all worked out.  Properly.

So, there are effectively 2 methods for a changing tasks.  These are:
1. Blocking via a function call
1. Preemption via timer IRQ

From the perspective of a process, there are 4 states to consider:
1. Starting a process
1. Blocking by function call
1. Preemption by timer IRQ
1. Terminating normally

Finally there are both entry and exit paths that need to be considered -- especially related to enabling interrupts and issuing EOI.

However, let's start with getting the timer working properly again.  I have all the code in the `TimerCallBack()` function commented out -- so we should get some interrupts without any task changing.  The timer is not working properly.

OK, I was able to narrow my problem down to `ProcessCreate()`.  And in this, it appears to be related to the `ProcessEnterPostpone()` and `ProcessExitPostpone()` function pair.  Actually those 2 functions really need to be updated to use the `AtomicInt_t` type.

I will have to take some of this on tomorrow.

---

### 2019-May-07

First order of business today is to take a look at the Process Enter and Postpose functions -- cleaning up the lock with an `AtomicInt_t`.

---

Hmmmm... maybe the timer is not getting reloaded -- acting like a one-shot?

OK, so, with this line of code:

```C
kprintf(".(%p)", (uint32_t)TimerCurrentCount(timerControl));
```

I am getting the following output:

```
.(0x00000000).(0x00000000).(0x00000000).(0x00000000).(0x00000000).(0x00000000).(0x00000000).(0x00000000).(0x00000000).(0x00000000).(0x00000000).(0x00000000).(0x00000000).(0x00000000).(0x00000000).(0x00000000).(0x00000000).(0x00000000).(0x00000000).(0x00000000).(0x00000000).(0x00000000).(0x00000000).(0x00000000).(0x00000000).(0x00000000).(0x00000000).(0x00000000).(0x00000000).(0x00000000).(0x00000000).(0x00000000).(0x00000000).(0x00000000).(0x00000000).(0x00000000).(0x00000000).(0x00000000).(0x00000000)
```

This tells me that I am not getting the timer reset properly -- and the timer is acting more like a one-shot than a periodic timer.

How about rpi2b?

---

OMG!!!  Finally!

Well, I have the same behavior on rpi as well.

Assuming that this worked in v0.4.2, the only material change here is the addition of `pendingErrNo` to the `Process_t` structure.  Other than that, the toolchains were rebuilt.

I'm going to test this on my old laptop, so I need an interim commit.




