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

OK, I have the same behavior on the old laptop on the current version...  but I also know it works on version 0.4.2.  So, I definitely broke something.

OK, maybe I'm missing a `volatile` keyword for the `schedulerLockCount` member...  but I had that problem before I made the change....

---

One more test reveals I have the same problem with v0.4.2 and it really is a preemption problem.  So, I need to be able to get to the bottom of that first.

In Bochs, I am getting a double fault:

```
So, the calculated clock divider is 0x000000b5
?
Double Fault
EAX: 0x00101ab0  EBX: 0x00000000  ECX: 0x80004750
EDX: 0x00000020  ESI: 0x00103100  EDI: 0x003fd800
EBP: 0x00000000  ESP: 0xff800fc0  SS: 0x10
EIP: 0x00000008  EFLAGS: 0x800b6000
CS: 0x200292  DS: 0x10  ES: 0x10  FS: 0x10  GS: 0x10
CR0: 0xe0000011  CR2: 0x00000000  CR3: 0x003fe000
Trap: 0x8  Error: 0x80004ba4
```

The `EIP` is interesting to be sure.  But the error code is odd as well.  `0x80004ba4` happens to be a good instruction address right after `EnableInterrupts()` in `kInit()`.  In addition, the `'?'` character indicates that I am servicing an interrupt of some sort.

A stack analysis is in order here:

```
<bochs:6> x /64 0xff800f4c
[bochs]:
0x00000000ff800f4c <bogus+       0>:    0x80001b74      0x80007e10      0x00000008      0x80004ba4
0x00000000ff800f5c <bogus+      16>:    0x003fe000      0x00103100      0x003fd800      0x00000000
0x00000000ff800f6c <bogus+      32>:    0x80001ba9      0xff800f90      0x8000840d      0xffffb000
0x00000000ff800f7c <bogus+      48>:    0x00000200      0x0000002f      0xff800fbc      0x000000b5
0x00000000ff800f8c <bogus+      64>:    0x80001aa0      0x00000010      0x00000010      0x00000010
0x00000000ff800f9c <bogus+      80>:    0x00000010      0x00000010      0x003fe000      0x00000000
0x00000000ff800fac <bogus+      96>:    0xe0000011      0x003fd800      0x00103100      0x00000000
0x00000000ff800fbc <bogus+     112>:    0xff800fc0      0x00000000      0x00000020      0x80004750
0x00000000ff800fcc <bogus+     128>:    0x00101ab0      0x00000008      0x00000000      0x80004ba4
0x00000000ff800fdc <bogus+     144>:    0x00000008      0x00200292      0x800b6000      0x000003e8
0x00000000ff800fec <bogus+     160>:    0x00000000      0x00000000      0x00000000      0x00000000
0x00000000ff800ffc <bogus+     176>:    0x00ee0000
```

| Address  | Contents | Commentary |
|:--------:|:--------:|:-----------|
| ff800ffc | 00ee0000 | |
| ff800ff8 | 00000000 | |
| ff800ff4 | 00000000 | |
| ff800ff0 | 00000000 | |
| ff800fec | 00000000 | |
| ff800fe8 | 000003e8 | |
| ff800fe4 | 800b6000 | |
| ff800fe0 | 00200292 | |
| ff800fdc | 00000008 | |
| ff800fd8 | 80004ba4 | |
| ff800fd4 | 00000000 | |
| ff800fd0 | 00000008 | |
| ff800fcc | 00101ab0 | |
| ff800fc8 | 80004750 | |
| ff800fc4 | 00000020 | |
| ff800fc0 | 00000000 | |
| ff800fbc | ff800fc0 | |
| ff800fb8 | 00000000 | |
| ff800fb4 | 00103100 | |
| ff800fb0 | 003fd800 | |
| ff800fac | e0000011 | |
| ff800fa8 | 00000000 | |
| ff800fa4 | 003fe000 | |
| ff800fa0 | 00000010 | |
| ff800f9c | 00000010 | |
| ff800f98 | 00000010 | |
| ff800f94 | 00000010 | |
| ff800f90 | 00000010 | |
| ff800f8c | 80001aa0 | |
| ff800f88 | 000000b5 | |
| ff800f84 | ff800fbc | |
| ff800f80 | 0000002f | |
| ff800f7c | 00000200 | |
| ff800f78 | ffffb000 | |
| ff800f74 | 8000840d | |
| ff800f70 | ff800f90 | |
| ff800f6c | 80001ba9 | |
| ff800f68 | 00000000 | |
| ff800f64 | 003fd800 | |
| ff800f60 | 00103100 | |
| ff800f5c | 003fe000 | |
| ff800f58 | 80004ba4 | |
| ff800f54 | 00000008 | This is expected to be a pointer to an IsrReg_t structure |
| ff800f50 | 80007e10 | Pointer in .rodata |
| ff800f4c | 80001b74 | Return point after calling Halt(), which will never happen |

---

### 2019-May-08

Continuation of the stack analysis:

| Address  | Contents | Commentary |
|:--------:|:--------:|:-----------|
| ff800ffc | 00ee0000 | |
| ff800ff8 | 00000000 | |
| ff800ff4 | 00000000 | |
| ff800ff0 | 00000000 | |
| ff800fec | 00000000 | |
| ff800fe8 | 000003e8 | |
| ff800fe4 | 800b6000 | |
| ff800fe0 | 00200292 | |
| ff800fdc | 00000008 | CS: |
| ff800fd8 | 80004ba4 | EIP: This is the address after calling `EnableInterrupts()` from `kInit()` |
| ff800fd4 | 00000000 | Error Code |
| ff800fd0 | 00000008 | Int Number -- double fault |
| ff800fcc | 00101ab0 | EAX: |
| ff800fc8 | 80004750 | ECX: |
| ff800fc4 | 00000020 | EDX: |
| ff800fc0 | 00000000 | EBX: |
| ff800fbc | ff800fc0 | ESP: |
| ff800fb8 | 00000000 | EBP: |
| ff800fb4 | 00103100 | ESI: |
| ff800fb0 | 003fd800 | EDI: |
| ff800fac | e0000011 | CR0: |
| ff800fa8 | 00000000 | CR2: |
| ff800fa4 | 003fe000 | CR3: |
| ff800fa0 | 00000010 | DS: |
| ff800f9c | 00000010 | ES: |
| ff800f98 | 00000010 | FS: |
| ff800f94 | 00000010 | GS: |
| ff800f90 | 00000010 | SS: |
| ff800f8c | 80001aa0 | `IsrCommonStub` return after call to `IsrHandler()` |
| ff800f88 | 000000b5 | sub 0x18; |
| ff800f84 | ff800fbc | sub 0x18; |
| ff800f80 | 0000002f | sub 0x18; |
| ff800f7c | 00000200 | sub 0x18; add 0x10; sub 0x0c; |
| ff800f78 | ffffb000 | sub 0x18; add 0x10; sub 0x0c; |
| ff800f74 | 8000840d | sub 0x18; add 0x10; sub 0x0c; |
| ff800f70 | ff800f90 | push 0x80007e3c; add 0x10; push EDX == `isrRegs_t` pointer |
| ff800f6c | 80001ba9 | return from `IsrHandler()` call assume to Isr08... |
| ff800f68 | 00000000 | IsrDumpState() |
| ff800f64 | 003fd800 | IsrDumpState() |
| ff800f60 | 00103100 | IsrDumpState() |
| ff800f5c | 003fe000 | IsrDumpState() |
| ff800f58 | 80004ba4 | IsrDumpState() -- last line output |
| ff800f54 | 00000008 | IsrDumpState() -- last line output |
| ff800f50 | 80007e10 | Pointer in .rodata (a 0-byte string?) |
| ff800f4c | 80001b74 | Return point after calling Halt(), which will never happen |

---

### 2019-May-09

Ok..  Double Fault not ending in a triple fault.

According to the Intel Guide, there are only a few conbinations that can end in a #DF.  First of all, the first fault needs to be a Contributory Exception class (0, 10, 11, 12, 13) or a Page Fault (14).  And then the second fault must be Contributory (or in the case of a Page Fault, also another Page Fault).

So, the things I need to look at for the triggering even are:
* #DE -- Divide Error (0)
* #TS -- Invalid TSS (10)
* #NP -- Segment Not Present (11)
* #SS -- Stack Fault (12)
* #GP -- General Protection (13)
* #PF -- Page Fault (14)

From Bochs, I am able to determine I am getting an interrupt and then somewhere in handling that interrupt I am having a problem.

I think what I need to do is to create an specific interrupt to dump the system state for debugging purposes -- maybe a special-purpose Panic software interrupt.  This would be the Century-OS version of a BSOD, but I would dump to the serial port for now.

Could it be that my ready queue implementation is not being properly initialized??

---

Well, it turns out my `xadd` functions return the *previous* value, which I neglected to remember in my atomic int implementation.

---

### 2019-May-10

Today (and tomorrow), I will be working on getting the semaphores working properly.  At the same time, I am very convinced that the scheduler is still very fragile and needs to be shored up.  I will take that on at the end of this debugging effort.

---

### 2019-May-11

I have been able to debug the basis of the semaphores....  I want to get a few more tests in before I wrap this up.

---

So, with that all (well, I believe all) sorted out, my scheduling code is still exceptionally fragile.  I need to get that sorted out, and I am going to take that on now.

There are a few things I can observe depending on the when I run the code:
* On Bochs, I end with a double fault (but not a triple)
* On real hardware, I end with a triple fault (but I cannot get logs because I do not have a cable)
* On QEMU, the timer stops firing (or never really fires to begin with)

(Trying to get a qemu log is near impossible -- it ran for 30 minutes, created a 10GB log, and had not even gotten to Grub yet.)

So, I need to focus on the Double Fault first, since that has the best toolset for the situation and probably the most damning error.

---

### 2019-May-12

I am working on that double fault....  My goal is to find the root cause of the fault.  Some things as I test:

* If I disable the additional processes (only the timer firing, no task changes), I still get the #DF.
* If I then return to the PIT, I end up with an interrupt 0x0f...??
* If I then also change the PIC to be the 8259 PIC, it works.
* And then if I return to the Local APIC for the timer, it also works.
* So, the problem appears to be in my IOACPIC implementation.

---

Hmmm....  is this right??  It looks like I am not incrementing the address pooperly.

```
00172985639d[IOAPIC] IOAPIC: write aligned addr=fec00000, data=00000000
00172985640d[IOAPIC] IOAPIC: read aligned addr=0x0000fec00010
00172985641d[IOAPIC] IOAPIC: write aligned addr=fec00000, data=00000001
00172985643d[IOAPIC] IOAPIC: read aligned addr=0x0000fec00010
00173091828d[IOAPIC] IOAPIC: write aligned addr=fec00000, data=00000010
00173091829d[IOAPIC] IOAPIC: read aligned addr=0x0000fec00010
00173091830d[IOAPIC] IOAPIC: write aligned addr=fec00000, data=00000011
00173091831d[IOAPIC] IOAPIC: read aligned addr=0x0000fec00010
00173147977d[IOAPIC] IOAPIC: write aligned addr=fec00000, data=00000010
00173147978d[IOAPIC] IOAPIC: read aligned addr=0x0000fec00010
00173147979d[IOAPIC] IOAPIC: write aligned addr=fec00000, data=00000011
```

---

Reviewing the document, I think I have the redirection table set incorrectly -- which is what is causing the double fault.

I need to review this to determine why this is not working properly.

---

I was able to determine that the IOAPIC was not being programmed properly for the timer (I was actually skipping that step).  But, fixing that had no effect on the behavior.

---

So, it looks like my IRQ translaction implementation is not quite right.  There are differences in what entry/location is used relative to any given IRQ.  This is what I need to clean up.  In short, I need to go through both PIC implementations and make sure I have a proper translation from the IRQ number to the location proper to each implementation.

---

I posted this question to `freenode#osdev`:

> I am having a problem with the APIC.  When I route the LAPIC timer through the 8259 PIC, things work; when I route wither the 8253 PIT or the LAPIC timer through the IOAPIC, I get a double fault (but not a triple).  When I do not enable interrupts things obviously do not fault.  As best as I can figure, I have something wrong in the code that sets up the redirection table, but damned if I cannot figure it out.  Any suggestions on what to look at?

---

### 2019-May-13

I figured it out.  The 8259 was not remapped and something was sneaking through.  Not exactualy sure what on Bochs...  Now, I have Bochs working perfectly.

QEMU on the other hand works for a short time and then I get burried in timer interrupts -- almost like the EOI was not received or I am getting spurrious interrupts to the timer IRQ.  Actually, QEMU is working right as well -- the timer frequency is a bit high for that emulator.

---

### 2019-May-14

OK, to clean this all up, I need to set some additional interrupt handlers.  I have moved the PIC to a range way out of the way, but there is a risk still of something sneaking through, and while I have the IRQs mapped out of the way there is no handler to take the load just in case.  I need to pull that together.  Which is now complete -- any IRQ from the 8259 will hit interrupts 240-255 and will be treated as a spurious interrupt.  There is room for improvement here -- to remove the individual entry points, but for now it will work since I do not yet have a better solution for that arm arch.

---

I think this wraps up this version, and it's time for a commit.

