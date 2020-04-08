# The Century OS Development Journal - v0.6.0

This version is focused on getting an integrated debugger built into the kernel.  This will be accessed over the serial port (which is already outputting debugging information).  To get to this point, there are several things that need to be addressed (grossly oversimplified):

* Reduce or eliminate all debugging output to the serial port
* Set up a better serial device driver to get input from the serial port, passing that along to the proper process
* Learn about the debugging controls that are available for each arch, and then build systems around them
* Add in structure dumps and other such queries so that I can get good output from the kernel
* Create a disassembler for each arch

## Version 0.6.0a

This first micro-version is first concerned with determining if I will be able to output ASNI escape sequences to the serial port and have them properly emulated in my screens.  I have 2 that I need be concerned with: the tty (typically from withing VSCode) and the PiBootLoader (pblserver).

### 2020-Apr-02

So, the first thing I need to do is to create a test to output decorated data to the serial port.  This should be easy enough, after which I just need to test it on each emulation model so I can determine what works and what doesn't (and what I can live with and what I can't).

* QEMU outputting to the tty works
* Bochs outputting to a different tty works
* rpi2b outputting to `pblserver` also works

This should also mean that outputting to `pblserver` from x86 will also work.  I am going to assume so and move on.

The next thing I need is a proper Serial Port driver.  I am going to work on getting it properly defined with an upper- and lower-half.  This, then, gets me into some research requirements.

---

So, the Xinu book has a reasonable discussion on the values of the upper/lower half driver architecture and its virtues.  In particular, the upper half (which is interacted with at the program/OS interface level) will be able to provide a common interface for all interactions whereas the lower half will abstract the details from the program interface.  Additionally, beyond that, the language runtime libraries may/should provide additional abstractions so that programs are portable across OSs.

So, it makes me think that I need to actually put some thought into a `tty` implementation.  Ultimately, what I am going after is a special-purpose tty terminal for debugging that is provided over the serial port.  Eventually, I might even want to tie this back into the rpi2b qemu emulation to speed the rpi2b development process.

The example Xinu implementation uses Semaphores to handle the synchronization primitives -- which I can probably wrap an `AtomicInt_t` in some `signal` and `wait` methods easy enough.

Now..!!!!  I am going for a micro-kernel.  This means that my serial port driver needs to be done outside the kernel and really needs to be set up as its own module (can I run this in user-mode?).  There are certainly a large number of firsts I need to get done for this to even be a possibility.  So...  let's revisit the goal of version 0.6.0a:
* Create a *proper* serial driver (upper- and lower-half driver)
* Create the serial driver as a module that is loaded by the bootloader
* Define a driver header that will properly identify a driver process to the kernel
* Re-create the ELF loader
    * parse the serial driver binary
    * get it distributed properly
    * create the process's own virtual memory space as required
* Get the serial driver running in user-mode as its own process
* Get the serial driver interrupt handler properly installed
* Add Semaphore support to the scheduler
* Create a SYSCALL infrastructure

That's quite a list for one version.

Let's start by getting a `serial-tty` module started in the source tree.  This will be started as a module, but will eventually end up with additional platform-specific code.

---

Almost right away, I end up with a block needing a `Semaphore_t` type implemented.  This will also need to be integrated into the scheduler.  However, this should also be relatively quick.

So, let's talk about that implementation....  There are 2 elements that are critical: the actual count and the list of processes waiting on this semaphore.  The actual count is mostly trivial -- I can use an `AtomicInt_t` type for that and do not need locks to protect that.

On the other hand, the list of waiting processes need to be protected.  I can use the `Process_t.statusQueue` element to keep track of the processes on this list.  All other lists where a `Process_t` can be added/removed are protected by `schedulerLock`.  While it does not thrill me to get a big lock like that, it also makes some sense -- I will need to also ready processes and schedule when a process unblocks.

Hmmm... but then again, this is something that is intended to be called from user mode.  Which means I need a SYSCALL interface.

This boils down to a chicken-and-egg problem.  Either I need to get back to some better basics (and maybe put off the debugger until the next version), or put the `serial-tty` driver into the kernel proper for now.

---

OK, so I think I have come to a decision: this is not going to be a `serial-tty` module but an interactive debugger.  The debugger will need to be built into the kernel.  I can/should make the debugger's serial input/output built into the kernel as well.  I may still build a tty-like structure to help, but it will be a watered-down and purpose-built structure only -- intended to be disabled at compile time if desired.

---

OK, I think I have the command line architecture set up.  I am able to prompt for a command and display the allowed responses.  This looks very similar to several of the Cisco switch prompts I have seen in the past.  The entire handling of the cursor position is accurate.  I do not have an interrupt to deal with -- I'm polling the serial port, but I think that is OK.  I also have the foundation of a finite state machine prepared (states anyway).

I think this is a good start.  Tomorrow, I plan to work on parsing the commands and advancing the state.

---

### 2020-Apr-03

Today I am going to start with debugging IPIs that stop all the cores until a request is satisfied.  Something like reading the scheduler might get dangerous if I do not have a lock....  Quick enough.

Now, the first 3 things I want to implement for the debugger are:
* Scheduler research
* Process research
* Virtual Memory research (kernel-specific for now)

The first thing I am going to look at will be the scheduler.  So, the commands to get me into that branch are:
* sched
* scheduler

And the commands I want to be able to execute are:
* show -- show all the processes on the system with their current status
* stat/status -- show the counts of the processes in each queue
* run/running -- show the processes in the running status
* ready -- show the processes in a ready list by priority
    * all -- show all the processes ready to run
    * OS -- show the processes in the OS queue
    * Hi/High -- show the processes in the High Queue
    * Norm/Normal -- show the processes in the Normal Queue
    * Low -- show the processes in the Low Queue
    * Idle -- show the processes in the Idle Queue
* list -- show the processes in a list
    * Blocked -- show the processes that are blocked
    * Sleeping -- show the processes in the sleeping queue
    * Zombie -- show the zombied processes waiting to be cleaned up

Now, for this to work, I need to develop a `strcmp()` function.  And of course, I want it to be efficient.

---

I was thinking that I would draw some boxes, but I do not have the ability in `kprintf()` to set a width.  So, for now, I am going to work on just text-based output -- nothing fancy.

So, it appears that Pi-Bootloader is not passing characters on to the physical hosts.  I will have to sort that out.

---

### 2020-Apr-04

I got this sorted out.  Turns out that there were problems on both sides of the conversation.  I cleaned up the pi-bootloader and pushed a commit there.

Now, with that, I have some problems to solve:
1. The rpi2b is not booting anymore.  It deadlocks somewhere along the lines.
1. After executing a `sched stat` command, the debugger loses its state.  After another command, the system deadlocks.

By the way, this is working properly on real x86 hardware.  So, I'm happy about that!

Since I think the problem with the rpi is related to terminating a process, I am going to bypass that step for now.  And as I thought the rpi boots.

So, this is what I have going on:

```
Dumping the status of the scheduler:
The scheduler is unlocked
       OS Queue process count: 0
     High Queue process count: 0
   Normal Queue process count: 0
      Low Queue process count: 0
     Idle Queue process count: 4
   Blocked List process count: 0
  Sleeping List process count: 1
Terminated List process count: 0
```

Let's see here, I should have:
* 4 idle tasks
* 1 Debugger task running
* 1 kInit() task most likely sleeping
* 3 kInitAp() tasks running in a busy loop

I have not yet built visibility to the running tasks, so take those 4 off the table, and the list looks good to me.

OK, but this is interesting!!!

```
Welcome to the Century-OS kernel debugger
- :> sched stat
Dumping the status of the scheduler:
The scheduler is unlocked
CPU0 does not have a process running on it
CPU1 does not have a process running on it
CPU2 does not have a process running on it
CPU3 does not have a process running on it
       OS Queue process count: 0
     High Queue process count: 0
   Normal Queue process count: 0
      Low Queue process count: 0
     Idle Queue process count: 4
   Blocked List process count: 0
  Sleeping List process count: 1
Terminated List process count: 0
sched :>
 (allowed: show,status,run,ready,list,exit)
```

Ahhh... but that is the problem with the ARM arch -- the current process is stored in a per-CPU register, not this structure.  I will need to clean that up to get this working better.

Better:

```
Welcome to the Century-OS kernel debugger
- :> sched
sched :> status
Dumping the status of the scheduler:
The scheduler is unlocked
CPU0 does have a process running on it
CPU1 does have a process running on it
CPU2 does have a process running on it
CPU3 does have a process running on it
       OS Queue process count: 0
     High Queue process count: 0
   Normal Queue process count: 0
      Low Queue process count: 0
     Idle Queue process count: 4
   Blocked List process count: 0
  Sleeping List process count: 1
Terminated List process count: 0
sched :>
 (allowed: show,status,run,ready,list,exit)
```

So...  now to get the debugger interface cleaned up a bit more.

OK, so it appears to be deadlocking in `DebuggerEngage()`.  I bet I missed a `volatile` on the memory location.

Nope!!  I forgot to issue an EOI!  D'oh!

Next, let's get the exit command working.

Now, I have moved on to dumping the contents of the running processes.  Interesting thing, I am finding all kinds of little bugs that I would not have found before -- like the number of characters printed returned by `kprintf()` when a string pointer is NULL.

Now that said, I have this output:

```
+------------------------+----------------+----------------+----------------+----------------+
| CPU                    |       CPU0     |       CPU1     |       CPU2     |       CPU3     |
+------------------------+----------------+----------------+----------------+----------------+
| Process Address:       | 0x90000238     | 0x9000028c     | 0x900002e0     | 0x900001e4     |
| Process ID:            | 6              | 7              | 8              | 5              |
| Command:               | <NULL>         | <NULL>         | <NULL>         | <NULL>         |
| Virtual Address Space: | 0x1001000      | 0x1001000      | 0x1001000      | 0x1001000      |
| Base Stack Frame:      | 0x0            | 0x0            | 0x1026         | 0x0            |
| Status:                | 1              | 1              | 1              | 1              |
| Priority:              | 30             | 30             | 30             | 30             |
| Quantum Left:          | 29             | 7              | 28             | 23             |
| Time Used:             | 7774000        | 7857000        | 18000          | 7597000        |
| Wake At:               | 0              | 0              | 0              | 0              |
| Queue Status:          |                |                |                |                |
+------------------------+----------------+----------------+----------------+----------------+
sched :>
```

So, a couple of things to take care of here:
* I want a name for each process and I will add that into `ProcessCreate()` along with the statically created processes.
* I want to change statuses from decimal to strings
* I want to change priority from decimal to strings
* I think I want to support 64-bit numbers in `kprintf()`

---

Cleaning up the first 3 bullets, I have found a bug in the rpi2b code:

```
+------------------------+-------------------+-------------------+-------------------+-------------------+
| CPU                    |        CPU0       |        CPU1       |        CPU2       |        CPU3       |
+------------------------+-------------------+-------------------+-------------------+-------------------+
| Process Address:       | 0x90000040        | 0x90000310        | 0x900003a0        | 0x90000430        |
| Process ID:            | 0                 | 5                 | 6                 | 7                 |
| Command:               | kInit             | kInitAp(1)        | kInitAp(2)        | kInitAp(3)        |
| Virtual Address Space: | 0x1000000         | 0x1000000         | 0x1000000         | 0x1000000         |
| Base Stack Frame:      | 0xff801000        | 0x0               | 0x0               | 0x0               |
| Status:                | DLYW              | RUNNING           | RUNNING           | RUNNING           |
| Priority:              | OS                | OS                | OS                | OS                |
| Quantum Left:          | 2082              | 10                | 24                | 7                 |
| Time Used:             | 91000             | 10876000          | 63153000          | 48849000          |
| Wake At:               | 138680000         | -1208077377       | -1466647602       | 358705100         |
| Queue Status:          | On some queue     | Not on a queue    | Not on a queue    | Not on a queue    |
+------------------------+-------------------+-------------------+-------------------+-------------------+
sched :>
 (allowed: show,status,run,ready,list,exit)
 ```

I had no idea I had this bug.  Now, I do not think it's material (to the kernel that is) since I just added that support to print the valid process addresses.  Most likely, I am not updating the process in the `cpus` structure when I swap a process.

---

That fixed, I now see this:

```
+------------------------+-------------------+-------------------+-------------------+-------------------+
| CPU                    |        CPU0       |        CPU1       |        CPU2       |        CPU3       |
+------------------------+-------------------+-------------------+-------------------+-------------------+
| Process Address:       | 0x90000280        | 0x900003a0        | 0x90000310        | 0x900004c0        |
| Process ID:            | 4                 | 6                 | 5                 | 8                 |
| Command:               | Idle Process�㔯���| kInitAp(2)        | kInitAp(1)        | Kernel Debugger�9�~O����y�L���'ܺ��|
| Virtual Address Space: | 0x1000000         | 0x1000000         | 0x1000000         | 0x1000000         |
| Base Stack Frame:      | 0x102d            | 0x0               | 0x0               | 0x102e            |
| Status:                | RUNNING           | RUNNING           | RUNNING           | RUNNING           |
| Priority:              | IDLE              | OS                | OS                | OS                |
| Quantum Left:          | 0                 | 18                | 3                 | 35                |
| Time Used:             | 956000            | 19508000          | 3415000           | 0                 |
| Wake At:               | -554102895        | -1466647602       | -1208077377       | -723395477        |
| Queue Status:          | Not on a queue    | Not on a queue    | Not on a queue    | Not on a queue    |
+------------------------+-------------------+-------------------+-------------------+-------------------+
Invalid command.  Available commands are below the input line.  Use 'help' for a detailed description.


Something went wrong (scheduler) -- a bug in the debugger is likely
sched :>
 (allowed: show,status,run,ready,list,exit)
```

This shows me a few things:
1. `kStrCpy()` likely has a bug on armv7
1. The 'Idle Process' is getting CPU time when it should not
1. The Wake At values are not set properly
1. The 'Kernel Debugger' is getting CPU time, but not reporting it properly
1. The 'Kernel Debugger' has a quantum more than its Priority would typically allow

From what I can tell, the Idle Process description is located at `8080883c`, which contains the following bytes (in hex):

`49 64 6c 65 20 50 72 6f 63 65 73 73 00`, which translates to 'Idle Process'.

Ahhh... but my `kStrCpy()` does not copy the terminating NULL.

From C++ Reference `strcpy()`:

> Copies the C string pointed by source into the array pointed by destination, including the terminating null character (and stopping at that point).

So I need to clean up my own code to also copy the terminating NULL.

This is improving, but I am not sure how the `Kernel Debugger` has more than 30 quantum left:

```
+------------------------+-------------------+-------------------+-------------------+-------------------+
| CPU                    |        CPU0       |        CPU1       |        CPU2       |        CPU3       |
+------------------------+-------------------+-------------------+-------------------+-------------------+
| Process Address:       | 0x90000430        | 0x900003a0        | 0x90000310        | 0x900004c0        |
| Process ID:            | 7                 | 6                 | 5                 | 8                 |
| Command:               | kInitAp(3)        | kInitAp(2)        | kInitAp(1)        | Kernel Debugger   |
| Virtual Address Space: | 0x1000000         | 0x1000000         | 0x1000000         | 0x1000000         |
| Base Stack Frame:      | 0x0               | 0x0               | 0x0               | 0x102e            |
| Status:                | RUNNING           | RUNNING           | RUNNING           | RUNNING           |
| Priority:              | OS                | OS                | OS                | OS                |
| Quantum Left:          | 19                | 12                | 23                | 41                |
| Time Used:             | 3553000           | 11321000          | 1541000           | 0                 |
| Wake At:               | 0                 | 0                 | 0                 | 0                 |
| Queue Status:          | Not on a queue    | Not on a queue    | Not on a queue    | Not on a queue    |
+------------------------+-------------------+-------------------+-------------------+-------------------+
sched :>
 (allowed: show,status,run,ready,list,exit)
```

Well, it turns out I was adding the quantum twice for rpi....   Hmmm...  that's not right either:

```
+------------------------+-------------------+-------------------+-------------------+-------------------+
| CPU                    |        CPU0       |        CPU1       |        CPU2       |        CPU3       |
+------------------------+-------------------+-------------------+-------------------+-------------------+
| Process Address:       | 0x90000280        | 0x90000430        | 0x90000310        | 0x900004c0        |
| Process ID:            | 4                 | 7                 | 5                 | 8                 |
| Command:               | Idle Process      | kInitAp(3)        | kInitAp(1)        | Kernel Debugger   |
| Virtual Address Space: | 0x1000000         | 0x1000000         | 0x1000000         | 0x1000000         |
| Base Stack Frame:      | 0x102d            | 0x0               | 0x0               | 0x102e            |
| Status:                | RUNNING           | RUNNING           | RUNNING           | RUNNING           |
| Priority:              | IDLE              | OS                | OS                | OS                |
| Quantum Left:          | -7433             | -29561            | -29838            | 30                |
| Time Used:             | 7438000           | 29561000          | 29888000          | 0                 |
| Wake At:               | 0                 | 0                 | 0                 | 0                 |
| Queue Status:          | Not on a queue    | Not on a queue    | Not on a queue    | Not on a queue    |
+------------------------+-------------------+-------------------+-------------------+-------------------+
```

Now that I look at it again, I think this is from the first time the process is scheduled.  I am pre-setting the quantum to the the priority and when the task gets scheduled again the quantum get added back in.  In nearly all of these cases, the quantum is getting doubled.  The key here is that the running processes should get a quantum (`ProcessInit()` and `kInitAp()`) whereas the processes created by `ProcessCreate()` should be set to 0 (since they are not running).

That's looking much better:

```
+------------------------+-------------------+-------------------+-------------------+-------------------+
| CPU                    |        CPU0       |        CPU1       |        CPU2       |        CPU3       |
+------------------------+-------------------+-------------------+-------------------+-------------------+
| Process Address:       | 0x900003a0        | 0x900004c0        | 0x90000310        | 0x90000430        |
| Process ID:            | 6                 | 8                 | 5                 | 7                 |
| Command:               | kInitAp(2)        | Kernel Debugger   | kInitAp(1)        | kInitAp(3)        |
| Virtual Address Space: | 0x1000000         | 0x1000000         | 0x1000000         | 0x1000000         |
| Base Stack Frame:      | 0x0               | 0x102e            | 0x0               | 0x0               |
| Status:                | RUNNING           | RUNNING           | RUNNING           | RUNNING           |
| Priority:              | OS                | OS                | OS                | OS                |
| Quantum Left:          | 5                 | 18                | 12                | 7                 |
| Time Used:             | 4359000           | 0                 | 331000            | 2347000           |
| Wake At:               | 0                 | 0                 | 0                 | 0                 |
| Queue Status:          | Not on a queue    | Not on a queue    | Not on a queue    | Not on a queue    |
+------------------------+-------------------+-------------------+-------------------+-------------------+
```

So, what's left?  The time accounting is not right and I believe the Idle Process still gets CPU time.  Well..., maybe I did fix the idle process by updating it's quantum properly when it wakes.  I won't know that for sure until I get to `sched show` where I dump all the running processes.

OK, time accounting is messed up because the variables are global, not per-CPU as they should be.  This will have to be cleaned up as well.

To get the time accounting worked out, I think I am going to need to start a set of `DebugTimer*()` functions.  The problem I believe I am having is that a process is picking up a timer value from one core and then comparing it to a timer value from another core...  resulting is odd timings.  Now, that said, I also believe that there is still a problem with why `Kernel Debugger` is not showing any CPU time -- which I need to get to the bottom of that.

Also, for the record, I am still seeing `Idle Process` running when I do not believe it should.  In fact, I have seen this get CPU time on the x86-pc target as well.  Ahhh....  I think I know what is going on with this.  A kernel job is running and the timer says it runs out of quantum.  So, the scheduler goes and looks for another job that is in the Ready queue.  It finds one (the currently running job is not on the ready queue, so it cannot check against its priority).  In short, the lower priority job is preempting the higher priority one.  To solve this problem, I will need to pass in the priority of the current running job and only check queues that are at the same or higher priority.

I think I need to get some of this stuff into Redmine.

I added 2:
* [Redmine #459](http://eryjus.ddns.net:3000/issues/459) -- Idle process getting CPU time when it should not.
* [Redmine #460](http://eryjus.ddns.net:3000/issues/460) -- `Kernel Debugger` not getting accounted for CPU time on rpi2b.

I have not added anything about the timer because I cannot prove what the problem is yet.  In the meantime, I am going to leave this running overnight so I can see how my 64-bit numbers come out in the morning.

---

### 2020-Apr-05

Well, I think this qualifies for getting into the 64-bit range:

```
+------------------------+-------------------+-------------------+-------------------+-------------------+
| CPU                    |        CPU0       |        CPU1       |        CPU2       |        CPU3       |
+------------------------+-------------------+-------------------+-------------------+-------------------+
| Process Address:       | 0x900003f8        | 0x90000370        | 0x90000480        | 0x900002e8        |
| Process ID:            | 7                 | 6                 | 8                 | 5                 |
| Command:               | kInitAp(3)        | kInitAp(2)        | Kernel Debugger   | kInitAp(1)        |
| Virtual Address Space: | 0x1001000         | 0x1001000         | 0x1001000         | 0x1001000         |
| Base Stack Frame:      | 0x100e            | 0x100d            | 0x1026            | 0x100c            |
| Status:                | RUNNING           | RUNNING           | RUNNING           | RUNNING           |
| Priority:              | OS                | OS                | OS                | OS                |
| Quantum Left:          | 10                | 26                | 12                | 18                |
| Time Used:             | 567312130000      | 567309095000      | 583126422000      | 567514601000      |
| Wake At:               | 0                 | 0                 | 0                 | 0                 |
| Queue Status:          | Not on a queue    | Not on a queue    | Not on a queue    | Not on a queue    |
+------------------------+-------------------+-------------------+-------------------+-------------------+
sched :>
 (allowed: show,status,run,ready,list,exit)
 ```

Today I am going to start on a timer dump.  I want to be able to dump the timer counters for each core every second.  To do this, I will need to alert each core to query its timer and load a structure and when all the cores report in, I print them all on the screen.

To do this, I will make use of the Debug IPI and add functionality to that feature.  I will also need a communication structure -- easy enough to develop.  Finally, I also want to clean up the handling of the `debugCommand` buffer since it is not handled properly -- or move that to Redmine so I do not forget it.

What the heck!?!?!?  `"in function `__aeabi_idiv0': undefined reference to `raise'"`!!!

So, not what I wanted to be doing today, but I am rebuilding my toolchain and working on getting that set properly now.  I hope this work -- building the toolchain can be fussy at best.

So, as an example, `isl` requires `aclocal` v1.15 and I have v1.16.1.  I had to back-level the `isl` library to v16.1.

Ultimately, my concern will be that I lose the hard-float capability and have to write a soft-float library.

OK, getting everything in sync, it worked,  Well, the compiler compiled.  But the rpi will not boot.

Disassembling the new code, I get the following:

```
00100030 <entry>:
  100030:       ee103fb0        mrc     15, 0, r3, cr0, cr0, {5}
  100034:       e2033003        and     r3, r3, #3
  100038:       e3530000        cmp     r3, #0
  10003c:       1a0001a1        bne     1006c8 <entryApHold>
```

and the armv7* code was:

```
00100030 <entry>:
  100030:       ee103fb0        mrc     15, 0, r3, cr0, cr0, {5}
  100034:       e2033003        and     r3, r3, #3
  100038:       e3530000        cmp     r3, #0
  10003c:       1a0001a1        bne     1006c8 <entryApHold>
```

Side-note: a conversation from `freenode#osdev` to follow-up on later is:

```
[11:01:29] <eryjus> thx heat, that was spot on
[11:01:56] <heat> eryjus, no problem :)
[11:03:19] <eryjus> or...  maybe not.. the rpi won't boot now
[11:07:51] <clever> eryjus: arm32 or arm64, which pi model?
[11:08:02] <eryjus> rpi2b -- 32-bit
[11:08:16] <clever> eryjus: got a jtag adapter, or a 2nd pi (any model)?
[11:08:43] <eryjus> i have a 3 & 4
[11:09:41] <clever> eryjus: this is an openocd config file, https://gist.github.com/cleverca22/8813d3b969ad6335285be0be5e208611
[11:09:42] <bslsk05> ​gist.github.com: rpi4-to-rpi2.tcl · GitHub
[11:09:56] <clever> eryjus: which configures an rpi4 to debug an rpi2 over jtag, using gpio bit-banging
[11:10:46] <clever> 2020-02-04 18:02:15 <@clever_> tck=11, tms=25, tdi=10, tdo=9, trst=7
[11:10:58] <clever> and what the 4 pins on line 2 are called
[11:11:32] <clever> eryjus: if you add `enable_jtag_gpio=1` to `config.txt` on the rpi2, then the firmware will put the right pins (different ones) into jtag mode
[11:12:53] <clever> eryjus: https://pinout.xyz/pinout/jtag that would be the jtag pins for the rpi2
[11:12:53] <bslsk05> ​pinout.xyz: JTAG at Raspberry Pi GPIO Pinout
[11:13:39] <clever> eryjus: then you simply connect each pair up, rpi2 tdi (4) to rpi4 tdi (10), making sure to map it back from broadcom to header#
[11:14:00] <clever> (gdb) target remote :3333
[11:14:01] <clever> (gdb) load /home/clever/apps/rpi-open-firmware/result/arm_chainloader.bin.elf
[11:14:02] <eryjus> clever, don't you need some jtag hardware for that?
[11:14:18] <clever> eryjus: the rpi4 can emulate the jtag adapter, by bit-banging its gpio pins
[11:15:33] <clever> `openocd -f rpi4-to-rpi2.tcl` will launch openocd, and connect to the rpi2 over jtag
[11:15:45] <clever> the `target remote :3333` will tell gdb to then connect to openocd over tcp, localhost:3333
[11:16:04] <clever> but you can use `ssh -L` to forward that from your x86 system to the rpi4
[11:16:47] <clever> eryjus: once fully working, you can load a .elf file for your kernel, halt the arm, and grab a backtrace
[11:17:08] <clever> symbol-file is used to tell gdb the symbols
[11:17:23] <clever> load is used to copy a .elf directly into target ram, and update the PC to the entry-point
[11:37:39] <eryjus> clever, thanks for that.  I will have to build a cable.  I don't think I have everything in my small parts box to get it to work...  which means I gotta order some stuff.  But I will definitely look into it
[11:38:18] <clever> eryjus: one min
[11:40:00] <clever> eryjus: https://imgur.com/y7Q6Nbk
[11:40:01] <bslsk05> ​imgur.com: Imgur: The magic of the Internet
[11:40:10] <clever> eryjus: an rpi4 cross-wired to an rpi3, doing jtag debug
[11:40:42] <clever> eryjus: an ftdi is also wired to the rpi3, and the only thing the breadboard is for is to tie the gnd of the 2 pi's and the ftdi
[11:42:39] <clever> eryjus: most of the connections are just https://www.sparkfun.com/products/8430, but the gnd's and ftdi are using M/F wires
[11:42:41] <bslsk05> ​www.sparkfun.com: Jumper Wires Premium 6" F/F Pack of 10 - PRT-08430 - SparkFun Electronics
```

and now going back to the old compiler, the rpi will no longer boot.  x86 still boots, so that is good.

OK, it looks like ny 64-bit numbers implementation really broke the rpi.

---

OK, that mess set aside, x86 shows the following with the timer:

```
Current Timer Counts (press <Enter> to exit)
CPU0 | 1793697000
CPU1 | 1793698000
CPU2 | 1793698000
CPU3 | 1793698000
```

The good news is that the timer remains in sync across all cores.  But, the frequency appears to be off by a factor of about 10.  The above should by in microseconds, so divide by 1000000.  This means my session was active for 1793 seconds (or, 29 minutes).  This is wrong.  An unscientific test of about 10 seconds shows:

```
Current Timer Counts (press <Enter> to exit)
CPU0 | 47741000
CPU1 | 47741000
CPU2 | 47741000
CPU3 | 47741000
- :>
 (allowed: scheduler,timer)
```

 So, maybe a factor of 5.

 The rpi hardware has a similar problem (same count but running too fast):

 ```
 Current Timer Counts (press <Enter> to exit)
CPU0 | 88671000
CPU1 | 88671000
CPU2 | 88671000
CPU3 | 88671000
- :>
 (allowed: scheduler,timer)
```

The Local APIC needs to be calibrated whereas the rpi works on a fixed frequency.  It will be easier to track down the rpi problem.

Hmmm...  let's get some information from timer device `timerControl`.  What do I want to see?
* Clock Frequency
* Timer Frequency
* !!

Wait!  Could it be that the frequency is really off by the number of cores I have running?  Each core would then update this count, making it invalid.

OK, this is working better, having changed the timer to only update for CPU0.  rpi looks reasonable accurate, and qemu runs a little fast.  But, x86 hardware is not running.  OK, bug in the code where I was checking for 4 cores (hard-coded) where my test laptop has only 2 cores.  ...  and then a whole lot of additional debugging.

In short, this is issuing times about properly on real hardware -- I am not going to worry about qemu.

Actually, I think I have enough changes to justify a push for the changes so far.  I will stay on this micro-version.

---

OK, let's go back to [Redmine #459](http://eryjus.ddns.net:3000/issues/459).  This should be an easy fix.

I was also able to take care of #460 at the same time.

With that, I really want to figure out the correct toolchain issues.  So, I am going to change back to the other one (the correct one) and see if I can't get it working properly.

OK, so I get this far:

```
CPU 0x1 running...
kInitAp() established the current process at 0x900002e8 for CPU1
Enabling interrupts on CPU 1
Cpus running is 2
Starting core with message to 0xf90000ac
..  waiting for core to start...
Timer Initialized
CPU 0x2 running...
kInitAp() established the current process at 0x90000370 for CPU2
Enabling interrupts on CPU 2
Cpus running is 3
```

OK, interesting... on the rpi, the timer appears to be getting disabled on CPU0.

I have not yet figured out why....  OK, I have a problem with a sleep there the resulting time has already passed.  Such as: `ProcessMicroSleep(0)`.

OK, so an explicit `EnableInterrupts()` call fixes my problem....  So, does this mean that the `ProcessStart()` function is not being called?

```c++
EXTERN_C EXPORT KERNEL
void DebugStart(void)
{
    EnableInterrupts();
```

OK, so the problem is that the `ProcessStart()` function would restore interrupts when it executed `ProcessUnlockScheduler()`, but would not explicitly enable interrupts.  I added an explicit enable after unlocking the scheduler.  This now works much better.

The last thing I wanted to do today was to get a 64-bit int version of `kprintf()` working.  There's something about the 64-bit division that is making the kernel not run on the rpi2b.  I can't figure it out yet.

---

### 2020-Apr-06

OK, so I found the following function in Linux yesterday:

https://github.com/spotify/linux/blob/master/lib/div64.c

This function performs the division by basically repeatedly subtracting the dividend from the divisor.  There is some scaling that happens, but the algorithm can be inefficient -- especially for very small dividends compared to the divisor.  This is what I am looking to do: really big number of microseconds since boot divided by 10 -- over and over.

For now, I will just add these comments to the Redmine.

---

For [Redmine #460](http://eryjus.ddns.net:3000/issues/460), this is working properly for x86-pc, but not rpi2b.  Also, the processes are migrating around the CPUs as `kInit()` wakes.  On the RPi, they do not migrate -- and I do not believe the timer is firing.

Hmmm....

```
sched :> stat
Dumping the status of the scheduler:
The scheduler is unlocked
CPU0 does have a process running on it
CPU1 does have a process running on it
CPU2 does have a process running on it
CPU3 does have a process running on it
       OS Queue process count: 0
     High Queue process count: 0
   Normal Queue process count: 1
      Low Queue process count: 0
     Idle Queue process count: 0
   Blocked List process count: 0
  Sleeping List process count: 0
Terminated List process count: 0
sched :>
 (allowed: show,status,run,ready,list,exit)
```

I lost the idle processes and and for some reason the `kInit()` process has been moved to the Normal queue.  Now, that said, the `Kernel Debugger` process is getting its quantum reset, so the timer has to be firing but the timer is not being updated.

OK, so I replaced the crappy timer IRQ counter with a better query of the hardware timer counter.  This is properly updating.  I still cannot say I am getting proper timer interrupts, but the quantum is changing for all processes.

Oh wait!!

```
+------------------------+-------------------+-------------------+-------------------+-------------------+
| CPU                    |        CPU0       |        CPU1       |        CPU2       |        CPU3       |
+------------------------+-------------------+-------------------+-------------------+-------------------+
| Process Address:       | 0x90000480        | 0x900002e8        | 0x90000370        | 0x900003f8        |
| Process ID:            | 618               | 5                 | 6                 | 7                 |
| Command:               | Kernel Debugger   | kInitAp(1)        | kInitAp(2)        | kInitAp(3)        |
| Virtual Address Space: | 0x1000000         | 0x1000000         | 0x1000000         | 0x1000000         |
| Base Stack Frame:      | 0x102e            | 0x1015            | 0x1016            | 0x1017            |
| Status:                | RUNNING           | RUNNING           | RUNNING           | RUNNING           |
| Priority:              | Unknown!          | OS                | OS                | OS                |
| Quantum Left:          | 10                | 5                 | 5                 | 4                 |
| Time Used:             | 0                 | 0                 | 0                 | 0                 |
| Wake At:               | 0                 | 0                 | 0                 | 0                 |
| Queue Status:          | Not on a queue    | Not on a queue    | Not on a queue    | Not on a queue    |
+------------------------+-------------------+-------------------+-------------------+-------------------+
sched :>
 (allowed: show,status,run,ready,list,exit)
```

I have a problem in the assembly again where the members are in the wrong location.  Did this happen when I changed toolchains??!

Whoa!!! changing the toolchain for armv7 also changed the location of structure members -- it is packing structures I did not expect.

OK, that corrected, I still have a problem with a couple of things on rpi2b:
* the Time Used field is not being updated
* the idle tasks are being 'lost'

OK, so the change from `gnueabi` to `eabi` changed the structure packing and also the member sizes.  Once I made the member sizes right (as in `ldrb`), the idle tasks worked again.

Finally!  Changing back the debugging code to not use 64-bit numbers changed the parameter value and therefore changed the results -- for the better.  Now, the processes are not migrating like I think they should, but that could just be a feature of the different archs.  The `kInit()` process is getting time properly.

So, this is working.

What to look at next???  Well, I am really interested in getting `sched show` working.  This will require a change to the `Process_t` structure to allow for a global process list.  That will be for tomorrow.

---

### 2020-Apr-07

So for the `shced show` command...  I first need to create a global list of processes.  I think I will bury that in the scheduler.  Then, there are a few places I will need to update to add processes into that list:
* `ProcessInit()` for the `kInit()` process
* `kInitAp()` for their respective processes
* `ProcessCreate()` for all the other processes

I will also need to maintain this list once I have the butler running and cleaning up.

Ok, that done, I need to start the Debug process to dump this list.  This should (at least in theory) be ordered by PID -- I am adding to the tail but I am making no effort to ensure this order.  If PID every wraps, things will be out of PID order but should still be in the order they were created.

So, what information do I want to see?  Well, this is my inspiration for this screen:

```
                             Work with Active Jobs                     SYSTEM
                                                         04/07/20  18:48:04 EDT
 CPU %:      .0     Elapsed time:   00:00:00     Active jobs:   794
                     Current
 Opt  Subsystem/Job  User        Type  CPU %  Function        Status
      BFCDAKOTA      QSYS        SBS      .0                   DEQW
        QP0ZSPWT     TOMDKTADC   BCI      .0  JVM-org.apache   SELW
      JDEE910        QSYS        SBS      .0                   DEQW
        JDENET_K     ONEWORLD    BCI      .0  PGM-JDENET_K     DEQW
        JDENET_K     ONEWORLD    BCI      .0  PGM-JDENET_K     DEQW
        JDENET_K     ONEWORLD    BCI      .0  PGM-jvmStartPa   DEQW
        JDENET_K     ONEWORLD    BCI      .0  PGM-jvmStartPa   DEQW
        JDENET_K     ONEWORLD    BCI      .0  PGM-JDENET_K     DEQW
        JDENET_K     ONEWORLD    BCI      .0  PGM-JDENET_K     DEQW
        JDENET_K     ONEWORLD    BCI      .0  PGM-JDENET_K     DEQW
        JDENET_K     ONEWORLD    BCI      .0  PGM-JDENET_K     DEQW
        JDENET_K     ONEWORLD    BCI      .0  PGM-JDENET_K     DEQW
        JDENET_K     ONEWORLD    BCI      .0  PGM-JDENET_K     DEQW
        JDENET_K     ONEWORLD    BCI      .0  PGM-JDENET_K     DEQW
        JDENET_K     ONEWORLD    BCI      .0  PGM-JDENET_K     DEQW
                                                                        More...
 ===>
 F21=Display instructions/keys
 ```

So, for my implementation of this list, I am wanting the following information:
* Command
* PID
* Priority
* Status
* Address
* Time Used

I can add to this list if I need to.

I'm pleased with this so far:

```
+---------------------------+--------+----------+----------+------------+-----------------------+
| Command                   | PID    | Priority | Status   | Address    | Time Used             |
+---------------------------+--------+----------+----------+------------+-----------------------+
| kInit                     | 0      | OS       | DLYW     | 0x90000040 | 0x00000000 0x00985fd0 |
| Idle Process              | 1      | IDLE     | READY    | 0x900000d0 | 0x00000000 0x00007918 |
| Idle Process              | 2      | IDLE     | READY    | 0x90000160 | 0x00000000 0x00007918 |
| Idle Process              | 3      | IDLE     | READY    | 0x900001f0 | 0x00000000 0x00007918 |
| Idle Process              | 4      | IDLE     | READY    | 0x90000280 | 0x00000000 0x00007918 |
| kInitAp(1)                | 5      | OS       | RUNNING  | 0x90000310 | 0x00000000 0x063c67b0 |
| kInitAp(2)                | 6      | OS       | RUNNING  | 0x900003a0 | 0x00000000 0x063b2760 |
| kInitAp(3)                | 7      | OS       | RUNNING  | 0x90000430 | 0x00000000 0x063b46a0 |
| Kernel Debugger           | 8      | OS       | RUNNING  | 0x900004c0 | 0x00000000 0x063b5e10 |
+---------------------------+--------+----------+----------+------------+-----------------------+
sched :>
 (allowed: show,status,run,ready,list,exit)
```

So, at this point, I can confirm that the idle processes are not getting CPU -- there is no need yet.  Also, `kInit()` gets a little cpu about every 2 seconds.  Both are what I would expect.

So, the next thing is that I think I have enough in place properly terminate the `kInitAp()` processes.

And when I try to allow this, I get a `#PF`:

```
Starting processes

Page Fault
CPU: 3
EAX: 0x01001000  EBX: 0x00000028  ECX: 0x01001000
EDX: 0x00000000  ESI: 0x00000028  EDI: 0x00000028
EBP: 0x00000048  ESP: 0xff800ebc  SS: 0x10
EIP: 0x01001000  EFLAGS: 0x00200046
CS: 0x8  DS: 0x28  ES: 0x28  FS: 0x28  GS: 0x90
CR0: 0xe0000011  CR2: 0x01001000  CR3: 0x01001000
Trap: 0xe  Error: 0x0
```

What's more, I am trying to execute the paging tables!!!

So, `ProcessTerminate()` really only needs to block itself just like `ProcessMicroSleepUntil()`.

I did replace `ProcessTerminate()` with `ProcessEnd()`, which will terminate the current process.  There is a flaw with `ProcessTerminate()` which will fail miserably if it is called against a process currently on a CPU.  I have protected the function with an `assert(false)` statement, but it may need to get fixed at some point.

Also, I ended up creating a barrier against which all the `kInitAp()` processes waited until released so that the processes could terminate properly.  The problem was that CPU1 was ready to terminate before CPU3 was started, and CPU0 was not in a position to deal with a reschedule yet.  In other words, `picControl->ipiReady = true;` had not been executed.

I got to this point far faster than I had thought I would.  Now I gotta find something new to work on.

---

OK, I have a few ideas:
* Continue to add functionality to the debugging (but these may be things I may not need for some time)
* Add messaging to the kernel (IPC is really the only feature missing in the kernel at this point)
* Turn the `kInit()` process into the `Butler()` process and start cleaning up processes
* Start the `PmmSanitize()` process to clean frames
* Re-enable user mode
* Perform the post-boot cleanup (likely integrated into the `Butler()` initialization)

No matter which directions I go, it is time to commit this and merge into v0.6.0.

---

OK, so for the user processes to work, I am going to need SYSCALLS implemented.  The `PmmSanitize()` function should run as a user process.  Adding debugging functionality before I really need it (frankly) sounds boring.  I think the next thing is going to be to get messaging and other IPC working in the kernel.

For messaging, however, I need to make sure I am building for Posix.  So, we start researching.  Also, I think I need to tickle the version.