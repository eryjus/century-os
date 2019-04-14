# The Century OS Development Journal - v0.4.1

## Version 0.4.1

The 0.4.x series of versions will enable SMP on both architectures.  This is all new things for me, so I do expect this to take a lot of work to get dialled in.

There is a roadmap I have developed:
* v0.4.0 -- enable caches and branch prediction on the arm cpu; and debug the resulting mess
* v0.4.1 -- replace the 8259 PIC with the Local APIC on x86-pc arch
* v0.4.2 -- implement atomic integers and replace the foundation of `Spinlock_t` implementation with `AtomicInt_t`
* v0.4.3 -- implement inter-process communication (IPC) in near-compliance with Posix
* v0.4.4 -- implement messaging in near-compliance with Posix
* v0.4.5 -- implement inter-processor interrupts and CPU-specific memory/structures
* v0.4.6 -- finally enable the APs; cleanup the resulting mess

---

### 2019-Apr-05

I have downloaded the ACPI specification and my chore today is to get familiar with it.  I may start laying structure definitions if I am feeling ambitious.

The first thing to do is to find the `RSDP` Root System Description Pointer.  This "pointer" is actually a table that will help identify its location.

This table should be located in low memory (as in < 1MB).  However, the rest of the ACPI tables are not guaranteed to be in mapped memory space.  Therefore, the loader will need to take care of this search to identify the `RSDP` and the save the results in the hardware discovery structure.

The `EarlyInit()` functions is architecture specific.  However, I can argue that ACPI and APIC are both platform level implementations.  Therefore, I will need to add a `PlatformEarlyInit()` function call to `EarlyInit()`, and provide that function from the pc platform.  Technically, I should so the same for arm.  If we want to get really technical about this, `SerialOpen()` should be called from `PlatformEarlyInit()`, but I am not going to do that since it is strictly related to debugging and will eventually be conditioned with preprocessor directives.

---

Having coded what is necessary to find the RSDP, I am not finding it on qemu.  Maybe it's the arch or maybe it's parameters I am passing to the emulator.  Not sure.

`bochs` reports that the APIC is enabled, but I cannot find the RSDP on that emulator either.  So, I have a bug in my code. As it turns out, my slick little trick to compare the signature is not as slick as I thought.

---

Cleaning up my code worked.  I'm calling it a night.

---

### 2019-Apr-06

So, now I am at a decision point.  In both qemu and bochs, the location of the rsdt is outside mapped memory and the xsdt does not exist.  But I am also executing the `PlatformEarlyInit()` function well after the MMU is up and running.  However, with that said, I have the following from the grub memory map:

```
iterating in mmap - size is: 0x00000030
    entry address is: 0x000100fc
    entry type is: 0x0x3
    entry base is: 0x00000000 : 0xbfff0000
    entry length is: 0x00000000 : 0x00010000
    entry size is: 0x00000014
  MMap Entry count is: 0x2
        32-bit entry 0x0 contains 0x00000014
        32-bit entry 0x1 contains 0xbfff0000
        32-bit entry 0x2 contains 0x00000000
        32-bit entry 0x3 contains 0x00010000
        32-bit entry 0x4 contains 0x00000000
        32-bit entry 0x5 contains 0x00000003
```

I need to confirm what `type 3` is, but that appears to coincide with the ACPI memory map.  If this is true, then it also means that this memory has not been allocated, and I should be able to map it.

So, this is in bochs, I want to confirm the same on qemu.  For qemu, this memory is of type 2, or unavailable.  So, this means I might actually have to try this on real hardware.

---

Well, the kernel boots on real hardware.  However, the thing I did not take into account was that I need a serial cable to see the output.  The monitor is irrelevant since I do not write to it.  This will end up being a trip to the small electronics store to get what I need.

So, what I need to do is figure out how I am going to handle this work.  Do I make this happen earlier in the kernel initialization (where I perform a function call before I actually jump into the loader code), prior to enabling the MMU?  Or do I find a place to map the acpi data into the virtual memory space and perform address translation on the tables?

The problem I am struggling with is that the location will be different on different systems (not even architectures or platforms -- this is at the individual system level.  There may even be a risk that the location could be different boot-to-boot (though unlikely).  Therefore, the only thing I can really do is to map the acpi tables into memory and work them from there, translating the addresses as I need to.

Now, on the other hand, the memory for both qemu and bochs for the actual acpi tables location is just below `0xc0000000`.  This memory is not used at the moment, so I do have a good chance to be able to identity map this address space.

---

So, I am wondering if I should not update the Page Fault handler to automatically map the ACPI space.  I would have to narrow this space a little bit, but it would make for a slick dynamic memory management.  The problem I have with that is that I am not really sure at this point where the ACPI tables are located on real hardware.  I may have to actually go after the screen for this.

---

### 2019-Apr-07

Well, I have been able to find the APIC local IC address (each processor has its own address of this structure).  However, the address is in my stacks area.  I will have to manage that carefully -- and may even shrink that memory allocation.

However, for some reason my x86 code is now locking up.  Not sure why -- it is after we are swapping processes, so I assume it is related to cache management.  I will come back to that in a bit -- for the moment, I want to use that to help me keep the data I want to review on the screen.

---

### 2019-Apr-08

Well, it looks like I am executing from the heap:

```
Page Fault
EAX: 0x003fe000  EBX: 0x8007a160  ECX: 0x003fdfff
EDX: 0x9000bab4  ESI: 0x00000000  EDI: 0x8007a010
EBP: 0x80002d1b  ESP: 0xff805eb0  SS: 0x10
EIP: 0x90000163  EFLAGS: 0x00200086
CS: 0x8  DS: 0x10  ES: 0x10  FS: 0x10  GS: 0x10
CR0: 0xe0000011  CR2: 0x013f603f  CR3: 0x003fe000
Trap: 0xe  Error: 0x0
```

I'm not sure where that is coming from....  But, again, for the moment, I am going to take advantage of it.

---

### 2019-Apr-09

I finally have all the structures presented to me parsed.  Now, what to do with the results?

Well, looking back to my Century32 code, which had a APIC timer implementation, and from reading more from the Intel manuals, there are a few bits of information I need and a few more steps I need to do before I pull the trigger on this and start writing code to actually support the APIC:
* I need to determine whether I even have an APIC.  No assumptions, please.  Now, if there is no APIC, there can only be one processor.  Looking at this the other way, uni-core only needs to support the 8259 PIC.  In other words, there should be no reason I should require support for an APIC to run.
* I need the MMIO address of the APIC.  This can be modified, so I will want to set that to be something worthwhile.
* I will need to capture the number of processors (there can be up to 255 processors from what I see).

So, I want to start with finally capturing CPUID data and storing that in the hardware discovery structure, since I will need that for determining if I even have an APIC.

Now, before I can use the CPUID function, I need to determine if it is available.

---

### 2019-Apr-10

Well, it looks like I have made a mess of the loader code, where I cannot output messages to the serial port (at least in x86).  I have to get that cleaned up before I can go on -- I need to be able to debug.  Honestly, though, I am not sure how much time I will be able to put to that.

Also, an interesting fact: bochs does not crash/lock while qemu does.  I have a problem with my cache instructions (I think anyway).

---

### 2019-Apr-11

OK, my faulting problem looks like this:
* On QEMU, I triple fault.
* On Bochs, I do not.
* On my test laptop, I triple fault.

I still believe that this is cache related somehow.  If I recall correctly, Bochs does not implement cache and the instruction operate as `nop`s.  Now, on the other hand, I am not sure about QEMU.  To make matters worse, when I comment out the line to clear cache on x86, it has no effect.  So I am not totally convinced _what_ I am dealing with.  A global disable on the x86 cache results in an Invalid Opcode Exception -- a jump to NULL.  AND, when I am creating a debugging log through QEMU it triple faults but when I just run it, the emulator issues the invalid opcode.

Hmmm...  do I have a race condition?  I am not seeing processes B, D, or F getting any CPU.  This means I might have to check the Ready Queue implementation....

---

```
[18:10] <eryjus> I have something really odd going on...  qemu (without the debug console) jumps to 0x0000 and the faults on an invalid opcode; qemu with debugging triple faults on a bad address.  Bochs runs clean with no issues.  Real hardware triple faults but since I am only outputting to the serial port (and I do not have a proper cable made yet), so I cannot concretely determine if it is really the same bahavior.
[18:10] <eryjus> so, my question is: does anyone know what differences there might be between what qemu and bochs emulate that might contribute to this behavior?
[18:13] <Mutabah> Reading/running junk code?
[18:14] <doug16k> eryjus, you can hack in stores to 0xb8000 to set the letter in the top left corner at certain places
[18:14] <doug16k> 65 == 'A'
[18:15] <doug16k> i.e. ASCII
[18:15] <doug16k> make those stores stall really really long on real machine so a human can catch it
[18:15] <eryjus> Well I'm certain I am.  I hoping to get that narrowed down a bit because I suck at single stepping code -- I'm far more effective with an execution log.  I'll do it when I have to, but I suck at it (measured in days versus hours)
[18:15] <doug16k> count from 8 billion to 0 in a loop or something
[18:16] <doug16k> that's how I got over "poof rebooted instantly" at first
[18:16] <eryjus> doug16k, qemu set up graphics for me, so it will be a little harder than that..  But I will look into the suggestions
[18:16] <doug16k> no I meant on real machine
[18:17] <doug16k> same thing will work with no stall on qemu since it will stop
[18:17] <doug16k> on qemu you can debug. how can you not know where it happens?
[18:18] <doug16k> if you didn't attach gdb yet, you should add -s to the qemu command line, then in a terminal do this: gdb -ex 'target remote localhost:1234' -ex 'layout asm' -ex 'layout reg'
[18:19] <doug16k> then you can look at where it is and perhaps see caller on stack by doing x /16gx $rsp  (wx and $esp for 32 bit code)
[18:20] <doug16k> if you have an executable with debug info, then put that before the first -ex and then even bt will give you a backtrace
[18:20] <doug16k> then layout src and even see source level display, then frame N where N >= 0 to go up to caller frames
[18:21] <eryjus> crap -- re-reading my own posts..  grub is setting up graphics, not qemu...  but let me try the gdb route..  I have not seen that set of parameters before
[18:21] <doug16k> those ex flags just mean "run this command right away after init"
[18:22] <doug16k> one connects to remote gdb that is listening in qemu due to -s, rest set up text UI to see code and regs
[18:22] <doug16k> remote gdbserver*
[18:22] <eryjus> it's the layouts that are new to me...  I'm gonna give it a try.  thanks again!
[18:23] <doug16k> they remove the tedium of constantly running commands to see where you are now
[18:23] <doug16k> or see what the register values are
[18:24] <doug16k> src will show your source code in layout src, layout asm will show raw disassembly and works with no symbols whatsoever
[18:26] <doug16k> if you want to make it stop before it dies, like at an entry point, then use -S in qemu and it will wait for a debugger to tell it to run
[18:27] <doug16k> then you can attach, place breakpoints, and run, before it even executes one instruction in the firmware
[18:35] <doug16k> eryjus, if you want a near-zero-effort way to emit debug strings, then add to qemu: -chardev stdio,id=dbgwhatever -device isa-debugcon,chardev=dbgwhatever
[18:36] <doug16k> then to print a string, load 0xe9 into dx, load the string pointer into rsi/esi, load the string length into rcx/ecx and execute cld rep outsb
[18:37] <doug16k> it's ludicrous speed in tcg, and pretty good in kvm
[18:38] <doug16k> so fast you could debug races if you redirect output, since it hardly interferes
[18:40] <doug16k> ludicrous speed is really fast, in case you don't get the spaceballs reference :P
[18:41] <doug16k> way faster than warp speed. warp speed if for chumps
[18:43] <doug16k> it's literally emitting an entire string as one instruction
[18:45] <doug16k> even under hardware virt, since it will see the rep when it traps and do the whole thing in one vmexit
[18:46] <doug16k> and under tcg those registers become actually a write to actual stdio, after virt-to-phys translation
[18:47] <doug16k> writev to be precise
[19:06] <doug16k> as if writev. I think it actually unrolls the rep outs to a loop of outb of 1 byte to the device io port handler in that particular case. other devices can do better though
[19:07] <doug16k> as one instruction as far as tcg sees it though
[19:07] <doug16k> or kvm for that matter
[19:31] <jjuran> doug16k: What!?  "Warp" is Star Trek; SpaceBalls is a Star Wars spoof!  Surely you mean "light speed".
[19:52] <doug16k> yes, I mean light speed, oops
[19:56] <jjuran> You were supposed to say "Don't call me Shirley."  Hand in your card, please. :-P
```

---

### 2019-Apr-12

Making use of Doug16k's suggestions, it looks like I might be burying my CPU in IRQs again....  and not getting anything done.  It looks like I have recursive calls to the IRQ Handler routine before I ever even get off the first instruction.

Let's see if I can map the stack:

|    Addr    |   Value    |  What is it?            |
|:----------:|:----------:|:------------------------|
| 0xff804ffc | 0x00200246 | Ivvavrn00odItsZaPc -- flags are reasonable, but from where??? |
| 0xff804ff8 | 0x80003350 | StartE() |
| 0xff804ff4 | 0xff804ff8 | esp |
| 0xff804ff0 | 0x80003350 | StartE()  |
| 0xff804fec | 0x80003350 | STartE()  |
| 0xff804fe8 | 0x003fe000 | cr3?  |
| 0xff804fe4 | 0x00000007 | eflags -- start of interrupt |
| 0xff804fe0 | 0x00000008 | cs |
| 0xff804fdc | 0x0000004c | eip -- jump to NULL?!?! |
| 0xff804fd8 | 0x00000000 | error code  |
| 0xff804fd4 | 0x00000000 | ackIRQ  |
| 0xff804fd0 | 0x00000006 | intno -- Invalid OpCode  |
| 0xff804fcc | 0x003fe012 | eax  |
| 0xff804fc8 | 0x00000001 | ecx  |
| 0xff804fc4 | 0x927e71ca | edx  |
| 0xff804fc0 | 0x80003350 | ebx  |
| 0xff804fbc | 0xff804fc0 | esp  |
| 0xff804fb8 | 0x003fdfff | ebp  |
| 0xff804fb4 | 0x00000004 | esi  |
| 0xff804fb0 | 0x00000005 | edi  |
| 0xff804fac | 0x80000011 | cr0  |
| 0xff804fa8 | 0x00000000 | cr2  |
| 0xff804fa4 | 0x003fe000 | cr3  |
| 0xff804fa0 | 0x00000010 | ds  |
| 0xff804f9c | 0x00000010 | es  |
| 0xff804f98 | 0x00000010 | fs  |
| 0xff804f94 | 0x00000010 | gs  |
| 0xff804f90 | 0x00000010 | ss  |
| 0xff804f8c | 0x80001710 | IsrCommonStub() [return point from call to IsrHandler()] |
| 0xff804f88 | 0x800800b8 | var 0x04 ???scheduler.schedulerLock.locked -- what the hell?? |
| 0xff804f84 | 0x800800c8 | var 0x08 ???scheduler.queueOS.lock.locked |
| 0xff804f80 | 0x800029ed | var 0x0c ???ProcessListRemove() [return point from call to SpinlockLock()]  |
| 0xff804f7c | 0x900001b4 | 2nd var 0x04 ???Heap process_t structure  |
| 0xff804f78 | 0x800800b8 | 2nd var 0x08 ???scheduler structure member  |
| 0xff804f74 | 0x90000198 | 2nd var 0x0c ???Heap process_t structure  |
| 0xff804f70 | 0xff804f90 | IsrRegs_t structure pointer  |
| 0xff804f6c | 0x8000180c | IsrHandler(), after a call to eax register -- this would be a far jump to handler from table  |
| 0xff804f68 | 0x80003350 | ebx -- from IsrDumpState() -- value of StartE() |
| 0xff804f64 | 0x800800c8 | ??? |
| 0xff804f60 | 0x8000307b | ??? |
| 0xff804f5c | 0x003fe000 | ??? |
| 0xff804f58 | 0x00000000 | ecx -- arg for kprintf() |
| 0xff804f54 | 0x00000006 | ebx -- arg for kprintf() |
| 0xff804f50 | 0x80005ced | push 0x80005cd8 -- kprintf() fmt string |
| 0xff804f4c | 0x800017e4 | IsrDumpState() [return point after call to Halt()]  |

---

I will write commentary on this tomorrow.  In the meantime I think one place to look is the spinlock (which might be artificially enabling interrupts).

---

### 2019-Apr-13

Since we are throwing a birthday party today, I'm not going to have too much time for the OS today.  But a few more things come to mind to investigate:
* Check the stack to mis-alignment
* Is this working still for rpi2b?  If so, then it must be arch-specific
* Check when interrupts are being enabled against when the interrupt is firing
* And, of course from yesterday, the spinlock/unlock code again

Another question is whether I can get it to duplicate again when I am starting the E process.

---

The rpi2b test has been running for about 5 minutes on real hardware.  However, I do notice that occasionally I have a task that sleeps out of order -- meaning it was probably preempted before it could get the sleep set up properly and then it gets control for sleep again.  Or maybe that is the timer.  Not sure, but it appears on the surface to be an anomaly and something to look at.

Well, it looks like that is perfectly normal -- the text is in process A and it is being preempted properly by G which is preempted by A again.

However, thinking about this, I probably have some race condition in x86 that is causing this.  It would explain the different behaviors in the emulator configs...
