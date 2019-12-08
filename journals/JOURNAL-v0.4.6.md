# The Century OS Development Journal - v0.4.6

## Version 0.4.6

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

### 2019-Jun-08

OK, it is time to start making use of the additional CPUs or cores.  I am going to start with the rpi2b platform.

So, for the APs, we are going to go off to another entry file (`entryAp.s`).  In this file each AP will wait until there is "permission" to continue, at which point, we will get a stack and start running some code.

The problem with the stack is that we cannot reuse the stack from CPU0.  For CPU0, we are starting with the stack at `0x8000`.  This persists until we jump to the kernel, `kinit()`, when the stack is replaced at that point.  So, what I really need in this case is a method to allocate stacks.  It should be as easy as taking the cpu number and setting an offset, but the AP does not have enough configuration to be able to map a stack into the MMU.  CPU0 will have to be responsible for this task during `MmuInit()`.

So, with that done, it is only a matter of setting a stack location before the first function call.

Now, all I need to do is jump to the `kInitAp()` function and then figure out what to do from there.  One question I have is whether the VBAR is a per-CPU register or a per-system register.

---

### 2019-Jun-09

Asked on `freenode#osdev` last night:

```
[21:15:54] <eryjus_> for armv7, am I correct that every cp15 register is duplicated as per-cpu registers?  or are any per-system registers?
[21:22:53] <geist> eryjus_: hmm. i'd like to say yep, but that's probably not true in all cases
[21:23:16] <geist> all the architectural ones are yes, but there are some systemwide control registers that may pop up in there depending on which core/soc you have
[21:23:38] <geist> like, say, if the GIC has its registers mapped into the cp15 space, which i think are in some cases
[21:23:58] <geist> for the most part yes though, all of the standard arm control registers are private
```

---

So, the problem I am having is that I think I am doing all the steps necessary to get the other cores up and running and I am not yet seeing them get to the point where i can see output on the serial port.  Now, there are only a couple of places where this can go wrong -- `pi-bootloader` may not be doing this right or the `entry` may not be doing this right in the kernel.  What I really want to be able to do is to interrogave the core state across cores so that CORE0 can interrogate the status of CORE1 and report it to me on the serial port, but I cannot imagine that can happen.

---

I do not think I am going to get there from here...  I am going to have to write some purpose-built code for testing/debugging.  The question I have is: can I write this code to boot from `pi-bootloader`?  Or do I need to do a replacement for that and go back to a bare-metal test?

---

After considering it while I was doing yard work, I think I am going to work from `pi-bootloader`.  There are a couple of reasons...  1) I will not have to reload the microSD chip with every code change and test, and 2) it is the eventual architecture it will run on.

---

OK, it looks like I am going to have to work completely with the bare metal...  Hmmmm....  https://stackoverflow.com/questions/28610045/how-to-start-additional-cores-on-the-raspberry-pi-2-under-bare-metal

---

OK, I was able to get this working while still using `pi-bootloader`.  The sequce is as documented in the above web site, but then send an `sev` to wake up all the cores from a `wfe` and any core that does not have a starting address in Mailbox3 loops back around to wait.

Now, on the other hand, I am going to have to do a bit of recoding to make this all work right.  That will be tomorrow.

---

### 2019-Jun-10

So, at this point, I need to revamp my startup code to make sure that the secondary cores (as they are referred to in the ARM environment) have a starting point and can get initialized properly.  The one thing I am able to do is to start each core separately, which is nice -- it should allow me to work on some basic SMP stuff before I have all 4 cores in the mix making things worse.

---

My first attempt is very unsuccessful.  I think i am going to need to do some non-paging testing to make sure I am really getting the core started.

---

### 2019-Jun-11

I have been able to confirm that core1 is actually being started.  I set a global flag that stopped core0 from printing dots (`'.'`) to the serial port and that worked.

The problem I think I am working with is that the serial port is not protected by a lock and both cores are trying to write at the same time.  So the problem becomes how to deal with this.  If I set it by lock, I cannot output anything during an exception.  This may not be horrible at this point, but I am not confident that I will not need that feature in the near future.  I can start a process to receive messages that outputs to the serial port and handle those in sequence (which might be nice to decorate with date/time/ticks).

From `freenode#osdev`:

```
[17:18:48] <eryjus> Please check my thinking...  I now have 2 cores operating and both cores are trying to output to the serial port with no lock to protect it.  Obvious problem there.  Now to solve this I can see 2 options -- 1) protect the serial port with a lock; but that will eliminate any option to output from an interrupt since an interrupt may happen while a lock is held; or, 2) start a separate process to output to the serial port and receive
[17:18:48] <eryjus> messages from any other process to perform this output.  Since I tend to overthink/overengineer things, is there a simpler third option I am missing?
[17:19:24] <zid> Make a queue
[17:19:40] <eryjus> feels the same as option 2
[17:19:59] <zid> doesn't have to be a seperate process, but a lock on a list >>> a lock on a device
[17:20:11] <zid> you only need to take it to update a single pointer
[17:20:50] <eryjus> hmmm....  let the timer perform the output to the serial port..??
[17:21:00] <zid> Could do, just the system maint task or whatever
```

I think I prefer my own option 2 over zid's because I really need an excuse to properly vet the messaging code.  It was not yet tested at all.

---

OK, so the problem with the process solution is that I have so much debugging output ahead of enabling interrupts (scheduling) and it may not be a good solution.  On the other hand, most of that might be able to be commented out or conditionally compiled since most of what I am working on is past that point anyway.

The thing I am realizing is that the messaging uses the heap and the heap is also controlled by locks.  And this again leaves me unable to produce any debugging output from an interrupt.

I can avoid the deadlock easy enough by testing the spinlock.

Perhaps zid is correct in that I can allocate a large queue for the timer interrupt and append to that queue while in the interrupt and then, when able, send a message to the serial output driver.  That's the only way I can figure out how to mitigate the race condition.  At the same time, I will need to have a queue for the heap debugger (since the heap will want to allocate memory when the lock is held -- deadlock!) and the queue will be passed off to the serial driver on exit from the heap functions.

Now, what to do about the early and intermediate serial output?

I obviously have some more thinking to do....

It's clear to me that the atomic operation needed here is something like `SerialQueueChar()`.  This function would only need to fill in a character and a pointer and increment the pointer atomically.  Something like `*p ++ = char;`.

---

Hmmm......


```
[19:41:59] <doug16k> you disable IRQ so you won't get interrupted while holding the lock, because the ISR will wait forever for the preempted code to release it
[19:42:10] <doug16k> if the other cpu has the lock, no worries, they will unlock it eventually
[19:42:41] <doug16k> if you have the lock and they want it, you will release it eventually
[19:43:27] <doug16k> if you acquire the lock then the IRQ comes in, then the IRQs will be masked, so you can continue with your locked thing, then unlock and unmask IRQ, you will immediately run the handler after reenabling irqs
[19:44:03] <doug16k> the handler will run and it will acquire the lock because it is guaranteed that it can't run the ISR if someone is holding that lock (because you mask IRQs when held)
[19:44:33] <doug16k> if the other cpu is holding it, doesn't count, they continue - you didn't preempt other cpu
[19:44:55] <doug16k> you just spin on the lock until they unlock it. no race or deadlock
[19:46:51] <doug16k> maybe an analogous case would be easier to picture: in my AHCI driver, I have to acquire the port lock to modify the command slots on a port. I have to do that for every I/O syscalls or whatever
[19:47:28] <doug16k> big problem: the controller might finish an I/O at any moment! what is the IRQ handler going to do if it interrupted code that was modifying a port's command slots?
[19:48:01] <doug16k> solution: when you are acquiring a lock that must be acquired in an IRQ handler, you must not allow IRQs when you are holding the lock outside the IRQ handler
[19:48:43] <doug16k> the IRQ will wait until you unmask IRQs. it won't be missed
[19:49:06] <doug16k> assuming sane wait time that is
```

Need to sleep on this -- I gotta rewrite my spinlock code (and everything that uses it!).

---

OK, I think as I rewrite this code I am going to need to also rename the fuctions to make sure I catch every use.  It's always the safest thing to do.  First, what functions do I need?

* `SpinLock()`
* `SpinUnlock()`
* `SPINLOCK_BLOCK()`
* `SPINLOCK_RLS()`
* `SPINLOCK_BLOCK_NO_INT()`
* `SPINLOCK_RLS_RESTORE_INT()`

The last 4 are macros that will call the first 2 and will manage interrupts.  I really should not have any direct calls to `SpinLock()` or `SpinUnlock()`.  Checking quickly, the new names are unique, so these should work.

Finally, I will also rename `SpinlockIsLocked()` to `SpinTest()` just to be on the safe side.

Did I mention this will touch most source files?

---

Well, not as many as I thought....

---

### 2019-Jun-13

Today I want to try to get the second core to be able to write to the serial port.

---

I have the serial port protected now and I am outputting characters to the port -- at least for a short time.  And then I have some deadlock that is happening.  The deadlock happens is rather short order.  But I am not able to determine where with the rpi since I have no abliity to debug the code.

I think my spinlock implementation (using `AtomicInt_t`) is flawed.  Comparing that to the `lk` implementation, I am not doing things right.

I am going to re-implement spinlocks with architecture-specific functions and take the `AtomicInt_t` implementation out of the mix.  And this solved my problem.

So, now, the next step is to make sure that Core1 is properly initialized.

---

I believe the only thing left to do is to set the timer up on Core1....  Other than that, the Core appears to be ready to rock.

Now, the PIC would have been programmed, so I am not able to just call `TimerInit()`.  I will need a purpose-built function to enable the timer on the next core.  And really, for the ARM SoC, it will be just unmasking the IRQ.  I take that back -- the per core PIC initialization needs to be completed.

---

I was worried about what would be needed between the cores, but it looks like I can really init the timer multiple times:

```
Starting core with message to 0xf900009c
.*CPU 0x1, CNTP_TVAL = 0xa2b22d18
Timer reload Value = 0x000003e8
```

Since t he reload values are different, I should be able to clean up the initialization code and be safe.

---

### 2019-Jun-16

At this point, I have the PIC and Timer initialization worked out by core (Core0 does a few more things).  However, when I try to enable interrupts on Core1, I get an undefined instruction fault.  Core0 keeps chugging along....

So there are a couple of things here I need to look at:
1. If the timer callback code reentrant (I cannot imagine why it would not be based on globals and such, but I need to look at everything it does)
1. I also need to stop the other cores when I get a fault so that I can read the actual data on the fault.

So, I need to take the `cpus` variable and move its allocation into `PlatformInit()`.  The structure for that will need to be driven by the number of cores discovered, and the `cpuCount` member needs to be initialized anyway.

I have a `microsSinceBoot` variable in rpi2b which is a global and being updated by all cores.  Now, here's the problem: this is also driving my sleep code so it needs to be a global variable, but it also needs to be only updated by one core.

---

Here was my conversation on `freenode#osdev`:

```
[00:23:07] <clever> 5.6.3 is already in nixpkgs
[09:19:21] <eryjus> Please check my thinking on this: I have 2 globals (tick_count and next_wake_ticks) which control the logic for my sleep functions.  I expect a process might sleep on core0 and the be woken up on core1 -- I am not locking a process to a core.  Currently all cores' timer irqs are updating tick_count and all cores are checking for processes to wake.  The first is an obvious problem; but the latter I believe also needs to be pinned to
[09:19:21] <eryjus> core0 -- my believe is that 1) more than 1 check is redundant, 2) if core0 is updating the tick_count, then it is in the best position to check if something needs to wake up.  Am I in the ballpark?
[09:20:30] <bcos_> eryjus: Yes; but this doesn't scale well
[09:21:05] <eryjus> bcos_: by its very nature or by my design?
[09:22:22] <bcos_> By nature - e.g. "single global queue with a single global timer" vs. "one queue per CPU with a timer for each CPU"
[09:23:21] <eryjus> this, then would imply "if a process sleeps on core0 it must be woken by core0" -- i assume that multiple timers are, by definition, out of sync
[09:23:26] <eryjus> multiple masters problem
[09:24:37] <bcos_> For multiple timers; you can keep them "sychronised enough". Note that "sleep()" doesn't guarantee that you'll be woken at exactly the right time (and will always be "never sooner than and often later than the exact time")
[09:24:53] <eryjus> hmmm...  but that does not imply that it has to execute again on core0...
[09:25:17] * bcos_ nods - woken by core0 (and scheduled to run on some other core after it got woked)
[09:27:45] <bcos_> Of course "one queue per CPU" can also have disadvantages, mostly for power management (e.g. where it'd be good to be able to send the CPU's queue elsewere so that the CPU's timer can be shut down and doesn't take the CPU out of a power saving state)
[09:28:25] <bcos_> ..so you could end up with a hybrid thing - e.g. "global queue and timer + queue and timer per CPU"
[09:29:59] * eryjus nods, and paces and codgigates
[09:30:35] <bcos_> Hrm
[09:31:09] * bcos_ would recommend looking into how "high precision timers" is implemented in Linux (e.g. their "timer wheel" stuff, ticklessness, etc).
[09:31:32] <bcos_> (it's actually the only piece of Linux that I'd say is good)
[09:32:57] <zid> I read that as ticklishness
[09:33:05] <zid> wonder how you measure how ticklelish a thread is
[09:33:21] <doug16k> eryjus, if a thread sleeps then you note when it has to wake up and put it to sleep. then the scheduler won't consider it until that time elapses, then it makes it ready. then it runs, who cares which cpu
[09:34:06] <doug16k> sleep can sleep in
[09:34:30] <doug16k> if you are unable to set it up so the timer fires at the perfect time every time that is
[09:35:11] <doug16k> eryjus, you have a state on threads right? they can transition to sleeping and become non-runnable?
[09:38:45] <doug16k> if you take that thread, remove it from its ready list and insert it into a sleep queue, then you can keep the sleep queue ordered too and only check "now" against the thread that is waking up soonest
[09:39:32] <doug16k> then each tick you shovel all the sleep queue items back into the ready queue until soonest one is in the future
[09:39:50] <doug16k> side bonus: good for power use to coalesce work like that
[09:40:26] <doug16k> hopefully programs don't call sleep unless they are totally just stalling in a unit test or something. a realistic "sleep" is a timeout wait
[09:41:10] <doug16k> i.e. "sleep" until timeout expires, and probably be awoken by a condition variable wake up or something, sooner
[09:44:50] <doug16k> a decent program will use a proper "poll" mechanism (which doesn't really poll, silly term), so realistically it won't just force wait N milliseconds, but it will frequently say "block me until this time elapses or one of these things I want to watch gets notified"
[09:45:16] <doug16k> which looks a lot like a sleep when nothing happens to the notification things
[09:46:33] <doug16k> I'd focus on the "wait but have a timeout" scenario way more than sleep(int) scenario. former is way more common and realistic, latter is hopefully rarely used
[09:47:59] <doug16k> once you have "wait with timeout" then you just wait for nothing and you have sleep(timeout)
[09:48:14] <doug16k> it falls out of the implementation of wait with timeout
```

While I do not want to completely discount that doug16k is saying, I believe that is something to be saved for a later enhancement.  He is right in that I would be able to save code by making a process wait on nothing with a timeout set.

Now, with the thought of setting a one-shot timer for wakeups....  I do like that idea but would have to revisit the code for that.  However, I am not exactly sure how that would work on non-x86 architectures.

So, this, then, leaves the concept of powering off a core when not in use.  For this, I believe that the butler or management process would be able to identify the opportunities to power off a core and send an IPI to that core to have it go into a more idle state and set itself for low power mode.  For this to work, a core would need to have the ability to move it's sleeping queue to another core while syncing up the tick-to-wake value.

So, here is what I see are the hallmarks on this:
1. Core0 is never placed into low power mode or powered off.
1. Core0 is responsible for maintaining "ticks since boot"; the other cores just read from that value.
1. Sleeping processes are not yet ordered and have been added as something to do:  http://eryjus.ddns.net:3000/issues/412.
1. I will have a sleeping queue for each core, making it easier to queue a sleeping task.  Each core will be responsible for waking a task which was put to sleep on its core.  However, this same awoken process does not need to execute on the same core -- let the scheduler work that out.
1. When I get into power management, I will need to be able to move sleeping processes from one core to another.

Now, back to my original problem, where I have all cores' timers updating the ticks.  Simple enough to fix.

OK, so here is the UNDEF fault:

```
.......U@n.d.e.f.i.n.e@d. .I.n.s.t.r@u.c.t.i.o.n.:@
.A.t. .a.d@d.r.e.s.s.:. @0.x.f.f.8.0.1@f.2.c.
. .R@0.:. .0.x.0.0@0.0.7.e.2.8. @ . .R.1.:. .0@x.0.0.0.0.0.0@4.1. . . .R.2@:. .0.x.8.0.0@a.5.1.7.c.
. .R.3.:. .0.x@8.0.0.0.5.6.1@8. . . .R.4.:@ .0.x.8.0.0.a@5.3.2.0. . . @R.5.:. .0.x.8@0.0.a.5.3.2.0@
. .R.6.:. @0.x.8.0.0.a.5@3.4.c. . . .R@7.:. .0.x.f.f@f.f.e.a.e.f. @ . .R.8.:. .0@x.f.f.f.f.e.f@e.f.
. .R.9@:. .0.x.f.b.b@d.b.f.7.f. . @R.1.0.:. .0.x@1.5.d.8.b.4.c@c. . .R.1.1.:@ .0.x.8.2.5.3@7.f.f.d.
.R@1.2.:. .0.x.0@0.0.0.0.0.0.0@ . . .S.P.:. @0.x.8.0.0.0.1@7.b.0. . . .L@R._.r.e.t.:. @0.x.8.0.0.0.5@6.2.c.
.S.P@S.R._.r.e.t.:@ .0.x.a.0.0.0@0.1.d.3. . . @ . .t.y.p.e.:@ .0.x.1.b.
.
.A.d.d.i.t@i.o.n.a.l. .D@a.t.a. .P.o.i@n.t.s.:.
.U@s.e.r. .L.R.:@ .0.x.c.f.9.8@f.d.8.d. . .U@s.e.r. .S.P.:@ .0.x.3.f.5.d@e.f.b.d.
.S@v.c. .L.R.:. @0.x.f.f.8.0.1@f.3.c.
```

This is a pain to read with all the difference characters interlaced, so to clean this up:

```
Undefined Instruction:
At address: 0xff801f2c
 R0: 0x00007e28   R1: 0x00000041   R2: 0x800a517c
 R3: 0x80005618   R4: 0x800a5320   R5: 0x800a5320
 R6: 0x800a534c   R7: 0xffffeaef   R8: 0xffffefef
 R9: 0xfbbdbf7f  R10: 0x15d8b4cc  R11: 0x82537ffd
R12: 0x00000000   SP: 0x800017b0   LR_ret: 0x8000562c
SPSR_ret: 0xa00001d3     type: 0x1b

Additional Data Points:
User LR: 0xcf98fd8d  User SP: 0x3f5defbd
Svc LR: 0xff801f3c
```

OK, I'm convinved this is related to enabling the FPU on the core.  The `LR_ret` value is from `TimerCallback()`.

```
CPACR == 0x00000000
NSACR == 0x00060c00
FPEXC == 0x00000000
```

I had to copy some code from `EarlyInit()`.  Now, the best thing to do here would be to move this code into a function....

---

Well, crap!!  I switched over to the x86-pc and it no longer boots!  This is going to be fun to debug.

Ahhhh...  it's a spinlock problem!

---

### 2019-Jun-17

Now to the trampoline code.  There are a few requirements I need to satisfy for this code:
* It must start in low (<1MB) memory, but the code will be loaded to >1MB and will need to be relocated
* The code must be completely position independent
* The CPU starts in real mode
* The CPU starts in 16-bit mode
* Protection needs to be enabled (meaning a GDT)
* I need to set an IDT
* I need to set a TSS
* I need to enable paging (with a proper cr3)

I am also realizing I need a semaphore to control the boot process since there is no way to control the stack addresses.

So that I have this link, I am using dgos as a roadmap...  https://github.com/doug65536/dgos/blob/c37f6d93584fb509cdc5e2ae04cf13d111bee00a/boot/mpentry_bios.S#L5

---

### 2019-Sep-19

Well, I walked away for a while....  I went camping and forgot my charger.  When I got back I was energized to get a bunch of work done around the house.

I am getting ready to go camping again, so I will have some time to spend on this again.

But first, I need to figure out where I left off....  The image runs fine.

---

I can see at this point that interrupts stop firing.  I am only getting the idle process loop outputting `'.'` characters.

When I comment out `ProcessMilliSleep()`, I start getting the interrupts to fire.  So, I still have a scheduling problem!!

---

I'm pretty sure I have something wrong with this implementation: https://wiki.osdev.org/Brendan%27s_Multi-tasking_Tutorial#Step_8:_Locking_Version_2

---

### 2019-Nov-25

I have a mess on my hands.  I have scheduler problems for sure.  Plus I am no longer happy with my code base.  A serious clean up is needed here.  I just need to make a decision on what I want to do.  It's between starting over or pressing forward with this code (or starting a new base and start copying things in).  As it is, I already deleted a branch to fix the scheduler.

---

I have a lot of work already invested in this kernel.  I am not ready to cut all those losses yet.  So, here is what I am going to do: I am going to work on cleaning up the scheduler first.  I really need that to be rock solid before I move on to other things.  I am putting SMP on hold until I get that wrapped up.  I have a working scheduler from the [Scheduler Test](https://github.com/eryjus/scheduler) I was working on.  I should be able to port that in relatively easily -- just taking into account the differences in design.

So, I need to understand what is working and what is not.  Right now the timer is firing and I am getting nothing else running.

The one key takeaway I have from the [Scheduler Test](https://github.com/eryjus/scheduler) is [documented here](https://github.com/eryjus/scheduler/blob/master/JOURNAL.md#2019-nov-24): the process startup function needs to be careful about what released it executes.

I was also looking at [dgos](https://github.com/doug65536/dgos/blob/86f017083f34fa7aef22811e39bbd207929d116b/kernel/lib/assert.h#L9) for inspiration.  I need to set up some asserts so that I can output better debug information -- relevant stuff, not just a bunch or stuff to look at.

I also see that [dgos has several variables at the per-CPU level](https://github.com/doug65536/dgos/blob/6a9de3e23d85bd7582a971b56671b1a37fd78312/kernel/arch/x86_64/cpu/asm_constants.h#L5), all as [offsets from the the `gs` segment selector](https://github.com/doug65536/dgos/blob/49fbb798e2dbb445f610447e4101334911123b59/kernel/arch/x86_64/cpu/segrw.h#L4).

It's been a while, so I also need to go through [Redmine](http://eryjus.ddns.net:3000/projects/century-os/issues).  I will be cleaning up some issues and adding several TODOs to work on as I identify what needs to happen.

The one thing I need to take away from today's agony is that I really want to work on a clean-up effort and not a rewrite effort.

## Version 0.4.6a

OK, so in this version, I am going to work on getting the scheduler code ported from [Scheduler Test](https://github.com/eryjus/scheduler) into this code.  When I do this, I should be able to get processes to run again.

### 2019-Nov-26

Today I worked on redoing all the scheduler/process code to more closely align with the tests I wrote.  It is not perfect yet and I have some debugging to do.  Something, I believe, is enabling interrupts when it shouldn't.  I am right now working on stripping a log to try to track that down.

---

Turns out all my thoughts were correct...  I was enabling interrupts in `ProcessUnlockScheduler()` rather than releasing the spinlock.  The code is working now -- well, more generally.

---

### 2019-Nov-27

I have been thinking about this.  The only real feature I have in the kernel that is (supposedly) fully vetted is the scheduler.  The rest is semaphores (which cannot be tested without getting the scheduler working), mutexes (which cannot be tested without getting the scheduler working), and initialization.  It may be a bit of a mess structure-wise, but that is working.  Yeah, I have some memory management stuff, but that is all working.

So, I really should be able to drop all the fuss here and work on the scheduler.  This is going to mean working similar tests to what I did with the [Scheduler Test](https://github.com/eryjus/scheduler).  However, all-in-all, this is not as bad as I was worried about: ignore the other stuff and get the scheduler working.

So, with that out of the way, I now have this problem:

```
Enabling interrupts now
Request to overwrite frame for page 0xff801000 refused; use MmuUnmapPage() first
```

I believe that this is happening when I create a new process (a problem I did not have before I tried to add a new process).

This I was able to track down to having SMP enabled when I was not ready for it yet.  So, for the moment I turned that back off at the `qemu` command line.

This now, lets me properly focus on creating a process.

---

So this works properly when there is no actual spinlock.  We can pretend to lock multiple times with no real impact.  However, in this case `ProcessCreate()` which locks the scheduler and then calls `ProcessReady()` to lock the scheduler again.  But this process already holds the lock and the lock is not available.  And, therefore, I have a problem.

There may be several possible solutions here:
* Create locking and non-locking versions of the key functions and call the proper one based on the locks already held
* Update my spinlock code to capture a depth and lock holder and then update the locks to populate the structures properly
* More I have not considered

One thing that I do want to be able to do is have a process that gets CPU that is waiting for a lock to be released is to be able to donate its unused quantum to the lock holder.  This would be so that in theory the lock holder would be able to complete what it needs to do and release the lock faster (in particular when the lock holder is at a lower priority than that of the lock desirer).  In particular when SMP is involved (since a single CPU should never have that state).

So, it looks like I will be updating my spinlock code to capture the current `Process_t` and count the lock depth.  This may bite me when I get to multi-threading....   Hmmmm... what does Linux do?  It only keeps track of owner and CPU when the debug flags are set.

... debating....

Actually, I have come to the conclusion that having a "depth" on a spinlock is actually not proper and I need to go the other way: create locking and non-locking version of the critical functions and make sure I am calling the correct one.  Each of the non-locking functions will be in the form `ProcessDoXxx()` while the locking form will be in the form `ProcessXxx()`.  This should make it easier to keep them straight.  Additionally, the `ProcessXxx()` form should be able to be implemented as a macro, lightening the needs of the stack and function calls.  If real functions are needed, I will keep them in the same source file, a departure from the 1-function-per-file sceme I have been working on so far.  However, this should be easier to keep straight this way so I am not having a problem with it.

---

With that done, my processes are never seeing the light of the CPU.  On top of that, the kernel is only running for a short period of time.

So, in `ProcessSchedule()`, with this block of code:

```C++
    if (AtomicRead(&scheduler.schedulerLockCount) != 0) {
        scheduler.processChangePending = true;
        kprintf("x");
        return;
    }

    kprintf("+");
```

... I am never seeing a `'+'` character emitted:

```
Enabling interrupts now
@@@@@@@the new process stack is located at 0@@@@@@@@@@@@@@@xf@f801000 (frame 0x00000401)
@@@@@@@@xx@xx@xx@xx@xx@xx@xx@xx@xx@xx@xx@xx@xx@xx@xx@xx@xx@xx@xx@xx@xx@xx@xx@xx@xx@xx@xx@xx@xx@xx@xx@xx@xx@xx@xxthe@xx@xx@xx@xx@xx@xx@xx@xx@xx@xx@xx new process @xx@xx@xx@xx@xx@xx@xx@xx@xx@xx@xx@xx@xx@xx@xx@xx@xx@xxstack is located@xx@xx@xx@xx@xx@xx@xx@xx@xx@xx@xx@xx@xx@xx@xx@xx at 0xff80@xx2000 (frame 0x00000402)
x@xx@xx@xx@xx@xx@xx@xx@xx@xx@xx@xx@xx@xx@xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx@xx@xx@xx@xx@xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx
```

This means that my `schedulerLockCount` is never making it to 0, or I have a problem with my `Atomic` implementation.

I was looking at `schedulerLockCount` rather than `postponeCount`...  grr....

Now, with that change, I am still not getting a process change.

---

### 2019-Nov-28

Last night as I was headed to bed, I realized that `ProcessSwitch()` was not coded correctly.  So I fixed that up this morning and I have the tasks at least cooperatively scheduling.  This is much better!

Now, sleeping appears to still be a problem.

This I found interesting:

```C++
        do {
            // -- -- temporarily unlock the scheduler and enable interrupts for the timer to fire
            ProcessUnlockScheduler();
            EnableInterrupts();
            HaltCpu();
            DisableInterrupts();
            ProcessLockScheduler(false);     // make sure that this does not overwrite the process's flags
            next = ProcessNext();
        } while (next == NULL);
```

This is the code for when no process is available to run.  The CPU will eventually be able to go into a deeper and deeper sleep mode.  However, I needed to properly unlock the scheduler to allow the timer to fire and get that same lock.  At the same time I had to force-enable interrupts and then force-disable interrupts.

What is more important here is that because I am forcefully changing the state of interrupts here, I also needed to take care that I did not actually change the state of the flags stored in the scheduler structure.  I guess technically I could have left out the calls to `EnableInterrupts()` and `DisableInterrupts()`, but I like the extra readability that this gives.  If I before concerned with performance later, I can tune this for sure.

I have now made some changes that need to be brought over to the RPi.

---

OK, for RPi, I had to work on cleaning up the architecture-specific `ProcessSwitch()` function.  That was a real mess and I'm not sure just how that worked to be honest.  That said, I have processes switching now, but the kernel locks after about a second..., and the sleep functions are not working properly -- I am just getting round-robin scheduling with no delays between letters being output:

> .AB.AB.AB.AB.AB.AB.AB.

I walked through `ProcessSwitch()` again and everything looks good to me.  So, I need to consider other things.  For example, am I getting the correct timer value and do I have another recursive lock to deal with?  I'm going to start with the timer....

* `TimerCallBack()` uses `TimerCurrentCount()` to determine the current timer value.
* `TimerCurrentCount()` uses the device-specific `dev->TimerCurrentCount()` function.
* For the bcm2836, this function is `_TimerCurrentCount()`.
* The `_TimerCurrentCount()` function is a common function across all platforms, and reads `microsSinceBoot`.
* `microsSinceBoot` is updated only from `_TimerPlatformTick()` function.
* This function is called from the device-specific `dev->TimerPlatformTick()` function.

So, in short, this should be working.

---

I did track this down to the code in `TimerCallBack()` that searches the list of sleeping tasks to see what is available to run next.  It gets caught in an infinite loop.  And it looks like the actual list head has been removed from the list -- the last process somehow points to the first one rather than the list head structure.

I'm starting to wonder if the wrong process is being added to the ready queue on a process change....  It would explain processes getting CPU when they are waiting and the ready queue being out of sync.

And when I comment out the call to `ProcessDoReady()` from `ProcessSwitch()`, I get the same behavior.  So, I am on the right track I think, but I'm looking in the wrong place.

So, I need to go back and redo my tests.  In particular, I need to look at `ProcessBlock()`.

This works for the x86-pc target but does not for the rpi2b target. Hmmm....

OK, I have been able to work this down to the fact that `ProcessSwitch()` is not calling `ProcessDoReady()` when it should.

And with those changes, at least the rpi2b and x86-pc targets are consistent -- and they both appear to be working properly.

---

OK, finally, I need to get this to work on real x86 hardware.  Currently, it will boot and I am getting some garbage to the serial port, i.e. lots of stuff like this:

> ����������p��������

and then a triple fault.

Let me commit this point and I will work on the baud mismatch.

---

OK, a quick check reveals I am setting the serial port to 38400 baud:

```C++
    outb(base + SERIAL_INTERRUPT_ENABLE, 0x00); // Disable all interrupts
    outb(base + SERIAL_LINE_CONTROL, 0x80);     // Enable DLAB (set baud rate divisor)
    outb(base + SERIAL_DIVISOR_LSB, 0x03);      // Set divisor to 3 (lo byte) 38400 baud
    outb(base + SERIAL_DIVISOR_MSB, 0x00);      //                  (hi byte)
    outb(base + SERIAL_LINE_CONTROL, 0x03);     // 8 bits, no parity, one stop bit
    outb(base + SERIAL_FIFO_CONTROL, 0xC7);     // Enable FIFO, clear them, with 14-byte threshold
    outb(base + SERIAL_MODEM_CONTROL, 0x0B);    // IRQs enabled, RTS/DSR set
```

This is what I need to clean up.  It should be easy enough to fix by changing the divisor to 1 (38400 * 3 == 115200).  So, I need to figure out if this is just as simple as changing the `SERIAL_DIVISOR_LSB` to 1.  A quick test says yes, that is it.  Plus I confirmed that this had no impact on the QEMU emulator.

---

So, now for the triple fault....  I believe that this is related to parsing the ACPI tables and one of the table addresses is not mapped in the MMU.

OK, so I have narrowed it down to a difference between the RSDT (which is available from the emulator) and the XSDT (which is what is presented on real hardware).  This boiled down to a difference between `uint32_t` and `uint64_t`....  Damned copy-and-paste!

So, now, I am getting to the point where I jump to the kernel.  And then I triple fault again.  This is probably also going to come down the the MMU -- but probably the frame buffer.

A quick test confirms that I am actually making it to the kernel.  However, the first thing that is done is a `kprintf()` call and that is failing.  Now, that may be the stack, to let's see if I can test that.  I can do that with a call to `DisableInterrupts()`:

```C++
void kInit(void)
{
DisableInterrupts();
while (true) {}
```

And this test failed.  This means I have a stack problem.

This now makes me wonder if the real MMU hardware will check the memory location before pushing values.

However, since the first thing that is done is to create 12 bytes of room, I doubt that is the case:

```
80005db0 <kInit>:
80005db0:       83 ec 0c                sub    $0xc,%esp
80005db3:       e8 98 a3 ff ff          call   80000150 <DisableInterrupts>
80005db8:       eb fe                   jmp    80005db8 <kInit+0x8>
```

So, I now will check for the mapping before I jump.

On real hardware, I have the following results:

```
MmuDumpTables: Walking the page tables for address 0xff800000
Level  Tabl-Addr   Index       Next Frame  us  rw  pr
-----  ----------  ----------  ----------  --  --  --
PD     0xfffff000  0x000003fe  0x00000000   0   0   0
```

... and on QEMU I have:

```
MmuDumpTables: Walking the page tables for address 0xff800000
Level  Tabl-Addr   Index       Next Frame  us  rw  pr
-----  ----------  ----------  ----------  --  --  --
PD     0xfffff000  0x000003fe  0x000003f7   1   1   1
PT     0xffffe000  0x00000000  0x000003f8   1   1   1
```

So, what the heck is this?

---

### 2019-Nov-29

So, my triple fault problem boiled down to the CPU count ending up being 0, and therefore a stack for that CPU was never created.  With no stack, no stack operations could be done or it would end in a triple fualt.

Now, my IBM T61 laptop boots all the way to the point where it is waiting for a Timer interrupt, which never comes.

smh....


```C++
__ldrtext PicDevice_t *PicPick(void)
{
    if (GetIoapicCount > 0) picControl = &ioapicDriver;
    else picControl = &pic8259;                         // -- fall back in the 8259 PIC
```

Missing the parenthesis for a functtion call to `GetIoapicCount()`.

And with that change, I am now officially running on real x86 hardware with the same results as the emulator!

## Version 0.4.6b

So, here I continue my cleanup.  And the major purpose of this version is to get a working `assert` macro -- and something I can disable when I am preparing release code.  This should also allow me to start removing much of my `kprintf()` debugging code.

Ha!!  This is going to drive me to implement `'%d'` in my `kprintf` implementaiton.

---

OK, that done, I am going to go back to retrofit the Scheduler code to use `assert`s.

Now, with that, the scheduler has a rather big dependency on Spinlock code.  So, I feel like a minute or 2 to clean up spinlocks will be good as well.

Also, since the heap code is good (I believe), I am going to set up some debugging flags so that I can turn on/off some of the checks as needed.

I think I need to commit this code.

---

## Version 0.4.6c

Before I can get to SMP properly, I need to be able to get the ACPI structure read on my hardware.  The problem here is that the ACPI tables are split across multiple MMU pages.  So, I will need to temporarily identity map and later unmap these pages.

That was relatively easy to do just by checking if the memory was mapped and mapping it if not.  I also took some time to clean up some of the loader stuff (not all of it).

So..., what else should I debug while I am at it?

I know it has been some time, but I have this feeling that I went through all these v0.4.x versions without really understanding what I was doing and really how to debug it all.  I probably need to go back to the beginning and redo some of this work and related testing.  Some I will not be able to do until I have SMP up and running properly.  But, some I really need to get done now.

So, I think the first thing to do is to make sure I am really using the IOAPIC.  Which is true.

This, then, leaved the Posix IPC synchronization primitives to debug.  Before I get into that, I think I want to commit this code.

---

## Version 0.4.6d

OK, now time to put some thought into debugging the IPC primitives.  Maybe this is where the rewrite needs to come in, I don't know.  But I do know I will need to spend some more time reading.

Right out of the chute, I realize that I started coding to XSI conformance -- which is an extension to the regular Posix conformance level.  So, I made a mistake.  In the Posix specification, I really need to focus on [`semaphore.h`](https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/semaphore.h.html) as the starting point for my work.

---

### 2019-Nov-30

OK, so maybe I was wrong yesterday.  It looks like `sem_open()` and the like are C runtime library routines whereas `semget()` is a system interface.  See `man 2 syscalls`.  This is what I am interested in.

---

I'm starting to wonder if I am not getting the cart before the horse here....  Do I really need APIs for all this user-space stuff before I actually have a user-space and before I have a C Runtime library?  In other words, shouldn't a spinlock be sufficient at the kernel level and leave all this other stuff to runtime?

Now, I will need some form of messaging between kernel processes.  But that is not yet the point either.  The point is to get all the processors running.  For that, I will need to go back to the [memory map](http://eryjus.ddns.net:3000/projects/century-os/wiki/High-Level_Virtual_Memory_Map) (which remains a living document) because I had problems with more than 1 processor stepping on stacks.

However, in just updating this map, I realize that I need a mechanism for allocating/releasing kernel stacks.  I have alocated space for 1024 kernel-related stacks (each 4K in size).  I only need a flag to indicate free or used (`bool`), so a bitmap should work.  The problem here is that I need to be able to do some of this in the loader and maintain it in the kernel as well.

I am also going to need to get my PMM up and running as a task as well.  This will end up in user-space (which is not yet working properly) and I will need message-passing capability.  I have a small PMM for initialization which will eventually step on the kernel data and I need to keep an eye on that -- a very close eye indeed!  But no point in taking that on yet.

Finally, I believe I have initialization all wrong.  I believe I want to actually start a process from the kernel to be able to map the devices and trigger driver loads and then let that process end.  But, once again, there is no need to take that on now either.

So, the stack allocator it is.  Up to 1024 stacks.  This will need 128 bytes to hold the bitmap.  Or, 32 X 32-bit dwords.  This really should be small enough to statically allocate.

Well, no....  I really do not need to go there yet either.  I am trying to set up a stack for every CPU as part of the Boot Processor initialization.  This is not correct.  I merely need to allocate one for the BP and move on.  So this will be cleaned up now.

Also, `cpus.cpuCount` is messed up and not handled properly.

---

Thinking about this a bit, I beleive the best effort here is going to be to keep track of the discovered cores and the running cores.  For the rpi2b target, it will be simple (at some point I need to pull in the dtb).  For the x86-pc target, I will need the MADT for the core count.  The easiest way to get there is to change the `cpus` structure member names and fix what breaks.

Where's my Easy Button?  Now, go back to starting the additional cores.

I thought I had the rpi2b core 1 starting:

```C++
__CENTURY_FUNC__ void CoresStart(void)
{
    // -- start core 1
    kprintf("Starting core with message to %p\n", IPI_MAILBOX_BASE + 0x0c + (0x10 * 1));
    MmioWrite(IPI_MAILBOX_BASE + 0x0c + (0x10 * 1), (uint32_t)entryAp);
    SEV();
}
```

It technically does, but there are issues:

```
Starting core with message to 0xf900009c
sDhaet an eEwx cperpotcieosns:
m.a.c kD aitsa  lFoacualtte dA dadtr e0sxsf:f 800x1f0f0800 1(fffr8a
 . .0 xD0a0t0a0 0F4a0u0l)t
ctthaet unse wR epgrioscteesrs:  s0txa0c0k0 0i0s8 0l7o
a.t.e dF aautl t0 xsftfa8t0u2s0 000x 7(:f rTarmaen s0lxa0t0i0o0n0 4f0a1u)�5�) .(�EE�����х�с����ᕍ�����ѽ����5�)5�)����"��х�с���Յ��ѱ������ɑ��͕���́������ᅙ����������j�RjrRrrr"��х�с���Յ��ѱ��M��M��х���Ձ�I��I����ͥ�͕�ɕ�Ɂ������������������ª�j�RjrRrrr2��Յ��ѱ��́�ͅ�х���Ձ��������Ł�Q������͝����ѕ���ѹ������ձ��с�������ɱ��с������ٙ���ɥ�5�)���������������5�)�������Ʌ�Օ��с������������ɕ����������
```

So, I better start off cleaning this up.  The x86-pc target does not start crap.

So, then assuming that ever other letter belongs to one core, I can pull this apart like this:

```
she new process
.. Data Fault Address: 0xff801ff8
.. Data Fault Status Register: 0x00000807
.. Fault status 0x7: Translation fau
```

If I am seeing this properly, then the stack for the second CPU is not mapped....  tomorrow.

---

### 2019-Dec-01

This morning I realized that I am passing the entire Loader portion of the boot code.  What was my thinking here??

> ### 2019-Jun-08
>
> [...]
>
> So, for the APs, we are going to go off to another entry file (`entryAp.s`).  In this file each AP will wait until there is "permission" to continue, at which point, we will get a stack and start running some code.
>
> The problem with the stack is that we cannot reuse the stack from CPU0.  For CPU0, we are starting with the stack at `0x8000`.  This persists until we jump to the kernel, `kinit()`, when the stack is replaced at that point.  So, what I really need in this case is a method to allocate stacks.  It should be as easy as taking the cpu number and setting an offset, but the AP does not have enough configuration to be able to map a stack into the MMU.  CPU0 will have to be responsible for this task during `MmuInit()`.

Ahhh...  This was my thinking.  It makes sense now.

I was going to have CPU0 perform all the mappings needed for the other CPUs for stacks and have them already ready and waiting, so all any AP was required to do was calculate its stack and set the register.

I removed this code earlier.

> ### 2019-Nov-30
>
> [...]
>
> So, the stack allocator it is.  Up to 1024 stacks.  This will need 128 bytes to hold the bitmap.  Or, 32 X 32-bit dwords.  This really should be small enough to statically allocate.
>
> Well, no....  I really do not need to go there yet either.  I am trying to set up a stack for every CPU as part of the Boot Processor initialization.  This is not correct.  I merely need to allocate one for the BP and move on.  So this will be cleaned up now.

So, I really do need a stack allocator for the kernel stacks.  I would be OK if I had not consumed these stacks with running processes, but I did and it properly exposed a problem.  I did have some decent bitmap code I can resurrect for this purpose.

---

After creating the stack manager, I also updated the 2 `ProcessNewStack()` functions to take advantage of this.  When I try to test it, the system goes no further.  Ahhh..  I'm also calling `StackAlloc()` with the lock held.

---

I thought I had this all cleaned up and then figured out that the rpi2b target locks on boot.  There is something going awry with `PlatformInit()` or thereafter.  This means I have something still very fragile.

---

### 2019-Dec-02

Well, I got that all sorted out.  My bitmap calculations were off.  Once I got that all worked out, things worked just fine, and I have 2 cores started on the rpi2b target.

I still have some significant issues to work through for the x86-pc target.

But, I was also thinking about how to get the second core to get into the scheduler.  There is really no reason to keep a process going at this point, so it is not like the boot processor that assumes the butler role.  On the other hand, I have no `Process_t` structure I can use to call `ProcessSchedule()` directly.  However, that is not to say that I cannot allocate a `Process_t` structure for the core and then call `ProcessTerminate()` on itself.

Before I go there, I really want to get another core started on an x86-pc.  For that, I need to read -- I'm all a mess there.

[This code](https://wiki.osdev.org/APIC#SIPI_Sequence) has a sample startup sequence.  And the startup sequence is great, but how to I set the startup location?

---

### 2019-Dec-03

I think I knew this before, but I just realized it again last night: *these cores all start up in real mode!*  This means I have to do all the work of getting them into protected mode.  Now, that really should not be a problem, but the whole thought of starting with 16-bit code threw me a bit last night.

...  and if you have a look at `entryAp.s` for x86, I stopped working on that while it was being worked on.  It really looks like a mess.

---

Well, I have that written.  My only problem is that I am stuck on the temporary stack.  But is that OK?  I think it is because I am not going to get very deep into anything before it self-terminates.

So, now I need to see if I can spin up a core and see if I can totally crash this thing.  Tomorrow.

---

### 2019-Dec-04

Thinking about this, I am going to have to be very careful about setting up a race condition between getting the temporary process stopped which will free up the stack and performing the post-loader cleanup which will dump the pages from the MMU tables.  The TODO is captured [here](http://eryjus.ddns.net:3000/issues/421).

Now, with that said, let me see if I can't get a CPU spun up.  I have that compelling feeling like I should start over again to clean up the code.  The problem I am having is that I feel like I should be able to spin the CPUs up as part of the loader while I am parsing the ACPI tables -- have them start and wait in a holding pen until a lock is release or a semaphore is increased.

OK, well, I press on....

---

Well I am able to get the next core to start.  It almost immediately started giving me `#UD` faults.

I got that figured out by setting the vector properly (which is `000vv000` where `vv` is the vector number!).  And, now I am having a location problem when the `kernel.elf` is linked.

---

### 2019-Dec-05

Well, I spend most of my time today debugging the trampoline code to get into protected mode again.  I finally have achieved protected mode (I had to use an `iretd` rather than a `jmp long` to get there -- the encoding was just not going to work).  Now, with that said, I am not certain I will be able to have this run on real hardware yet.  Though I am hopeful.

What I did not realize (or perhaps remember), what that I need a TSS for each CPU.  My current problem is this:

```
00171667489d[CPU1  ] protected mode activated
00171667499d[CPU1  ] inhibit interrupts mask = 3
00171667504e[CPU1  ] LTR: doesn't point to an available TSS descriptor!
```

I am using the same TSS as core0, so that makes sense.  So, how many CPUs should I support?  For that, I need to go back to the GDT and IDT and the amount of available space between them....

I will also have some clean-up to perform as the GDT and IDT also need to be mapped into virtual memory space -- and this will need to be done after the MMU is initialized.

---

### 2019-Dec-06

This should be the mapping to the GDT and IDT:

```
<bochs:7> page 0x3fd800
 PDE: 0x00000000000000e7    PS g pat D A pcd pwt U W P
 PTE: 0x0000000000000000       g pat d a pcd pwt S R p
physical address not available for linear 0x00000000003fd000
```

...  and this all looks fubar to me.

OK, the base of the page table is at `0x3fe000`.  When I turn on paging, I have:

```
<bochs:13> page 0x3fe000
 PDE: 0x00000000000000e7    PS g pat D A pcd pwt U W P
 PTE: 0x0000000000000000       g pat d a pcd pwt S R p
 ```

 Before I enable paging, it is:

 ```
 <bochs:3> page 0x3fe000
 PDE: 0x00000000000000e7    PS g pat D A pcd pwt U W P
linear page 0x00000000003fe000 maps to physical page 0x0000003fe000
```

... which is correct.  This is a 4MB page mapping -- a large page.

Hmmm...  for CPU0:

```
<bochs:5> creg
CR0=0xe0000011: PG CD NW ac wp ne ET ts em mp PE
CR2=page fault laddr=0x0000000000000000
CR3=0x0000003fe000
    PCD=page-level cache disable=0
    PWT=page-level write-through=0
CR4=0x00000010: pke smap smep osxsave pcid fsgsbase smx vmx osxmmexcpt umip osfxsr pce pge mce pae PSE de tsd pvi vme
CR8: 0x0
EFER=0x00000000: ffxsr nxe lma lme sce
```

... while for CPU1:

```
<bochs:7> creg
CR0=0x60000011: pg CD NW ac wp ne ET ts em mp PE
CR2=page fault laddr=0x0000000000000000
CR3=0x0000003fe000
    PCD=page-level cache disable=0
    PWT=page-level write-through=0
CR4=0x00000000: pke smap smep osxsave pcid fsgsbase smx vmx osxmmexcpt umip osfxsr pce pge mce pae pse de tsd pvi vme
CR8: 0x0
EFER=0x00000000: ffxsr nxe lma lme sce
```

`cr4` has the `PSE` flag set on CPU0 whereas it is not set on CPU1.  PSE means 'page size extension'.  I bet that is relevant!

And now it appears to be a `#PF`.  The output is intermingled with real output, so it's a mess to determine what is what.

However, I can get this data from Bochs:

```
<bochs:3> creg
CR0=0xe0000011: PG CD NW ac wp ne ET ts em mp PE
CR2=page fault laddr=0x000000006a14ec83
CR3=0x0000003fe000
    PCD=page-level cache disable=0
    PWT=page-level write-through=0
CR4=0x00000010: pke smap smep osxsave pcid fsgsbase smx vmx osxmmexcpt umip osfxsr pce pge mce pae PSE de tsd pvi vme
CR8: 0x0
EFER=0x00000000: ffxsr nxe lma lme sce
<bochs:4> reg
CPU1:
rax: 00000000_00000017 rcx: 00000000_00000000
rdx: 00000000_80006dd9 rbx: 00000000_0000000e
rsp: 00000000_00001f66 rbp: 00000000_00000000
rsi: 00000000_00000000 rdi: 00000000_00000018
r8 : 00000000_00000000 r9 : 00000000_00000000
r10: 00000000_00000000 r11: 00000000_00000000
r12: 00000000_00000000 r13: 00000000_00000000
r14: 00000000_00000000 r15: 00000000_00000000
rip: 00000000_80000655
eflags 0x00000016: id vip vif ac vm rf nt IOPL=0 of df if tf sf zf AF PF cf
<bochs:5> print-stack
Stack address size 4
 | STACK 0x00001f66 [0x80001c54]
 | STACK 0x00001f6a [0x80006dd9]
 | STACK 0x00001f6e [0x0000000e]
 | STACK 0x00001f72 [0x00000000]
 | STACK 0x00001f76 [0x003fe000]
 | STACK 0x00001f7a [0x00000000]
 | STACK 0x00001f7e [0x00000000]
 | STACK 0x00001f82 [0x00000004]
 | STACK 0x00001f86 [0x80001c7c]
 | STACK 0x00001f8a [0x00001faa]
 | STACK 0x00001f8e [0x00000000]
 | STACK 0x00001f92 [0x1f9c6800]
 | STACK 0x00001f96 [0x00004871]
 | STACK 0x00001f9a [0x1fc00000]
 | STACK 0x00001f9e [0x68004bdf]
 | STACK 0x00001fa2 [0x00000004]
 | STACK 0x00001fa6 [0x80001b82]
 | STACK 0x00001faa [0x00000010]
 | STACK 0x00001fae [0x00000010]
 | STACK 0x00001fb2 [0x00000010]
 | STACK 0x00001fb6 [0x00000010]
 | STACK 0x00001fba [0x00000010]
 | STACK 0x00001fbe [0x003fe000]
 | STACK 0x00001fc2 [0x6a14ec83]
 | STACK 0x00001fc6 [0xe0000011]
 | STACK 0x00001fca [0x00000018]
 | STACK 0x00001fce [0x00000000]
 | STACK 0x00001fd2 [0x00000000]
 | STACK 0x00001fd6 [0x00001fda]
 | STACK 0x00001fda [0x00000004]
 | STACK 0x00001fde [0x00000000]
 | STACK 0x00001fe2 [0x00000008]
 | STACK 0x00001fe6 [0x800040a0]
 | STACK 0x00001fea [0x0000000e]
 | STACK 0x00001fee [0x00000000]
 | STACK 0x00001ff2 [0x00000000]
 | STACK 0x00001ff6 [0x6a14ec83]
 | STACK 0x00001ffa [0x00000008]
 | STACK 0x00001ffe [0x00010086]
 ```

So it looks like I end up almost directly into a `#PF` -- as in the stack is not that deep -- as in nothing on it.  `0x00010086` is `eflags`.

Ok, so my jump code is:

```
800090bb:       b8 a0 40 00 80          mov    $0x800040a0,%eax
800090c0:       ff 20                   jmp    *(%eax)
```

And the code bytes at `0x800040a0` are:

```
800040a0 <kInitAp>:
800040a0:       83 ec 14                sub    $0x14,%esp
800040a3:       6a 2a                   push   $0x2a
```

... or `0x6a14ec83`.  This matches the value in `cr2`.  So my problem is I am improperly jumpting to the location identified by the contents of the address I loaded into `eax`.  Fail.

However, after that change, both cpus are running.  This is a huge accomplishment.  I am going to wait for tomorrow and then test again and commit my code.

---

### 2019-Dec-08

This morning (well yesterday when I had a few minutes), I added a lock to the `kprintf()` function, so that the entire line can be printed properly.  This should help getting the data sorted out when a CPU crashes.

So, now, I need to figure out the TSS for the second core.  I commented that code out to get past that problem (the TSS was already being used).

---

I spent a little time documenting [Low Memory Usage Map](http://eryjus.ddns.net:3000/projects/century-os/wiki/Low_Memory_Usage_Map) for the physical frames.  These should all be static locations, so it will be safe to reference them directly.

I also spend some time documenting the target [GDT](http://eryjus.ddns.net:3000/projects/century-os/wiki/GDT), which will have to move.

But, I think it is time to commit this code.


