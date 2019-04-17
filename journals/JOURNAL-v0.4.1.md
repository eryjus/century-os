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

---

### 2019-Apr-14

Out of desperation, I did check out v0.4.0 and compile and run it.  This version still works.  The current version of v0.4.1 does not.  So, it is safe to say that the things I changed between the 2 somehow either broke code or exposed broken code.

So, what changed between the 2 versions?
1. I increased memory for the qemu emulator (this is a possible cause)
1. I added a command to write to the USB stick (not likely)
1. I created a `PlatformEarlyInit()` function and called it (this is a possible cause)
1. I added a function to check if `CPUID` is supported (low likelihood, but cannot be eliminated)
1. I added a function to collect the `CPUID` information (low likelihood, but cannot be eliminated)
1. I also extended the `localHwDisc` structure which may be bigger than one frame and my compile time check is failing (near 0 likelihood, but will check to confirm)
1. I disabled debugging on `MmuEarlyInit()` (near 0 likelihood but will check to confirm)
1. I added conditional compile tags to `MmuEarlyInit()` (near 0 likelihood but will check to confirm)
1. I added conditional compile tags to `MmuInit()` (near 0 likelihood but will check to confirm)
1. I moved the location of `#define WBINVD()` to be conditioned (not likely since the change was done to debug this code)
1. I changed several function prototypes from `extern "C"` to `__CENTURY_FUNC__` (not likely since they are the same)
1. I added a `FrameBufferPutHex()` macro and called it (not likely, but will check)
1. I added several ACPI determination functions (not likely but will check)

Let me go after the 2 most probable causes.  First the memory change.  And....!!!!  it turned out to be the memory change that created the problem!!

WTF!!!!

Ok, bochs was set to 3072 MB memory.  So I changed it to 3584 MB, and it still works.

So, this makes the possible candidates the PMM and the Memory Map.  I am unable to check MB2 since that does not return a memory map for some reason.  OK, but MB2 does have the code to determine the memory map and that should be working.  So, let me start with that.

---

Well, the MB2 structure is located at `0x00103048`.  I believe that this is right after the kernel.  On the other hand, the MB1 structure is located at `0x00010000`, putting it in low memory.

Both multiboot specifications state:

> The Multiboot information structure and its related substructures may be placed anywhere in memory by the boot loader (with the exception of the memory reserved for the kernel and boot modules, of course). It is the operating system’s responsibility to avoid overwriting this memory until it is done using it.

I may be having such a problem with MB2 since it is hanging off the end of the kernel.  Before moving everything into the kernel, this all worked, so that may have something to do with it....

---

### 2019-Apr-15

After removing the intermediate alignment statements I get the following code:

```
00100030 <MultibootHeader2>:
  100030:       d6                      (bad)
  100031:       50                      push   %eax
  100032:       52                      push   %edx
  100033:       e8 00 00 00 00          call   100038 <MultibootHeader2+0x8>
  100038:       40                      inc    %eax
  100039:       00 00                   add    %al,(%eax)
  10003b:       00 ea                   add    %ch,%dl
  10003d:       ae                      scas   %es:(%edi),%al
  10003e:       ad                      lods   %ds:(%esi),%eax
  10003f:       17                      pop    %ss

00100040 <Type4Start>:
  100040:       04 00                   add    $0x0,%al
  100042:       01 00                   add    %eax,(%eax)
  100044:       0c 00                   or     $0x0,%al
  100046:       00 00                   add    %al,(%eax)
  100048:       00 00                   add    %al,(%eax)
        ...

0010004c <Type5Start>:
  10004c:       05 00 01 00 14          add    $0x14000100,%eax
  100051:       00 00                   add    %al,(%eax)
  100053:       00 00                   add    %al,(%eax)
  100055:       04 00                   add    $0x0,%al
  100057:       00 00                   add    %al,(%eax)
  100059:       03 00                   add    (%eax),%eax
  10005b:       00 10                   add    %dl,(%eax)
  10005d:       00 00                   add    %al,(%eax)
        ...

00100060 <Type6Start>:
  100060:       06                      push   %es
  100061:       00 01                   add    %al,(%ecx)
  100063:       00 08                   add    %cl,(%eax)
  100065:       00 00                   add    %al,(%eax)
        ...

00100068 <Type6End>:
  100068:       00 00                   add    %al,(%eax)
  10006a:       00 00                   add    %al,(%eax)
  10006c:       08 00                   or     %al,(%eax)
        ...
```
 Putting the alignments back in:

 ```
 00100030 <MultibootHeader2>:
  100030:       d6                      (bad)
  100031:       50                      push   %eax
  100032:       52                      push   %edx
  100033:       e8 00 00 00 00          call   100038 <MultibootHeader2+0x8>
  100038:       48                      dec    %eax
  100039:       00 00                   add    %al,(%eax)
  10003b:       00 e2                   add    %ah,%dl
  10003d:       ae                      scas   %es:(%edi),%al
  10003e:       ad                      lods   %ds:(%esi),%eax
  10003f:       17                      pop    %ss

00100040 <Type4Start>:
  100040:       04 00                   add    $0x0,%al
  100042:       01 00                   add    %eax,(%eax)
  100044:       0c 00                   or     $0x0,%al
  100046:       00 00                   add    %al,(%eax)
  100048:       00 00                   add    %al,(%eax)
        ...

0010004c <Type4End>:
  10004c:       90                      nop
  10004d:       90                      nop
  10004e:       90                      nop
  10004f:       90                      nop

00100050 <Type5Start>:
  100050:       05 00 01 00 14          add    $0x14000100,%eax
  100055:       00 00                   add    %al,(%eax)
  100057:       00 00                   add    %al,(%eax)
  100059:       04 00                   add    $0x0,%al
  10005b:       00 00                   add    %al,(%eax)
  10005d:       03 00                   add    (%eax),%eax
  10005f:       00 10                   add    %dl,(%eax)
  100061:       00 00                   add    %al,(%eax)
        ...

00100064 <Type5End>:
  100064:       90                      nop
  100065:       90                      nop
  100066:       90                      nop
  100067:       90                      nop

00100068 <Type6Start>:
  100068:       06                      push   %es
  100069:       00 01                   add    %al,(%ecx)
  10006b:       00 08                   add    %cl,(%eax)
  10006d:       00 00                   add    %al,(%eax)
        ...
 ```

Notice the nop. between the End and Start tags.

Now, the former will not boot.  It cannot identify the tags properly and finds something incorrect.  The latter on the other hand does boot but I do not get any boot information (including memory map).

---

OK, I have my MB2 issues figured out.  The short answer is that my _localHwDisc structure is overlaying my MB2 MBI structure.  Grub2 is putting this structure at `0x103048` and my `_localHwDisc` structure is at `0x103000`, but is approaching a page in size.  I am ultimately overwriting the MBI with the `_localHwDisc` data.

I am going to articulate why this is happening, but I need to take this on tomorrow.

---

### 2019-Apr-16

Today I need to start by properly documenting what I found last night with the MB2 MBI table being overwritten (and then once that is done, fix it!).

First, the output from the kernel indicates the location of the MBI block:

```
Parsing MB2 Info at 0x00103048 (MB1 info at 0x00000000)
.. size = 0x0
.. resv = 0x0
```

However, notice that the size is `0`, which is not what I would expect.  To confirm, I went to an execution log:

```
----------------
IN:
0x001022c3:  3d 89 62 d7 36           cmpl     $0x36d76289, %eax
0x001022c8:  75 06                    jne      0x1022d0

EAX=36d76289 EBX=00103048 ECX=00000000 EDX=00000000
ESI=00000000 EDI=00000000 EBP=00000000 ESP=0007ff00
EIP=001022c3 EFL=00200006 [-----P-] CPL=0 II=0 A20=1 SMM=0 HLT=0
ES =0018 00000000 ffffffff 00cf9300 DPL=0 DS   [-WA]
CS =0010 00000000 ffffffff 00cf9a00 DPL=0 CS32 [-R-]
SS =0018 00000000 ffffffff 00cf9300 DPL=0 DS   [-WA]
DS =0018 00000000 ffffffff 00cf9300 DPL=0 DS   [-WA]
FS =0018 00000000 ffffffff 00cf9300 DPL=0 DS   [-WA]
GS =0018 00000000 ffffffff 00cf9300 DPL=0 DS   [-WA]
LDT=0000 00000000 0000ffff 00008200 DPL=0 LDT
TR =0000 00000000 0000ffff 00008b00 DPL=0 TSS32-busy
GDT=     000010b0 00000020
IDT=     00000000 00000000
CR0=00000011 CR2=00000000 CR3=00000000 CR4=00000000
DR0=00000000 DR1=00000000 DR2=00000000 DR3=00000000
DR6=ffff0ff0 DR7=00000400
CCS=2badb002 CCD=0b29b287 CCO=SUBL
EFER=0000000000000000
```

That address is stored in `ebx`.  Looking now at the MB2 specification, I note that this address is indeed supposed to be in `ebx`:

> #### 3.6.1 Boot information format
> Upon entry to the operating system, the EBX register contains the physical address of a Multiboot2 information data structure, through which the boot loader communicates vital information to the operating system. The operating system can use or ignore any parts of the structure as it chooses; all information passed by the boot loader is advisory only.
> The Multiboot2 information structure and its related substructures may be placed anywhere in memory by the boot loader (with the exception of the memory reserved for the kernel and boot modules, of course). It is the operating system’s responsibility to avoid overwriting this memory until it is done using it.

So, indeed, `ebx` contains the value I am looking for.

Looking at a disassembly of my binary, I see the following structure ahead of this location:

```
00103000 <_localHwDisc>:
        ...
```

This means that the local structure for `_localHwDisc` is located at the frame address just ahead of the MBI structure.  This is not supposed to overlap.  Currently the structure is defined as:

```C
//
// -- This structure will hold a stage-3 loader local copy of the provided multiboot information.
//    -------------------------------------------------------------------------------------------
typedef struct HardwareDiscovery_t {
    //
    // -- The BIOS information
    //    --------------------
    archsize_t ebdaLocation;
    uint16_t com1;
    uint16_t com2;
    uint16_t com3;
    uint16_t com4;
    uint16_t lpt1;
    uint16_t lpt2;
    uint16_t lpt3;
    uint16_t videoPort;

    //
    // -- the memory limit information
    //    ----------------------------
    bool memLimitsAvail;
    uint32_t availLowerMem;
    uint32_t availUpperMem;
    uint64_t upperMemLimit;

    //
    // -- the memory map information
    //    --------------------------
    bool memMapAvail;
    int memMapCount;
    MMap_t mmap[NUM_MMAP_ENTRIES];

    //
    // -- the module information
    //    ----------------------
    bool modAvail;
    int modCount;
    Module_t mods[MAX_MODULES];
    frame_t modHighestFrame;

    //
    // -- the Physical Memory Manager location and other relevant info
    //    ------------------------------------------------------------
    uint32_t *pmmBitmap;
    size_t pmmFrameCount;

    //
    // -- FrameBufferInformation
    //    ----------------------
    // -- Frame Buffer Info
    bool frameBufferAvail;
    uint16_t *fbAddr;
    uint32_t fbPitch;
    uint32_t fbWidth;
    uint32_t fbHeight;
    uint8_t fbBpp;
    FrameBufferType fbType;

    //---------------------------------

    //
    // -- The console properties; which will be also passed to the kernel
    //    ---------------------------------------------------------------
    uint16_t bgColor;
    uint16_t fgColor;
    uint16_t rowPos;
    uint16_t colPos;


    //
    // -- The location of the ACPI tables
    //    -------------------------------
    archsize_t rsdp;


    //
    // -- CPUID Data
    //    ----------
    bool cpuidSupported;

    uint32_t cpuid00eax;
    uint32_t cpuid00ebx;
    uint32_t cpuid00ecx;
    uint32_t cpuid00edx;

    uint32_t cpuid01eax;
    uint32_t cpuid01ebx;
    uint32_t cpuid01ecx;
    uint32_t cpuid01edx;

    uint32_t cpuid02eax;
    uint32_t cpuid02ebx;
    uint32_t cpuid02ecx;
    uint32_t cpuid02edx;

    uint32_t cpuid03eax;
    uint32_t cpuid03ebx;
    uint32_t cpuid03ecx;
    uint32_t cpuid03edx;

    uint32_t cpuid04eax;
    uint32_t cpuid04ebx;
    uint32_t cpuid04ecx;
    uint32_t cpuid04edx;

    uint32_t cpuid05eax;
    uint32_t cpuid05ebx;
    uint32_t cpuid05ecx;
    uint32_t cpuid05edx;

    uint32_t cpuid06eax;
    uint32_t cpuid06ebx;
    uint32_t cpuid06ecx;
    uint32_t cpuid06edx;

    uint32_t cpuid07eax;
    uint32_t cpuid07ebx;
    uint32_t cpuid07ecx;
    uint32_t cpuid07edx;

    uint32_t cpuid09eax;
    uint32_t cpuid09ebx;
    uint32_t cpuid09ecx;
    uint32_t cpuid09edx;

    uint32_t cpuid0aeax;
    uint32_t cpuid0aebx;
    uint32_t cpuid0aecx;
    uint32_t cpuid0aedx;

    uint32_t cpuid0beax;
    uint32_t cpuid0bebx;
    uint32_t cpuid0becx;
    uint32_t cpuid0bedx;
} HardwareDiscovery_t;
```

... this is easily more than `0x48` bytes long, even without counting bytes.  Therefore, the data elements are overlapping.  This should not be the case.

Specifically, the linker script is written to address this (or, so I thought):

```
    .loader : {       /* This is where the loader will be located -- things to be reclaimed: */
        *(.mboot)                       /* -- multiboot header comes first */
        *(.ldrtext)                     /* -- loader text (code) */
        *(.text.startup)                /* -- this is additional initialization code */
        . = ALIGN(8);                   /* -- the next table needs to align to 8 bytes */

        init_start = .;                 /* -- here we have an array of functions that need to be called */
        *(.ctors)
        *(.ctors.*)
        *(.init_array)
        *(SORT_BY_INIT_PRIORITY(.init_array.*))
        init_end = .;

        . = ALIGN(8);                   /* -- re-align to 8 bytes */
        *(.ldrrodata)                   /* -- loader rodata (like strings) */
        *(.ldrdata)                     /* -- loader data (things that will not get passed to the kernel */

        phys_loc = .;                   /* -- provide variable `phys_loc` with the value of PHYS */
        LONG(PHYS);

        kern_loc = .;                   /* -- provide variable `kern_loc` wiht the value of KERN */
        LONG(KERN);

        *(.ldrbss)                      /* -- loader bss (again, things that will not get passed to the kernel */
        . = ALIGN(4096);

        _localHwDisc = .;               /* -- this will be the location of the hardware discovery structire */
        BYTE(0);                        /* -- be sure something is allocated */
    }
    . = ALIGN(4096);
    _loaderEnd = .;
    PHYS_OFFSET = .;
```

Well, I am reserving a single byte and then aligning the rest of the loader section to the frame (or so I thought).

However, looking at the sections using `readelf -a targets/x86-pc/bin/boot/kernel.elf`, I get the following program headers:

```
Program Headers:
  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align
  LOAD           0x001000 0x00100000 0x00100000 0x03001 0x03001 RWE 0x1000
  LOAD           0x005000 0x80000000 0x00104000 0x06c1c 0x06c1c R E 0x1000
  LOAD           0x00c000 0x80007000 0x0010b000 0x791c8 0x7e68c RW  0x1000
```

In particular the first LOAD header is loaded at address `0x00100000` and has a memory length of `0x03001` bytes.  This, then, puts the ending location of the `_loaderHwDisc` structure at `0x00103001` from the linker script but `sizeof(_loaderHwDisc)` in the C code (which again, while I have not counted it, is clearly more than `0x48` bytes long).

So, then, GRUB2 was able to find a little wedge of memory between the (incorrect) end of the `_localHwDisc` structure and the start of the kernel code page targeted for upper memory and place the data for the MBI in that space.

Finally, to seal my fate here, the structure is located in what is effectively an uninitialized loader bss section.  As a result I am very diligent about initializing every byte of this memory for fear it is not pre-cleared for me when I first access it:

```C
__ldrdata HardwareDiscovery_t *localHwDisc = (HardwareDiscovery_t *)_localHwDisc;

//
// -- Perform the hardware discovery
//    ------------------------------
void __ldrtext HwDiscovery(void)
{
    lMemSetB(localHwDisc, 0, sizeof(HardwareDiscovery_t));
    Mb1Parse();
    Mb2Parse();
//    ArchDiscovery();
}
```

This function clears the `_localHwDisc` structure, and the MBI structure at the same time.

---

Now, I can get back to my problem with memory size....  In particular, when I have 3072MB of memory, the system works properly; when I have 3584MB of memory the system crashes.

So, one of the questions I have is about where the bounds are on this.
* 4096 crashes
* 4000 simply locks up
* 3070 runs
* 3072 runs
* 3073 runs
* 3075 runs
* 3080 runs
* 3090 runs
* 3095 runs
* 3200 runs
* 3300 runs
* 3400 runs
* 3500 runs
* 3550 runs
* 3580 runs
* 3583 runs
* 3584 indeed crashes

And with this last crash, I have some interesting results:

```
Found the mbi structure at 0x00010000
  The flags are: 0x00001a67
Setting basic memory information
Identifying command line information:
Setting memory map data
  iterating in mmap - size is: 0x000000a8
    entry address is: 0x0001009c
    entry type is: 0x0x1
    entry base is: 0x00000000 : 0x00000000
    entry length is: 0x00000000 : 0x0009fc00
    entry size is: 0x00000014
  MMap Entry count is: 0x0
        32-bit entry 0x0 contains 0x00000014
        32-bit entry 0x1 contains 0x00000000
        32-bit entry 0x2 contains 0x00000000
        32-bit entry 0x3 contains 0x0009fc00
        32-bit entry 0x4 contains 0x00000000
        32-bit entry 0x5 contains 0x00000001
  Through all entries...
  iterating in mmap - size is: 0x00000090
    entry address is: 0x000100b4
    entry type is: 0x0x2
    entry base is: 0x00000000 : 0x0009fc00
    entry length is: 0x00000000 : 0x00000400
    entry size is: 0x00000014
  MMap Entry count is: 0x1
        32-bit entry 0x0 contains 0x00000014
        32-bit entry 0x1 contains 0x0009fc00
        32-bit entry 0x2 contains 0x00000000
        32-bit entry 0x3 contains 0x00000400
        32-bit entry 0x4 contains 0x00000000
        32-bit entry 0x5 contains 0x00000002
  Through all entries...
  iterating in mmap - size is: 0x00000078
    entry address is: 0x000100cc
    entry type is: 0x0x2
    entry base is: 0x00000000 : 0x000f0000
    entry length is: 0x00000000 : 0x00010000
    entry size is: 0x00000014
  MMap Entry count is: 0x1
        32-bit entry 0x0 contains 0x00000014
        32-bit entry 0x1 contains 0x000f0000
        32-bit entry 0x2 contains 0x00000000
        32-bit entry 0x3 contains 0x00010000
        32-bit entry 0x4 contains 0x00000000
        32-bit entry 0x5 contains 0x00000002
  Through all entries...
  iterating in mmap - size is: 0x00000060
    entry address is: 0x000100e4
    entry type is: 0x0x1
    entry base is: 0x00000000 : 0x00100000
    entry length is: 0x00000000 : 0xbfee0000
    entry size is: 0x00000014
  MMap Entry count is: 0x1
        32-bit entry 0x0 contains 0x00000014
        32-bit entry 0x1 contains 0x00100000
        32-bit entry 0x2 contains 0x00000000
        32-bit entry 0x3 contains 0xbfee0000
        32-bit entry 0x4 contains 0x00000000
        32-bit entry 0x5 contains 0x00000001
  Through all entries...
  iterating in mmap - size is: 0x00000048
    entry address is: 0x000100fc
    entry type is: 0x0x2
    entry base is: 0x00000000 : 0xbffe0000
    entry length is: 0x00000000 : 0x00020000
    entry size is: 0x00000014
  MMap Entry count is: 0x2
        32-bit entry 0x0 contains 0x00000014
        32-bit entry 0x1 contains 0xbffe0000
        32-bit entry 0x2 contains 0x00000000
        32-bit entry 0x3 contains 0x00020000
        32-bit entry 0x4 contains 0x00000000
        32-bit entry 0x5 contains 0x00000002
  Through all entries...
  iterating in mmap - size is: 0x00000030
    entry address is: 0x00010114
    entry type is: 0x0x2
    entry base is: 0x00000000 : 0xfffc0000
    entry length is: 0x00000000 : 0x00040000
    entry size is: 0x00000014
  MMap Entry count is: 0x2
        32-bit entry 0x0 contains 0x00000014
        32-bit entry 0x1 contains 0xfffc0000
        32-bit entry 0x2 contains 0x00000000
        32-bit entry 0x3 contains 0x00040000
        32-bit entry 0x4 contains 0x00000000
        32-bit entry 0x5 contains 0x00000002
  Through all entries...
  iterating in mmap - size is: 0x00000018
    entry address is: 0x0001012c
    entry type is: 0x0x1
    entry base is: 0x00000001 : 0x00000000
    entry length is: 0x00000000 : 0x20000000
    entry size is: 0x00000014
  MMap Entry count is: 0x2
        32-bit entry 0x0 contains 0x00000014
        32-bit entry 0x1 contains 0x00000000
        32-bit entry 0x2 contains 0x00000001
        32-bit entry 0x3 contains 0x20000000
        32-bit entry 0x4 contains 0x00000000
        32-bit entry 0x5 contains 0x00000001
  Through all entries...
Memory Map is complete
```

I need to clean this up a bit, but the last memory location is outside 32-bit address space.

I am going to clean up the logs first and then do some additional testing.

---

So, for 3584MB of memory, the usable mmap looks like this:

```
Setting memory map data
  iterating in mmap
    entry address is: 0x0001009c
    entry type is: 0x1
    entry base is: 0x00000000 : 0x00000000
    entry length is: 0x00000000 : 0x0009fc00
    entry size is: 0x00000014
  iterating in mmap
    entry address is: 0x000100e4
    entry type is: 0x1
    entry base is: 0x00000000 : 0x00100000
    entry length is: 0x00000000 : 0xbfee0000
    entry size is: 0x00000014
  iterating in mmap
    entry address is: 0x0001012c
    entry type is: 0x1
    entry base is: 0x00000001 : 0x00000000
    entry length is: 0x00000000 : 0x20000000
    entry size is: 0x00000014
Memory Map is complete
```

This equates to about 640KB, <3GB, and then 512MB (which is above 32-bit address space).

For 3583MB, I get the following map:

```
Setting memory map data
  iterating in mmap
    entry address is: 0x0001009c
    entry type is: 0x1
    entry base is: 0x00000000 : 0x00000000
    entry length is: 0x00000000 : 0x0009fc00
    entry size is: 0x00000014
  iterating in mmap
    entry address is: 0x000100e4
    entry type is: 0x1
    entry base is: 0x00000000 : 0x00100000
    entry length is: 0x00000000 : 0xdfde0000
    entry size is: 0x00000014
Memory Map is complete
```

... and notice that first of all the memory is in 32-bit address space.

Finally, the last thing to consider here is that I am artificially converting the value to 32-bits:

```C
    for (int i = 0; i < GetMMapEntryCount(); i ++) {
        frame_t frame = ((archsize_t)GetAvailMemStart(i)) >> 12;
        size_t count = ((archsize_t)GetAvailMemLength(i)) >> 12;
```

This means that my starting address of `0x0000 0001 0000 0000` is actually converted to `0x00000000` and that is wrong.

In fact, I cannot use any physical memory that is outside this 32-bit size.  I am going to have to test for and discard anything that cannot be used.

And that works.  Good god!  I am going to commit these changes.

---






