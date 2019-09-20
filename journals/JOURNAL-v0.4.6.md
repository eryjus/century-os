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




