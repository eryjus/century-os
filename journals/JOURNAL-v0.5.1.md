# The Century OS Development Journal - v0.5.1

This version continues the cleanup effort for the kernel.  No new features will be included in this version.

Several Redmine issues have been addressed, but several remain.  This version intends to wrap up the rest of the Redmine issues.  I may or may not actually include the code formatting cleanup in this version.  If I do not, there will be a v0.5.2 version.

## Version 0.5.1a

At this point I am down to 16 open issues in Redmine to address plus the source formatting cleanup.  The purpose of this version is to address the open Redmine issues.

### 2020-Mar-06

There are 5 bugs and 11 TODOs to address.  The top of the list will be bugs.  And the top of that list will be [#458](http://eryjus.ddns.net:3000/issues/458).

The first order of business is to be able to reliably duplicate this.  Adding a `ProcessMilliSleep(500)` call into the wait for confirmation for the IPI ack seems to duplicate this about 4 tests out of 5.  Also, it appears in rpi2b as well.

For rpi2b:

```
Data Exception:
.. Data Fault Address: 0x00000030
.. Data Fault Status Register: 0x00000807
.. Fault status 0x7: Translation fault (Second level)
.. Fault occurred because of a write
At address: 0xff800f40
 R0: 0x00000006   R1: 0x00000038   R2: 0x81000508
 R3: 0x00000000   R4: 0x81000498   R5: 0x810001ec
 R6: 0x00142c10   R7: 0x00000000   R8: 0x000c8af0
 R9: 0x00000000  R10: 0x81000604  R11: 0x810004b4
R12: 0x81000508   SP: 0x80802f74   LR_ret: 0x80802fa0
SPSR_ret: 0xa00001d3     type: 0x17

Additional Data Points:
User LR: 0xfffb56bf  User SP: 0xdde7e2d3
Svc LR: 0xff800f50
```

For x86_pc:

```
CPU: 0
EAX: 0x00000000  EBX: 0xffffffff  ECX: 0xffffffff
EDX: 0x00000000  ESI: 0x00583dd8  EDI: 0x00000000
EBP: 0x00000003  ESP: 0xff800f2c  SS: 0x10
EIP: 0x808031f4  EFLAGS: 0x00200013
CS: 0x8  DS: 0x28  ES: 0x28  FS: 0x28  GS: 0x48
CR0: 0xe0000011  CR2: 0x0000002c  CR3: 0x01001000
Trap: 0xe  Error: 0x2
```

Interestingly, the faulting address is nearly the same.

I start with the faulting address.  For x86, the following instruction:

```S
808031f4:       89 70 2c                mov    %esi,0x2c(%eax)
```

... and for rpi2b, the following:

```S
80802fa0:       e1c363f0        strd    r6, [r3, #48]   ; 0x30
```

In both cases, we are referencing a location that has not been set (`0x00000000`).  Do I have an initialization order problem?  I cannot since this is initialized at compile time.

It is this line that is creating the problem:

```C++
Enqueue(&scheduler.listSleeping, &scheduler.currentProcess->stsQueue);
```

This, then, means that somehow the `scheduler.listSleeping` elements are being set to 0.  Or, perhaps `scheduler.currentProcess`, which is far more likely.

WAIT!  Didn't I change that to be in the per-cpu structures?  I think that is the problem.

```C++
// x86
EXTERN struct Process_t *currentThread asm("%gs:4");

// rpi2b
#define currentThread ((Process_t *)ReadTPIDRURO())
```

This is where it should be looking..  `scheduler.currentProcess` needs to go away.  At the same time, I need a method to be able to update the `currentThread`.

---

### 2020-Mar-08

OK, I am still faulting.  But now I am trying to execute code:

```
Page Fault
CPU: 3
EAX: 0x01001000  EBX: 0x00000020  ECX: 0x01001000
EDX: 0x00000000  ESI: 0x00000086  EDI: 0x00000000
EBP: 0x00000086  ESP: 0xff804f7c  SS: 0x10
EIP: 0x00000001  EFLAGS: 0x00000046
CS: 0x8  DS: 0x28  ES: 0x28  FS: 0x28  GS: 0x90
CR0: 0xe0000011  CR2: 0x00000001  CR3: 0x01001000
Trap: 0xe  Error: 0x0
```

... in particular, this is on CPU 3, so there really is nothing to execute for that CPU -- everything else should be sleeping on another CPU.

Now, this may also be related to locks and lock counts.  These also need to be moved to the per-CPU structures.  This is under [Redmine #413](http://eryjus.ddns.net:3000/issues/413) and it appears I need to resolve this before I can really get to the bottom of [Redmine #458](http://eryjus.ddns.net:3000/issues/458).

---

OK, the scheduler is fubar for both archs.  It's likely something very similar for both.  I haven't really changed the logic, just the location of several variables.  Perhaps initialization?  Doubt it.  Let's start with some Bochs instrumentation.

Ok, so  this is what I get from Bochs:

```
00224155918i[      ] ----------------------------------------------------------
00224155918i[      ] CPU 3: ret
00224155918i[      ] LEN 1	BYTES: c3
00224155918i[      ] 	BRANCH TARGET 0000000000000000 (TAKEN)
00224155918i[      ] MEM ACCESS[0]: 0x00000000ff805fa0 (linear) 0x000001023fa0 (physical) RD SIZE: 4
```

I was able to identify this return to be from here in code:

```S
808036ac <ProcessSwitch.noVASchg>:
808036ac:       5d                      pop    %ebp
808036ad:       5f                      pop    %edi
808036ae:       5e                      pop    %esi
808036af:       5b                      pop    %ebx
808036b0:       c3                      ret
```

So this is from `ProcessSwitch()`.  Looking at what I have changed in that file using `git`, I see the following block:

```diff
@@ -64,10 +64,9 @@ PROC_QUANTUM_LEFT       EQU     16
 ;;
 ;; -- some local equates for accessing the structure offsets
 ;;    ------------------------------------------------------
-SCH_CURRENT_PROCESS     EQU     0
-SCH_CHG_PENDING         EQU     0x10
-SCH_LOCK_COUNT          EQU     0x18
-SCH_POSTPONE_COUNT      EQU     0x1c
+SCH_CHG_PENDING         EQU     0x0c
+SCH_LOCK_COUNT          EQU     0x14
+SCH_POSTPONE_COUNT      EQU     0x18
```

And in particular, several fields I removed from the `scheduler` structure.  `SCH_LOCK_COUNT` is the only field that remains.

I think I might also have a few other things I need to sort out:

```C++
//
// -- Set up the common CPU elements across all archs.  The actual ArchCpu_t structure will be defined
//    in the arch-cpu.h include.
//    ------------------------------------------------------------------------------------------------
#define COMMON_CPU_ELEMENTS                 \
    int cpuNum;                             \
    archsize_t stackTop;                    \
    archsize_t location;                    \
    SMP_UNSTABLE CpuState_t state;          \
    THR_UNSTABLE int kernelLocksHeld;       \
    THR_UNSTABLE bool processChangePending; \
    SMP_UNSTABLE AtomicInt_t postponeCount; \
    int disableIntDepth;                    \
    archsize_t flags;                       \
    ArchCpu_t *cpu;                         \
    INT_UNSTABLE struct Process_t *process;


//
// -- This is the abstraction of the x86 CPU
//    --------------------------------------
typedef struct ArchCpu_t {
    COMMON_CPU_ELEMENTS;
    Tss_t tss;
    archsize_t gsSelector;
    archsize_t tssSelector;
} ArchCpu_t;
```

I am expecting the address of the CPU structure to be at offset `0x00` and the address of the current process to be at offset `0x04`.  Let me confirm that....

No, the code is right.  But, that also means that the value at `[gs:0]` is really an address of the `ArchCpu_t` structure for each core.  Therefore, I really need double indirection.

Oh, that's so much better!!  Everyone gets a `#PF`!!

```
Page Fault
CPU: 2
EAX: 0x01001000  EBX: 0x810002ec  ECX: 0x01001000

Page Fault
EDX: 0x00000000  ESI: 0x900000e8  EDI: 0x90000040
CPU: 3
EBP: 0x00000000  ESP: 0xffffffd8  SS: 0x10
EAX: 0xfee00000  EBX: 0x00000003  ECX: 0x0000001d
EIP: 0x808036ae  EFLAGS: 0x00000046
EDX: 0xff803f9c  ESI: 0x00000000  EDI: 0x00000000
CS: 0x8  DS: 0x28  ES: 0x28  FS: 0x28  GS: 0x78
EBP: 0x00000000  ESP: 0xff803f10  SS: 0x10

Page Fault
EIP: 0x8080203a  EFLAGS: 0x00000086
CPU: 1
CS: 0x8  DS: 0x28  ES: 0x28  FS: 0x28  GS: 0x90
EAX: 0xfee00000  EBX: 0x00000001  ECX: 0x0000001d
CR0: 0xe0000011  CR2: 0xfee000b0  CR3: 0x01001000
EDX: 0xff801f9c  ESI: 0x00000000  EDI: 0x00000000
Trap: 0xe  Error: 0x2

EBP: 0x00000000  ESP: 0xff801f10  SS: 0x10
CR0: 0xe0000011  CR2: 0x00000000  CR3: 0x01001000
EIP: 0x8080203a  EFLAGS: 0x00000086
Trap: 0xe  Error: 0x0

CS: 0x8  DS: 0x28  ES: 0x28  FS: 0x28  GS: 0x60
CR0: 0xe0000011  CR2: 0xfee000b0  CR3: 0x01001000
Trap: 0xe  Error: 0x2
```

Let me see if I can make some sense of this:

```
Page Fault
CPU: 1
EAX: 0xfee00000  EBX: 0x00000001  ECX: 0x0000001d
EDX: 0xff801f9c  ESI: 0x00000000  EDI: 0x00000000
EBP: 0x00000000  ESP: 0xff801f10  SS: 0x10
EIP: 0x8080203a  EFLAGS: 0x00000086
CS: 0x8  DS: 0x28  ES: 0x28  FS: 0x28  GS: 0x60
CR0: 0xe0000011  CR2: 0xfee000b0  CR3: 0x01001000
Trap: 0xe  Error: 0x2

Page Fault
CPU: 2
EAX: 0x01001000  EBX: 0x810002ec  ECX: 0x01001000
EDX: 0x00000000  ESI: 0x900000e8  EDI: 0x90000040
EBP: 0x00000000  ESP: 0xffffffd8  SS: 0x10
EIP: 0x808036ae  EFLAGS: 0x00000046
CS: 0x8  DS: 0x28  ES: 0x28  FS: 0x28  GS: 0x78
CR0: 0xe0000011  CR2: 0xfee000b0  CR3: 0x01001000
Trap: 0xe  Error: 0x2

Page Fault
CPU: 3
EAX: 0xfee00000  EBX: 0x00000003  ECX: 0x0000001d
EDX: 0xff803f9c  ESI: 0x00000000  EDI: 0x00000000
EBP: 0x00000000  ESP: 0xff803f10  SS: 0x10
EIP: 0x8080203a  EFLAGS: 0x00000086
CS: 0x8  DS: 0x28  ES: 0x28  FS: 0x28  GS: 0x90
CR0: 0xe0000011  CR2: 0x00000000  CR3: 0x01001000
Trap: 0xe  Error: 0x0
```

Interesting...  CPU 3 has an error code `0x00`, not `0x02` like the other CPUs.  Also, the faulting `eip` register is not consistent.  I may have things out of order, but this will be interesting research.

I also notice that CPU2 `esp` register is a mess as well.

Actually, I need to change the format of the register dump and run this again.

Ok, this is even more interesting:

```
.(0);A(0);B(0);B(0);A(2);
Page Fault
[CPU 2]: EAX: 0x01001000  EBX: 0x01001000  ECX: 0x01001000
[CPU 2]: EDX: 0x00000000  ESI: 0x00200086  EDI: 0x00200086
[CPU 2]: EBP: 0x00000000  ESP: 0xff805fd4  SS: 0x10
[CPU 2]: EIP: 0x00000000  EFLAGS: 0x00000046
[CPU 2]: CS: 0x8  DS: 0x28  ES: 0x28  FS: 0x28  GS: 0x78
[CPU 2]: CR0: 0xe0000011  CR2: 0x00000000  CR3: 0x01001000
[CPU 2]: Trap: 0xe  Error: 0x0
```

This is not the same result.  Now, at the same time, I can tell which process is being executed on which CPU at which time.  The last thing output was the `'A'` process which was running on core 2.  And then core 2 faults, trying to run code at address `0x00000000`.

This almost feels like a race condition.

Another execution offers the following:

```
Page Fault
[CPU 1]: EAX: 0x01001000  EBX: 0x8100024c  ECX: 0x01001000
[CPU 1]: EDX: 0x00000000  ESI: 0x90000094  EDI: 0x90000040
[CPU 1]: EBP: 0x00000000  ESP: 0xffffffd8  SS: 0x10
[CPU 1]: EIP: 0x808036ae  EFLAGS: 0x00000046
[CPU 1]: CS: 0x8  DS: 0x28  ES: 0x28  FS: 0x28  GS: 0x60
[CPU 1]: CR0: 0xe0000011  CR2: 0x00000000  CR3: 0x01001000
[CPU 1]: Trap: 0xe  Error: 0x0
```

This is a stack problem.  Another execution yields the following:

```
(0);B(0);B(0);A(1);B(1);B(1);A(1);B(1);B(1);A(0);
Page Fault
[CPU 0]: EAX: 0x01001000  EBX: 0x00000000  ECX: 0x01001000
[CPU 0]: EDX: 0x00000000  ESI: 0xffffffff  EDI: 0x00000086
[CPU 0]: EBP: 0x808038bd  ESP: 0xff805f7c  SS: 0x10
[CPU 0]: EIP: 0x00000000  EFLAGS: 0x00000046
[CPU 0]: CS: 0x8  DS: 0x28  ES: 0x28  FS: 0x28  GS: 0x48
[CPU 0]: CR0: 0(0);B(0);B(0);A(1);B(1);B(1);A(1);B(1);B(1);A(0);
Page Fault
[CPU 0]: EAX: 0x01001000  EBX: 0x00000000  ECX: 0x01001000
[CPU 0]: EDX: 0x00000000  ESI: 0xffffffff  EDI: 0x00000086
[CPU 0]: EBP: 0(0);B(0);B(0);A(1);B(1);B(1);A(1);B(1);B(1);A(0);
Page Fault
[CPU 0]: EAX: 0x01001000  EBX: 0x00000000  ECX: 0x01001000
[CPU 0]: EDX: 0x00000000  ESI: 0xffffffff  EDI: 0x00000086
[CPU 0]: EBP: 0x808038bd  ESP: 0xff805f7c  SS: 0x10
[CPU 0]: EIP: 0x00000000  EFLAGS: 0x00000046
[CPU 0]: CS: 0x8  DS: 0x28  ES: 0x28  FS: 0x28  GS: 0x48
[CPU 0]: CR0: 0xe0000011  CR2: 0x00000000  CR3: 0x01001000
[CPU 0]: Trap: 0xe  Error: 0x0x808038bd  ESP: 0xff805f7c  SS: 0x10
[CPU 0]: EIP: 0x00000000  EFLAGS: 0x00000046
[CPU 0]: CS: 0x8  DS: 0x28  ES: 0x28  FS: 0x28  GS: 0x48
[CPU 0]: CR0: 0xe0000011  CR2: 0x00000000  CR3: 0x01001000
[CPU 0]: Trap: 0xe  Error: 0x0xe0000011  CR2: 0x00000000  CR3: 0x01001000
[CPU 0]: Trap: 0xe  Error: 0x0
```

...  again, executing at address `0x00000000`.

---

### 2020-Mar-09

What I find interesting here is that there are actually 3 faults in that last block, but they are all labeled as CPU0.

I think the first order of business here is going to be to instrument this through Bochs.

Huh???  The stack dump at the point of the exception looks like this:

```
 | STACK 0xff805f80 [0x900001e4] (<unknown>)
 | STACK 0xff805f84 [0x00000000] (<unknown>)
 | STACK 0xff805f88 [0xff805f8c] (<unknown>)
 | STACK 0xff805f8c [0x00000000] (<unknown>)
```

In particular, the `ss` register appears to be blank (`0xff805f8c` is the `esp` register).

But...  the output from the register dump looks like this:

```
Page Fault
[CPU 3]: EAX: 0x00000000  EBX: 0x00000000  ECX: 0x90000218
[CPU 3]: EDX: 0x90000218  ESI: 0x00000bb8  EDI: 0x00000000
[CPU 3]: EBP: 0x00000000  ESP: 0x00000000  SS: 0xff805f90
[CPU 3]: EIP: 0x81000bc0  EFLAGS: 0x00000000
[CPU 3]: CS: 0xfdd4  DS: 0x5a  ES: 0x1e  FS: 0x1  GS: 0x1001000
[CPU 3]: CR0: 0x00000000  CR2: 0x00001023  CR3: 0x00000005
[CPU 3]: Trap: 0xbab6badc  Error: 0xbab6badd
```

Again, the `ss` register is messed up.  So, I have a stack alignment problem for the interrupt stack.

So, the whole stack...

```
 | STACK 0xff805ee8 [0x80801a24] (in IsrDumpState() return from Halt())
 | STACK 0xff805eec [0x80806943] (????)
 | STACK 0xff805ef0 [0x00000003] (<unknown>)
 | STACK 0xff805ef4 [0xbab6badc] (<unknown>)
 | STACK 0xff805ef8 [0xbab6badd] (<unknown>)
 | STACK 0xff805efc [0x80806a77] (text??)
 | STACK 0xff805f00 [0x80803c8c] (in TimerCallBack return from ProcessLockScheduler())
 | STACK 0xff805f04 [0x00000001] (<unknown>)
 | STACK 0xff805f08 [0x80801c48] (in IsrInt0e() return from IsrDumpState())
 | STACK 0xff805f0c [0x900001e4] (<unknown>)
 | STACK 0xff805f10 [0x00000000] (<unknown>)
 | STACK 0xff805f14 [0x00200086] (<unknown>)
 | STACK 0xff805f18 [0x808038bd] (in ProcessUnlockScheduler() return from RestoreInterrupts())
 | STACK 0xff805f1c [0x00200086] (<unknown>)
 | STACK 0xff805f20 [0xffffffff] (<unknown>)
 | STACK 0xff805f24 [0x00bf9eb0] (<unknown>)
 | STACK 0xff805f28 [0x00000000] (<unknown>)
 | STACK 0xff805f2c [0x900001e4] (<unknown>)
 | STACK 0xff805f30 [0x00000000] (<unknown>)
 | STACK 0xff805f34 [0x00000000] (<unknown>)
 | STACK 0xff805f38 [0x80801a4c] (in IsrHandlers() return from call to the ISR Handler)
 | STACK 0xff805f3c [0xff805f5c] (<unknown>)
 | STACK 0xff805f40 [0x80803a2c] (in _SerialPutchar() return from call to SpinUnlock())
 | STACK 0xff805f44 [0x81000a44] (<unknown>)
 | STACK 0xff805f48 [0x80801934] (in IsrcommonStub() return from call to IsrHandler())
 | STACK 0xff805f4c [0x00000010] (<unknown>)
 | STACK 0xff805f50 [0x00000090] (<unknown>)
 | STACK 0xff805f54 [0x00000028] (<unknown>)
 | STACK 0xff805f58 [0x80801934] (in IsrcommonStub() return from call to IsrHandler())
 | STACK 0xff805f5c [0x00000010] (<unknown>)
 | STACK 0xff805f60 [0x00000048] (<unknown>)
 | STACK 0xff805f64 [0x00000028] (<unknown>)
 | STACK 0xff805f68 [0x00000028] (<unknown>)
 | STACK 0xff805f6c [0x00000028] (<unknown>)
 | STACK 0xff805f70 [0x01001000] (<unknown>)
 | STACK 0xff805f74 [0x00000000] (<unknown>)
 | STACK 0xff805f78 [0xe0000011] (<unknown>)
 | STACK 0xff805f7c [0x00000000] (<unknown>)
 | STACK 0xff805f80 [0x900001e4] (<unknown>)
 | STACK 0xff805f84 [0x00000000] (<unknown>)
 | STACK 0xff805f88 [0xff805f8c] (<unknown>)
 | STACK 0xff805f8c [0x00000000] (<unknown>)
 | STACK 0xff805f90 [0x00200086] (<unknown>)
 | STACK 0xff805f94 [0x00000000] (<unknown>)
 | STACK 0xff805f98 [0x00200086] (<unknown>)
 | STACK 0xff805f9c [0x00000020] (<unknown>)
 | STACK 0xff805fa0 [0x00000001] (<unknown>)
 | STACK 0xff805fa4 [0x00000000] (<unknown>)
 | STACK 0xff805fa8 [0x8080355b] (<unknown>)
 | STACK 0xff805fac [0x00000008] (<unknown>)
 | STACK 0xff805fb0 [0x00200292] (<unknown>)
 | STACK 0xff805fb4 [0x00000005] (<unknown>)
 | STACK 0xff805fb8 [0x00000000] (<unknown>)
 | STACK 0xff805fbc [0x00000000] (<unknown>)
 | STACK 0xff805fc0 [0x8080380d] (<unknown>)
 | STACK 0xff805fc4 [0x810001c4] (<unknown>)
 | STACK 0xff805fc8 [0x00000000] (<unknown>)
 | STACK 0xff805fcc [0x00bf9eb0] (<unknown>)
 | STACK 0xff805fd0 [0x80803836] (<unknown>)
 | STACK 0xff805fd4 [0x00000000] (<unknown>)
```

So, from what I am seeing, the same stack is being used by all the CPUs.  Or something like that anyway.

However, I am not tracking CPU0.  But a quick dump of the registers shows:

```
<bochs:10> reg
CPU0:
rax: 00000000_00000000
rbx: 00000000_810009a0
rcx: 00000000_00000001
rdx: 00000000_ff805f5c
rsp: 00000000_ff805ecc
rbp: 00000000_ff805ed4
rsi: 00000000_00200086
rdi: 00000000_00000000
r8 : 00000000_00000000
r9 : 00000000_00000000
r10: 00000000_00000000
r11: 00000000_00000000
r12: 00000000_00000000
r13: 00000000_00000000
r14: 00000000_00000000
r15: 00000000_00000000
rip: 00000000_80803a62
eflags 0x00200097: ID vip vif ac vm rf nt IOPL=0 of df if tf SF zf AF PF CF
```

In fact, here are each of the CPU `esp` values:

| CPU | `esp` value  |
|:---:|:-------------|
|  0  | `0xff805ecc` |
|  1  | `0xff801f0c` |
|  2  | `0xff802f0c` |
|  3  | `0xff805ee8` |

---

OK, it took a lot of reading of logs, but I think I know what is going on here.

The process running on CPU3 is using stack `0xff803000`:

```
00224159538i[      ] ----------------------------------------------------------
00224159538i[      ] CPU 3: mov edi, dword ptr ss:[esp+20]
00224159538i[      ] LEN 4	BYTES: 8b7c2414
00224159538i[      ] MEM ACCESS[0]: 0x00000000ff803f3c (linear) 0x00000100ef3c (physical) RD SIZE: 4
00224159538i[      ]
```

And then it gets into `ProcessSwitch()` and actually finds a new process to run:

```
00224159543i[      ] ----------------------------------------------------------
00224159543i[      ] CPU 3: mov esp, dword ptr ds:[edi]
00224159543i[      ] LEN 2	BYTES: 8b27
00224159543i[      ] MEM ACCESS[0]: 0x00000000900001e4 (linear) 0x0000010101e4 (physical) RD SIZE: 4
00224159543i[      ]
```

Well, CPU0 has no process to run, but is held `ProcessSchedule()` with the same stack.  Well, that is a clear conflict.

I need to figure out what I am going to do about this.  It certainly would have been far simpler to have an idle task for each CPU!!!  But I have a feeling I am going to need to tap into the TSS a little bit.

I got some research to do.

---

So, I think I have a plan.
* Interrupt occurs on the task's local stack (`flags`, `esp`, `ss`)
* Context is saved on the task's local stack (regs, seg-regs)
* The interrupted task state is saved on the current-process structure (as if for a task change)
* The per-CPU structures are queried for which stack to use (different for exceptions than ISRs)
* The `ss` and `esp` are loaded with the new per-CPU stack
* The interrupt is serviced using the per-CPU stack (non-reentrant -- and the current task is updated as required)
* The task's stack is restored
* The context is restored
* Control is returned to the interrupted task

In effect, every exception/interrupt/IRQ/Syscall is now a possible task swap and the structures are prepared accordingly.  This might actually be a little more elegant than my current desgn.

---

### 2020-Mar-10

OK, so the next thing to consider is the timer interrupt.  It is possible (and highly probably) that we will get a timer interrupt on one core while the sheduler is locked on another core.  When that happens, the current solution is to wait....  but why do that?  Why not go ahead and give the CPU to the current process for another timer count?

To accomodate this, I need a spinlock with a timeout.  However, a spinlock with a timeout will not work from within an interrupt context.  Instead, I need a spinlock lock function that attempts once (or a small number of times) and reports success/failure.

It is this component that I need to work on first, as it may address the problem at hand, and if so I will be able to take the other changes on as as a later change.

This will need to be a form of `SpinLock()`, such as `SpinLockTry()`.

---

### 2020-Mar-11

So, `SpinLockTry()` -- first thing to do is implement that.

With those changes, I end up with a triple fault.  Hmmmm... CPU3 faults, not CPU0.  But, not even Bochs gives me any indication where it fails.  But from the Serial Output, it does look like an IPI that triggered it.

Hmmm....

```
(gdb) info registers
eax            0x81000101          -2130706175
ecx            0xffffffff          -1
edx            0xff800084          -8388476
ebx            0x81000160          -2130706080
esp            0xff7fffc4          0xff7fffc4
ebp            0x0                 0x0
esi            0xff800084          -8388476
edi            0x200002            2097154
eip            0x8080439c          0x8080439c
eflags         0x200092            [ AF SF ID ]
cs             0x8                 8
ss             0x10                16
ds             0x28                40
es             0x28                40
fs             0x28                40
gs             0x48                72
```

The `esp` value is not right; do I have a stack overflow?  That would certainly explain the triple fault with no explanation.

---

### 2020-Mar-13

I am back to `#PF` debugging today.  I am observing either the `eip` register or (presumably) the `esp` being set to `0x00000000`.  In the case of the stack, the stack is dropping below `0xfffffffc` and is impacting the x86 MMU tables.

I'm betting I have a structure location out of position in the assembly.

---

### 2020-Mar-14

Happy Pi Day!

```
A(1);Sanity Checking process 0x900001e4 (CPU = 1; stack = 0x00001023; tos = 0xff805fe8)
Stack Dump:
... 0: (0xff805fe8) 0x00000000
... 1: (0xff805fe4) 0x00000006
... 2: (0xff805fe0) 0x8080415a
... 3: (0xff805fdc) 0x00000000
... 4: (0xff805fd8) 0x00372918
... 5: (0xff805fd4) 0x00000000 (eip)
... 6: (0xff805fd0) 0x808038b6 (ebx)
... 7: (0xff805fcc) 0x00000000 (esi)
... 8: (0xff805fc8) 0x00372918 (edi)
... 9: (0xff805fc4) 0x00000000 (ebp)

Page Fault
[CPU 1]: EAX: 0x01001000  EBX: 0x01001000  ECX: 0x01001000
[CPU 1]: EDX: 0x00000000  ESI: 0x00200292  EDI: 0x00200292
[CPU 1]: EBP: 0x00000000  ESP: 0xff805fd4  SS: 0x10
[CPU 1]: EIP: 0x00000000  EFLAGS: 0x00000046
[CPU 1]: CS: 0x8  DS: 0x28  ES: 0x28  FS: 0x28  GS: 0x60
[CPU 1]: CR0: 0xe0000011  CR2: 0x00000000  CR3: 0x01001000
[CPU 1]: Trap: 0xe  Error: 0x0
```

By changing how I handle the registers for `ProcessSwitch()`, I was able to get that resolved.  And now, I am getting a deadlock.  On other executions, I get a `#PF`.

OK, so let's see if we can figure out what happened with this:

```
Sanity Checking process 0x900001e4 (CPU = 0; stack = 0x00001023; tos = 0xff805fa0)
Stack Dump:
... 0: (0xff805fa0) 0x808031ac [in `ProcessLockScheduler()` after call to `DisableInterrupts()`]
... 1: (0xff805f9c) 0x00200096
... 2: (0xff805f98) 0x00000008
... 3: (0xff805f94) 0x81000c20
... 4: (0xff805f90) 0x808031bb [in `ProcessLockScheduler()` after call to `SpinLock()`]
... 5: (0xff805f8c) 0x00000000
... 6: (0xff805f88) 0x00000000
... 7: (0xff805f84) 0x00000008
... 8: (0xff805f80) 0x900001e4
... 9: (0xff805f7c) 0x00000001

Page Fault
[CPU 0]: EAX: 0x8100022c  EBX: 0x90000094  ECX: 0x0000001e
[CPU 0]: EDX: 0x00000001  ESI: 0x00000000  EDI: 0x00200202
[CPU 0]: EBP: 0x808031ac  ESP: 0xff805f78  SS: 0x10
[CPU 0]: EIP: 0x8080371f  EFLAGS: 0x00000046
[CPU 0]: CS: 0x8  DS: 0x28  ES: 0x28  FS: 0x28  GS: 0x48
[CPU 0]: CR0: 0xe0000011  CR2: 0x00000008  CR3: 0x01001000
[CPU 0]: Trap: 0xe  Error: 0x0
```

Faulting instruction:

```
8080371f:       83 7e 08 01             cmpl   $0x1,0x8(%esi)
```

Also, the previous instruction is:

```
80803718:       65 8b 35 04 00 00 00    mov    %gs:0x4,%esi
```

So, `currentThread` is `NULL`.

After decorating the addresses, I am left with the impression I am mixing stacks still...  Well, I know I am, but I believe that is a continuing problem.

Let me work on getting the rpi version compiling again.

OK, to work on changing stacks on an interrupt/exception.  This will need to take place on every interrupt.  There should be several stacks that are available for each CPU.  Let's see:
* One for the IPIs
* One for syscalls/timer
* One for `#PF`
* One for `#DF`
* One for all other execptions
* Each other IRQ's stack is provided by its handler and handled outside this facility?

This means I need to allocate 5 stacks for each CPU.

Ok, I figured out that I am allocating a kernel stack for the CPU but not putting that to use.  So for now I am going make them all use the one stack.

---

### 2020-Mar-15

Today, I am working on refactoring the interrupt stack.  This is going to have some implications on how tasks are changed, as the code to do that now becomes trivial.  Here's the thinking:
* A CPU interrupt takes place
* The interrupt handler makes the effort to save the current process context and decorate

!!!!!  However, all this assumes that every task change takes place while on the cpu stack (and not the process stack).  This is not always the case.  I really need to check in `ProcessSwitch()` and update the stack if required.  Well, more to the point, `ProcessSchedule()` will need to take care of this.

Something does not feel right with this....

OK, `ProcessSchedule()` is only called from 3 locations:
* `ProcessBlock()`
* `ProcessUnlockAndSchedule()`
* `TimerCallBack()`

`TimerCallBack()` can only be called from an interrupt context.  Also, `ProcessSwitch()` is only called from `ProcessSchedule()`.

The reason I am thinking about this is what if I create a software interrupt for scheduling?  In this case, I would only need to call this from `ProcessBlock()` and `ProcessUnlockAndSchedule()`.  `TimerCallBack()` would be able to call `ProcessSchedule()` directly.

So, I think the way to handle this is to check if the stack is on the CPU's stack, and if it is then I can go ahead and call `ProcessSchedule()` directly.  If the stack pionter is not on the CPU's stack, then I will need to execute an interrupt to get there.

So, first is to make sure the interrupts are using the cpu stack and to debug that mess.  And as expected, the first timer interrupt makes a mess.

OK, so this change made a mess of the stack frame, and that stack frame is used to determine what interrupt is being called.  So, I really need to reconstruct the stack frame of an interrupt.  In particular, what resides on the process stack?  And what resides on the CPU stack?  Well, the process stack still needs to hold all the information to restore the process context properly.  So this means all registers.

On the other hand, the kernel stack will need all the same information.  Now, I can either copy the same information to the kernel stack, but that has flaws as well.  I could also place a pointer to the information on the kernel stack; but that may not be mapped in kernel space.  I think the best thing to do is to copy the data to the kernel stack.

So the stack data is expected to look like this (x86):
* `eflags` (highest address)
* `cs`
* `eip`
* error code (if required)
* requires an ACK? (can this be removed?)
* interrupt number
* `eax`
* `ecx`
* `edx`
* `ebx`
* `esp`
* `ebp`
* `esi`
* `edi`
* `cr0`
* `cr2`
* `cr3`
* `ds`
* `es`
* `fs`
* `gs`
* `ss`

These 22 elements would need to be copied to the kernel stack.

---

### 2020-Mar-16

Adding the proper information to the kernel stack gets me to the correct interrupt number.  But I now have a `#PF` again.  Shocker!

```
*(0);>
Page Fault
[CPU 0]: EAX: 0x00000000  EBX: 0x00000000  ECX: 0xffffffff
[CPU 0]: EDX: 0x00000000  ESI: 0xff801000  EDI: 0x000f5a70
[CPU 0]: EBP: 0xff800f8c  ESP: 0xff801f80  SS: 0x10
[CPU 0]: EIP: 0x80801999  EFLAGS: 0x00200092
[CPU 0]: CS: 0x8  DS: 0x28  ES: 0x28  FS: 0x28  GS: 0x48
[CPU 0]: CR0: 0xe0000011  CR2: 0x0000001c  CR3: 0x01001000
[CPU 0]: Trap: 0xe  Error: 0x0
```

This happens on the way back out -- after the `IsrHandler()` call.  In particular:

```
80801990:       65 8b 1d 04 00 00 00    mov    %gs:0x4,%ebx
80801997:       8b 1b                   mov    (%ebx),%ebx
80801999:       8b 43 1c                mov    0x1c(%ebx),%eax
```

So, the current process is set to `NULL`.  Turned out to be not setting up the environment correctly.  I am now getting a deadlock (or what appears to be a deadlock).

---

Ahhhh.... This could be a problem.

```
Calling per cpu(0)
... stack is at 0xff801000
!!>> [0]: Setting the gs selector to 0x48 and the tss selector to 0x50
..back
Calling per cpu(1)
... stack is at 0xff801000
!!>> [1]: Setting the gs selector to 0x60 and the tss selector to 0x68
..back
Calling per cpu(2)
... stack is at 0xff802000
!!>> [2]: Setting the gs selector to 0x78 and the tss selector to 0x80
..back
Calling per cpu(3)
... stack is at 0xff803000
!!>> [3]: Setting the gs selector to 0x90 and the tss selector to 0x98
..back
Finalizing CPU initialization
```

CPU0 and CPU1 share the same stack.

---

### 2020-Mar-17

OK, this is looking like a stack symmetry problem.  Thi is the path out, focusing on the first pop.

```
00174792338i[      ] ----------------------------------------------------------
00174792338i[      ] CPU 1: pop eax
00174792338i[      ] LEN 1	BYTES: 58
00174792338i[      ] MEM ACCESS[0]: 0x00000000ff803f8c (linear) 0x00000100df8c (physical) RD SIZE: 4
00174792338i[      ]
...
00174792358e[CPU1  ] iret: return CS selector null
00174792358i[      ] CPU 1: exception 0dh error_code=0
```

and the path in looks like this:

```
00174788238i[      ] ----------------------------------------------------------
00174788238i[      ] CPU 1: push eax
00174788238i[      ] LEN 1	BYTES: 50
00174788238i[      ] MEM ACCESS[0]: 0x00000000ff803f8c (linear) 0x00000100df8c (physical) WR SIZE: 4
00174788238i[      ]
```

So, this is a correct address -- the address accessed is correct.  How about the last pop vs the first push?

```
00174792358i[      ] ----------------------------------------------------------
00174792358i[      ] CPU 1: pop eax
00174792358i[      ] LEN 1	BYTES: 58
00174792358i[      ] MEM ACCESS[0]: 0x00000000ff803fc8 (linear) 0x00000100dfc8 (physical) RD SIZE: 4
```

vs

```
00174788208i[      ] ----------------------------------------------------------
00174788208i[      ] CPU 1: push 0x00000000
00174788208i[      ] LEN 2	BYTES: 6a00
00174788208i[      ] MEM ACCESS[0]: 0x00000000ff803fd4 (linear) 0x00000100dfd4 (physical) WR SIZE: 4
00174788208i[      ]
```

and there is the symmetry problem.

So, looking at this another way:

| Element | Addr In    | Addr Out   |
|:-------:|:----------:|:----------:|
| `eflags`|        fe0 | 0xff803fe0 |
| `cs`    |        fdc | 0xff803fdc |
| `eip`   |        fd8 | 0xff803fd8 |
| err cd  | 0xff803fd4 |  |
| Ack IRQ | 0xff803fd0 |  |
| Int #   | 0xff803fcc |  |
| `eax`   | 0xff803fc8 | 0xff803fc8 |
| `ecx`   | 0xff803fc4 | 0xff803fc4 |
| `edx`   | 0xff803fc0 | 0xff803fc0 |
| `ebx`   | 0xff803fbc | 0xff803fbc |
| `esp`   | 0xff803fb8 | 0xff803fb8 |
| `ebp`   | 0xff803fb4 | 0xff803fb4 |
| `esi`   | 0xff803fb0 | 0xff803fb0 |
| `edi`   | 0xff803fac | 0xff803fac |
| `cr0`   | 0xff803fa8 |  |
| `cr2`   | 0xff803fa4 |  |
| `cr3`   | 0xff803fa0 |  |
| `ds`    | 0xff803f9c | 0xff803f9c |
| `es`    | 0xff803f98 | 0xff803f98 |
| `fs`    | 0xff803f94 | 0xff803f94 |
| `gs`    | 0xff803f90 | 0xff803f90 |
| `ss`    | 0xff803f8c | 0xff803f8c |

This all looks correct -- I misread the log.

So the values look like this:

```
 | STACK 0xff803fd8 [0x01001000] (<unknown>)
 | STACK 0xff803fdc [0x00000000] (<unknown>)
 | STACK 0xff803fe0 [0xe0000011] (<unknown>)
```

OK, here's the problem:

```
00174788263i[      ] CPU 1: push dword ptr ss:[ebp+52]
00174788263i[      ] LEN 3	BYTES: ff7534
00174788263i[      ] MEM ACCESS[0]: 0x00000000ff803fc0 (linear) 0x00000100dfc0 (physical) RD SIZE: 4
00174788263i[      ] MEM ACCESS[1]: 0x00000000ff803fdc (linear) 0x00000100dfdc (physical) WR SIZE: 4
```

I am writing over the existing stack.  I believe that this may be because I am not properly allocating the stacks I am statically allocating.

---

### 2020-Mar-18

OK, pedantically walking down into the fault, I have found that the fault occurs when:
* creating the process
* there is a page to be mapped
* the IPI is waited for
* there is no other process to be scheduled
* we enable interrupts to allow the timer to fire
* and the process structure is NULL

In other words, the following is not a good bit of code:

```S
    mov         ebx,[gs:4]              ;; get the process structure address
    mov         [ebx+0x1c],eax          ;; save the stack segment
    mov         [ebx],esp               ;; and save the stack pointer
    mov         ebp,esp                 ;; save this stack pointer for later
```

---

### 2020-Mar-19

I left the code yesterday with 2 simultaneous `#PF` events:

```
Page Fault

Page Fault
[CPU 2]: EAX: 0x01001000  EBX: 0x00000028  ECX: 0x01001000
[CPU 0]: EAX: 0x00000004  EBX: 0x01001000  ECX: 0x00000004
[CPU 2]: EDX: 0x00000000  ESI: 0x00000048  EDI: 0x00000010
[CPU 0]: EDX: 0x00200002  ESI: 0x00000028  EDI: 0xe0000011
[CPU 2]: EBP: 0x8080199e  ESP: 0xff800ec4  SS: 0x10
[CPU 0]: EBP: 0x00000010  ESP: 0xff800e94  SS: 0x10
[CPU 2]: EIP: 0x00000028  EFLAGS: 0x00010046
[CPU 0]: EIP: 0x00000048  EFLAGS: 0x00210092
[CPU 2]: CS: 0x8  DS: 0x28  ES: 0x28  FS: 0x28  GS: 0x78
[CPU 0]: CS: 0x8  DS: 0x28  ES: 0x28  FS: 0x28  GS: 0x48
[CPU 2]: CR0: 0xe0000011  CR2: 0x00000028  CR3: 0x01001000
[CPU 0]: CR0: 0xe0000011  CR2: 0x00000048  CR3: 0x01001000
[CPU 2]: Trap: 0xe  Error: 0x0

[CPU 0]: Trap: 0xe  Error: 0x0
```

What is interesting is that both are using the same stack -- still!  So, I need to map these out a bit:

* CPU0 -- `0xff802000` - `0xff803000`
* CPU1 -- `0xff803000` - `0xff804000`
* CPU2 -- `0xff804000` - `0xff805000`
* CPU3 -- `0xff805000` - `0xff806000`
* ProcessA -- Never started
* ProcessB -- Never started
* kInit() -- `0xff801000` - `0xff802000`

Which got me thinking -- do I have a problem with sleeping during a critical section (where a spinlock is held)?  I am certainly doing that.  I commented out the line and I certainly get different results.

So, let's assume for a moment that sleeping (and scheduling) while a spinlock is held is considered improper behavior for the kernel.  What else can we take away from that?
* The `ProcessSleep*()` functions will never be called on a given CPU with a lock held
* `ProcessBlock()` will never be called on a given CPU with a lock held
* `ProcessSchedule()` should not be called on the CPU
* The timer will not fire on the CPU (locks disable interrupts on the CPU)

So, when a lock is held on another CPU:
* The process holding the lock is on the `PROC_RUNNING` status (and cannot be picked up for scheduling on another CPU)
* Any other process in the `PROC_READY` status can be scheduled
* If there are none in the `PROC_READY` status and the current process blocked, then the CPU idles
* Otherwise, if the current process is `PROC_RUNNING` it can continue to run

In short, everything should be able to change to the per-CPU stack on an interrupt.

So, with that, let me see if I can get the timer firing on all the CPUs properly.

And I am still getting a `#PF`.  So, I need to get that sorted out properly first.

Going back to the stacks:
* `0xff800000` -- Never properly allocated; but used
* `0xff801000` -- StackAlloc: Allocating the stack at base address 0xff801000 (STACK_LOCATION)
* `0xff802000` -- StackFind(): New stack allocated at address 0xff802000 (CPU0)
* `0xff803000` -- StackFind(): New stack allocated at address 0xff803000 (CPU1)
* `0xff804000` -- StackFind(): New stack allocated at address 0xff804000 (CPU2)
* `0xff805000` -- StackFind(): New stack allocated at address 0xff805000 (CPU3)
* `0xff806000` -- StackFind(): New stack allocated at address 0xff806000 (Process for CPU1)
* `0xff807000` -- StackFind(): New stack allocated at address 0xff807000 (Process for CPU2)
* `0xff808000` -- StackFind(): New stack allocated at address 0xff808000 (Process for CPU3)
* `0xff809000` -- Unused
* `0xff80a000` -- Unused
* `0xff80b000` -- Unused
* `0xff80c000` -- Unused
* `0xff80d000` -- Unused
* `0xff80e000` -- Unused
* `0xff80f000` -- Unused

So, this was resolved -- I was using `STACK_LOCATION` rather than `STACK_BASE` for the base of the allocations.  Cleaning that up gets the stacks aligned -- 8 stacks allocated.

However, I am still getting a `#PF`:

```
Page Fault
[CPU 1]: EAX: 0x01001000  EBX: 0x81000a84  ECX: 0x01001000
[CPU 1]: EDX: 0x00000000  ESI: 0x80803a3c  EDI: 0x810009c0
[CPU 1]: EBP: 0xff800f38  ESP: 0xff800f04  SS: 0x10
[CPU 1]: EIP: 0xff802000  EFLAGS: 0x00000046
[CPU 1]: CS: 0x8  DS: 0x28  ES: 0x28  FS: 0x28  GS: 0x60
[CPU 1]: CR0: 0xe0000011  CR2: 0x01001000  CR3: 0x01001000
[CPU 1]: Trap: 0xe  Error: 0x0
```

What is interesting about this is that the stack is for the `kInit()` stack but that process should be sleeping for 2 seconds and therefore the stack should be that of CPU0 (`0xff801000`).

OK, another thing is that I am allocating the stack for the initialization process into the CPU structures, and then replacing that stack with the proper CPU stack once the initialization gets mature enough to allocate another stack.

So, with that, let's try that again:
* `0xff800000` -- kInit() initialization stack (`STACK_BASE`); later allocated with StackAlloc()
* `0xff801000` -- StackFind(): New stack allocated at address 0xff801000 (CPU0)
* `0xff802000` -- StackFind(): New stack allocated at address 0xff802000 (Process for CPU1)
* `0xff803000` -- StackFind(): New stack allocated at address 0xff803000 (Process for CPU2)
* `0xff804000` -- StackFind(): New stack allocated at address 0xff804000 (Process for CPU3)
* `0xff805000` -- StackFind(): New stack allocated at address 0xff805000 (CPU1)
* `0xff806000` -- StackFind(): New stack allocated at address 0xff806000 (CPU2)
* `0xff807000` -- StackFind(): New stack allocated at address 0xff807000 (CPU3)

Now, with this corrected information, the `esp` address of `0xff800f04` is part of the `kInit()` stack.

So, I think I need to dump the CPU structures:

```
Dumping the CPU structures:
=====================================

   CPU0:
   ----
   CPU Stack Top: 0xff802000
   Process Stack Loc: 0xff801000

   =====================================

   CPU1:
   ----
   CPU Stack Top: 0xff806000
   Process Stack Loc: 0xff802000

   =====================================

   CPU2:
   ----
   CPU Stack Top: 0xff807000
   Process Stack Loc: 0xff803000

   =====================================

   CPU3:
   ----
   CPU Stack Top: 0xff808000
   Process Stack Loc: 0xff804000

   =====================================
```

What is interesting is that there is overlap.  Wait, there is difference in what I am looking at: the process is the stack base, not the top.

So stripping this down to the basics (and not enabling interrupts which may be corrupting these numbers), I am still not right!!:

```
Dumping the CPU structures:
=====================================
   CPU0:
   ----
   CPU Stack Top: 0xff802000
   Process Stack Top: 0x00001000
   =====================================
   CPU1:
   ----
   CPU Stack Top: 0xff806000
   Process Stack Top: 0xff804000
   =====================================
   CPU2:
   ----
   CPU Stack Top: 0xff807000
   Process Stack Top: 0xff805000
   =====================================
   CPU3:
   ----
   CPU Stack Top: 0xff808000
   Process Stack Top: 0xff806000
   =====================================
Starting processes
```

The `entryAp.s` is setting the stack properly:

```S
    mov     esp,[ebx+4]                 ;; set the stack
```

OK, I have myopia!!!

After correcting the code outputting the debugging info, I have the following:

```
Dumping the CPU structures:
=====================================
   CPU0:
   ----
   CPU Stack Top: 0xff802000
   Process Stack Top: 0x00000000
   =====================================
   CPU1:
   ----
   CPU Stack Top: 0xff806000
   Process Stack Top: 0xff803000
   =====================================
   CPU2:
   ----
   CPU Stack Top: 0xff807000
   Process Stack Top: 0xff804000
   =====================================
   CPU3:
   ----
   CPU Stack Top: 0xff808000
   Process Stack Top: 0xff805000
   =====================================
Starting processes
```

And, the CPU0 process has not top of stack reported.  This will be in `ProcessInit()`.

OK, cleaning that up yields the correct results.

Now, I enable interrupts on CPU0 and observe the results.  This still looks reasonable:

```
Dumping the CPU structures:
=====================================
   CPU0:
   ----
   CPU Stack Top: 0xff802000
   Process Stack Top: 0xff800f7c
   =====================================
   CPU1:
   ----
   CPU Stack Top: 0xff806000
   Process Stack Top: 0xff803000
   =====================================
   CPU2:
   ----
   CPU Stack Top: 0xff807000
   Process Stack Top: 0xff804000
   =====================================
   CPU3:
   ----
   CPU Stack Top: 0xff808000
   Process Stack Top: 0xff805000
   =====================================
Starting processes
```

The key difference here is that CPU0 process has been interrupted, and therefore the stack top has been updated.

So, now I make the same change (enable interrupts) on the other CPUs.  And, again, this is reasonable:

```
Dumping the CPU structures:
=====================================
   CPU0:
   ----
   CPU Stack Top: 0xff802000
   Process Stack Top: 0xff800f6c
   =====================================
   CPU1:
   ----
   CPU Stack Top: 0xff806000
   Process Stack Top: 0xff802f9c
   =====================================
   CPU2:
   ----
   CPU Stack Top: 0xff807000
   Process Stack Top: 0xff803f9c
   =====================================
   CPU3:
   ----
   CPU Stack Top: 0xff808000
   Process Stack Top: 0xff804f9c
   =====================================
Starting processes
```

... and for the same reasons.

OK, so when I re-intruduce the process blocking, I end up with the following:

```
Dumping the CPU structures:
=====================================
   CPU0:
   ----
   CPU Stack Top: 0xff802000
   Process Stack Top: 0xff800f18
   =====================================
   CPU1:
   ----
   CPU Stack Top: 0xff806000
   =====================================
   CPU2:
   ----
   CPU Stack Top: 0xff807000
   =====================================
   CPU3:
   ----
   CPU Stack Top: 0xff808000
   =====================================
Starting processes
.(0);Locking scheduler on CPU0
Preparing to sleep....
Unlocking scheduler on CPU0

Page Fault
[CPU 2]: EAX: 0x01001000  EBX: 0x81000a84  ECX: 0x01001000
[CPU 2]: EDX: 0x00000000  ESI: 0x80803a3c  EDI: 0x00000082
[CPU 2]: EBP: 0x810009c0  ESP: 0xff802f04  SS: 0x10
[CPU 2]: EIP: 0xff806000  EFLAGS: 0x00000046
[CPU 2]: CS: 0x8  DS: 0x28  ES: 0x28  FS: 0x28  GS: 0x78
[CPU 2]: CR0: 0xe0000011  CR2: 0x01001000  CR3: 0x01001000
[CPU 2]: Trap: 0xe  Error: 0x0
```

---

### 2020-Mar-20

So, the interesting thing from yesterday is that the stack is from one of the `kInitAp()` processes whereas the IP is from one of the CPU stacks.  This is the top of CPU1 stack or the base of CPU2 stack.

So this makes me wonder if the voluntarily giving up the CPU is creating a problem..., such as when sleeping with nothing ready to run.

* Process calls `ProcessSleep()`
  * `ProcessSleep()` calls `ProcessMicroSleepUntil()`
    * `ProcessMicroSleepUntil()` calls `ProcessLockAndPostpone()`
    * `ProcessMicroSleepUntil()` calls `ProcessDoMicroSleepUntil()`
      * `ProcessDoMicroSleepUntil()` calls `ProcessDoBlock()`
        * `ProcessDoBlock()` calls `ProcessSchedule()`
          * `ProcessSchedule()` calls `ProcessNext()` which returns `NULL`
          * `ProcessSchedule()` assigns `currentThread` to be `NULL`
          * `ProcessSchedule()` calls `ProcessUnlockScheduler()`
          * `ProcessSchedule()` enables interrupts
          * `ProcessSchedule()` halts the processor to wait for the timer to fire, calling `TimerCallBack()`
            * !!!!
    * `ProcessMicroSleepUntil()` calls `ProcessUnlockAndSchedule()`

OK, it is at this point I realize that `ProcessSchedule()` releases the scheduler lock when it probably should not.  When it does this, `TimerCallBack()` can get this same lock and it should not be able to do this since the structures are in transition.  Instead, the lock should remain with `ProcessSchedule()` but the `processChangePending` flag is set.

Now, my next problem is that I need to be able to ready processes (which is being bypassed with the lock unavailable).  As a result, the system deadlocks.  The problem is that the scheduler blocks on CPUn with nothing to do, but nothing else will be able to do anything (block, unlock timer, etc.) until this lock is released.

At this point, I think the better answer is to redo the scheduler so that there is always an idle process to run on each CPU.  It's going to get me past this hurdle faster and is still an acceptable solution at this piont.

---

### 2020-Mar-21

Thinking about this last night while I could not sleep, I think I am going to allocate a special-purpose process for each CPU and add that into the CPU structures.  Now, can this really be the same process?  Technically, no since they all cannot use the same stack.  However, this is a special-purpose process that will not call anything (maybe something like `HaltCpu()`) which will not use the stack for anything.  Actually, this will not work either since getting the process to start is going to require a stack.  Better to just allocate `MAX_CPUS` idle processes and drop those into the per-CPU structures.

```
!!! ASSERT FAILURE !!!
/home/adam/workspace/century-os/modules/kernel/src/process/ProcessCreate.cc(34) rv != NULL Out of memory allocating a new Process_t


Out of memory allocating a new Process_t
```

Holy crap!!  Did I really run out of heap already??

Yup:

```c++
//
// -- This is how much of the heap we will allocate at compile time.  This will really be frames that will be moved
//    during initialization.  This is 64K.
//    -------------------------------------------------------------------------------------------------------------
#define INITIAL_HEAP        (4096*16)
```

I am only starting with 64K of memory.  I think it is finally time to write that `HeapExpand()` function....  It's taken me about 10 years to get to this point after the initial development of the heap.

So, the challenge here will be to insert the new block into the list of holes without the ability to allocate a nee fixed list entry.  I do have a function (`HeapNewListEntry()`) which will look for a list entry from the static array, which should work for now.  This will need to be revisited again at some point.

So, this `HeapExpand()` function is going to look a lot like `HeapInit()` except it will be dealing with expanding the existing heap rather than initializing the first block in the heap.

So, am I running out of memory before I call `HeapInit()`???

```
!!! ASSERT FAILURE !!!
/home/adam/workspace/century-os/modules/kernel/src/heap/HeapExpand.cc(44) (archsize_t)kHeap->endAddr < (archsize_t)kHeap->maxAddr Unable to expand heap any more

Heap End: 0x00000000
Heap Max: 0x00000000
```

I am.  The CPUs are initialized in `PlatformEarlyInit()`, whereas the Heap is initialized immediately after that call.  So, I wasn't out of memory.

To protect myself from this mistake again, I added the following into `HeapAlloc()`:

```c++
    if (!assert_msg(kHeap->strAddr != 0, "Heap is not initialized!")) {
        CpuPanicPushRegs("Heap is not initialized in call to allocate memory");
    }
```

... which results in the following message:

```
!!! ASSERT FAILURE !!!
/home/adam/workspace/century-os/modules/kernel/src/heap/HeapAlloc.cc(60) kHeap->strAddr != 0 Heap is not initialized!


Heap is not initialized in call to allocate memory
```

OK, that's better.  Now, to move the CPU Idle Process creation into `ProcessInit()`.

---

OK, crap!!!  I cannot use `ProcessCreate()` to create the idle processes; the schedule will want to schedule it.  I need to manually create them.

It looks like I am still getting the same stack for multiple CPUs:

```
Page Fault
[CPU 1]: EAX: 0x01001000  EBX: 0x81000160  ECX: 0x01001000
[CPU 1]: EDX: 0x00000000  ESI: 0xffffffff  EDI: 0xff804f18
[CPU 1]: EBP: 0x00202e48  ESP: 0xff80af30  SS: 0x10
[CPU 1]: EIP: 0xff804f18  EFLAGS: 0x00000086
[CPU 1]: CS: 0x8  DS: 0x28  ES: 0x28  FS: 0x28  GS: 0x60
[CPU 1]: CR0: 0xe0000011  CR2: 0x01001000  CR3: 0x01001000
[CPU 1]: Trap: 0xe  Error: 0x0


Page Fault
[CPU 2]: EAX: 0xfee00000  EBX: 0xff80b000  ECX: 0x01001000
[CPU 2]: EDX: 0xff80afa8  ESI: 0x80807ce9  EDI: 0xff804fec
[CPU 2]: EBP: 0xff804f18  ESP: 0xff80af20  SS: 0x10
[CPU 2]: EIP: 0x00000000  EFLAGS: 0x00000086
[CPU 2]: CS: 0x8  DS: 0x28  ES: 0x28  FS: 0x28  GS: 0x78
[CPU 2]: CR0: 0xe0000011  CR2: 0x00000000  CR3: 0x01001000
[CPU 2]: Trap: 0xe  Error: 0x0
```

---

OK, so I am still having a problem with voluntarily yielding the CPU.  When an interrupt fires, the process change happens properly.  However, when a process calls a form of `Sleep()` (and I assume anything else that will block a running process, ultimately calling `ProcessSchedule()`), there is a stack problem.

I think at this point my problem is really over synchronization between disabling interrupts and restoring interrupts.  What is leading me to consider this (cleaning up the normal output from `kprintf()`)?

```
Stack Dump:
... [CPU0] 0: (0xff801f24) 0x00243d58
... [CPU0] 1: (0xff801f20) 0x80803893
... [CPU0] 2: (0xff801f1c) 0xff800f3c
... [CPU0] 3: (0xff801f18) 0xff800f3c
... [CPU0] 4: (0xff801f14) 0xff800f3c
... [CPU0] 5: (0xff801f10) 0x00000000
... [CPU0] 6: (0xff801f0c) 0x00000000
... [CPU0] 7: (0xff801f08) 0x00000000
... [CPU0] 8: (0xff801f04) 0x81000160
... [CPU0] 9: (0xff801f00) 0x80803b3d
... [CPU0] 10: (0xff801efc) 0x00000000
... [CPU0] 11: (0xff801ef8) 0x00000000
... [CPU0] 12: (0xff801ef4) 0x00200002
... [CPU0] 13: (0xff801ef0) 0x00000000
... [CPU0] 14: (0xff801eec) 0x00000000
... [CPU0] 15: (0xff801ee8) 0x01340000
... [CPU0] 16: (0xff801ee4) 0x00000000
... [CPU0] 17: (0xff801ee0) 0xff801f44
... [CPU0] 18: (0xff801edc) 0x00000033
... [CPU0] 19: (0xff801ed8) 0x01000000
... [CPU0] 20: (0xff801ed4) 0x80808257
... [CPU0] 21: (0xff801ed0) 0x80808257
... [CPU0] 22: (0xff801ecc) 0x0000000a
... [CPU0] 23: (0xff801ec8) 0x00200002
... [CPU0] 24: (0xff801ec4) 0x80804a34
.. switching to the new task at 0x90000098 on CPU0

Page Fault
[CPU 0]: EAX: 0x01001000  EBX: 0x81000a0c  ECX: 0x01001000
[CPU 0]: EDX: 0x00000000  ESI: 0xffffffff  EDI: 0x00000001
[CPU 0]: EBP: 0x00243d58  ESP: 0xff801f30  SS: 0x10
[CPU 0]: EIP: 0x00000001  EFLAGS: 0x00200086
[CPU 0]: CS: 0x8  DS: 0x28  ES: 0x28  FS: 0x28  GS: 0x48
[CPU 0]: CR0: 0xe0000011  CR2: 0x00000001  CR3: 0x01001000
[CPU 0]: Trap: 0xe  Error: 0x0
```

and then...

```s
80804a2f:       e8 5c f1 ff ff          call   80803b90 <RestoreInterrupts>
80804a34:       83 c4 10                add    $0x10,%esp
```

The top of the stack always appears to be after a call to `RestoreInterrupts()` in `kprintf()`.  So, more to the point:

```s
RestoreInterrupts:
    mov     eax,[esp+4]                                        ;; get the parameter
    push    eax                                                ;; push it on the stack
    popfd                                                      ;; and pop off the flags register
    ret
```

The `popfd` instruction will restore interrupts.  When that happens, the timer is likely to fire before the `ret` happens especially when delayed for some time.  But is this really the problem, or just a symptom or trigger??

---

### 2020-Mar-22

Well, today I could not take it anymore.  I switched over to the bochs instrumentation project and added a register dump and an `eip` dump to that instrumentation.  I also turned on the logging for all the CPUs.  Then I recreated the problem.  The resulting log file is 44.8GB.

It is able to be processed by `glogg`, but it takes a long time to do any searches.  It will take some time to get to the root of the problem.  However, I am certain I have the problem captured with all the relevant information required to find it.  Now, it's just a needle in just 1 haystack, not a field if them.

Well, problem #1 was I was outputting the wrong side of the stack... meaning I was outputting garbage.  I have corrected that and am capturing logs again (which I can check against the output).

OK, so the output I am researching is the following:

```
*(1);*(2);*(3);*(0);*(1);Preparing to schedule a new task on CPU2...
*(3);*(0);.(1);Preparing to schedule a new task on CPU2...
*(3);*(0);*(1);..  getting a new process to run on CPU2...
*(3);*(0);*(1);..  resulting process address 0x90000040
*(3);*(0);*(1);Sanity Checking process 0x90000040 (CPU = 2; stack = 0xff800000; tos = 0xff800fa0)
*(3);*(0);*(1);Stack Dump:
*(3);*(0);*(1);... [CPU2] 0: (0xff800fa0) 0x01009000
*(3);*(0);*(1);... [CPU2] 1: (0xff800fa4) 0x00000000
*(3);*(0);*(1);... [CPU2] 2: (0xff800fa8) 0x90000098
*(3);*(0);*(1);... [CPU2] 3: (0xff800fac) 0x80808261
*(3);*(0);*(1);... [CPU2] 4: (0xff800fb0) 0x80803802
*(3);*(0);*(1);... [CPU2] 5: (0xff800fb4) 0x90000098
*(3);*(0);*(1);... [CPU2] 6: (0xff800fb8) 0x00000000
*(3);*(0);*(1);... [CPU2] 7: (0xff800fbc) 0x00000029
*(3);*(0);*(1);... [CPU2] 8: (0xff800fc0) 0x00000029
*(3);*(0);*(1);... [CPU2] 9: (0xff800fc4) 0x00200096
*(3);*(0);*(1);... [CPU2] 10: (0xff800fc8) 0x00338b50
*(3);*(0);*(1);... [CPU2] 11: (0xff800fcc) 0x00000000
*(3);*(0);*(1);... [CPU2] 12: (0xff800fd0) 0x80803ab6
*(3);*(0);*(1);... [CPU2] 13: (0xff800fd4) 0x00200096
*(3);*(0);*(1);... [CPU2] 14: (0xff800fd8) 0x00338b50
*(3);*(0);*(1);... [CPU2] 15: (0xff800fdc) 0x00000000
*(3);*(0);*(1);... [CPU2] 16: (0xff800fe0) 0x8080460f
*(3);*(0);*(1);... [CPU2] 17: (0xff800fe4) 0x00000006
*(3);*(0);*(1);... [CPU2] 18: (0xff800fe8) 0x00000000
*(3);*(0);*(1);... [CPU2] 19: (0xff800fec) 0x00200292
*(3);*(0);*(1);... [CPU2] 20: (0xff800ff0) 0x00200292
*(3);*(0);*(1);... [CPU2] 21: (0xff800ff4) 0x01003800
*(3);*(0);*(1);... [CPU2] 22: (0xff800ff8) 0x001000c8
*(3);*(0);*(1);... [CPU2] 23: (0xff800ffc) 0x01003800
*(3);*(0);*(1);... [CPU2] 24: (0xff801000) 0x00000000
*(3);*(0);*(1);.. switching to the new task at 0x90000040 on CPU2
*(3);*(0);*(1);.(2);Preparing to schedule a new task on CPU3...
*(0);*(1);Locking scheduler on CPU2
Preparing to schedule a new task on CPU3...
*(0);*(1);..  getting a new process to run on CPU3...
*(0);*(1);..  resulting process address 0x90000250
*(0);*(1);Sanity Checking process 0x90000250 (CPU = 3; stack = 0xff803000; tos = 0xff80af24)
*(0);*(1);Stack Dump:
*(0);*(1);... [CPU3] 0: (0xff80af24) 0x00000000
*(0);*(1);... [CPU3] 1: (0xff80af28) 0x90000040
*(0);*(1);... [CPU3] 2: (0xff80af2c) 0x00000019
*(0);*(1);... [CPU3] 3: (0xff80af30) 0xff801004
*(0);*(1);... [CPU3] 4: (0xff80af34) 0x808037c5
*(0);*(1);... [CPU3] 5: (0xff80af38) 0x90000040
*(0);*(1);... [CPU3] 6: (0xff80af3c) 0x90000040
*(0);*(1);... [CPU3] 7: (0xff80af40) 0x00000002
*(0);*(1);... [CPU3] 8: (0xff80af44) 0x00000027
*(0);*(1);... [CPU3] 9: (0xff80af48) 0xff80b001
*(0);*(1);... [CPU3] 10: (0xff80af4c) 0x81000a2c
*(0);*(1);... [CPU3] 11: (0xff80af50) 0x00000000
*(0);*(1);... [CPU3] 12: (0xff80af54) 0x8080418e
*(0);*(1);... [CPU3] 13: (0xff80af58) 0x00000000
*(0);*(1);... [CPU3] 14: (0xff80af5c) 0x00000000
*(0);*(1);... [CPU3] 15: (0xff80af60) 0x00000000
*(0);*(1);... [CPU3] 16: (0xff80af64) 0x0033d1a0
*(0);*(1);... [CPU3] 17: (0xff80af68) 0x00000000
*(0);*(1);... [CPU3] 18: (0xff80af6c) 0x00000000
*(0);*(1);... [CPU3] 19: (0xff80af70) 0x00000000
*(0);*(1);... [CPU3] 20: (0xff80af74) 0xff80b000
*(0);*(1);... [CPU3] 21: (0xff80af78) 0x808088f9
*(0);*(1);... [CPU3] 22: (0xff80af7c) 0xff803fec
*(0);*(1);... [CPU3] 23: (0xff80af80) 0xff803f18
*(0);*(1);... [CPU3] 24: (0xff80af84) 0x80801c8c
*(0);*(1);.. switching to the new task at 0x90000250 on CPU3
*(0);*(1);[CPU2]: Preparing to sleep....

[CPU3]: General Protection Fault
*(0);*(1);.. qualified to sleep on CPU2
Entered _LApicBroadcastIpi on CPU 3 for dev 0x81000080
*(0);*(1);Preparing to block process at address 0x90000040 voluntarily on CPU2
.. Qualified on CPU 3
*(0);*(1);Preparing to schedule a new task on CPU2...
.. The ESR report 0x00000000
Unlocking scheduler on CPU2
.. Delivery status reports 0x000c00f0
Preparing to schedule a new task on CPU2...
.. Completed on CPU 3
..  getting a new process to run on CPU2...
[CPU 3]: EAX: 0x00000019  EBX: 0x90000250  ECX: 0xff80af24
..  resulting process address 0x900002a8
[CPU 3]: EDX: 0x00000000  ESI: 0x808088f9  EDI: 0xff803fec
Sanity Checking process 0x900002a8 (CPU = 2; stack = 0xff804000; tos = 0xff80bf24)
[CPU 3]: EBP: 0xff803f18  ESP: 0xff80af08  SS: 0x10
Stack Dump:
[CPU 3]: EIP: 0x80801b56  EFLAGS: 0x00010086
... [CPU2] 0: (0xff80bf24) 0xff80bfa8
[CPU 3]: CS: 0x8  DS: 0x28  ES: 0x28  FS: 0x28  GS: 0x90
... [CPU2] 1: (0xff80bf28) 0x00000003
[CPU 3]: CR0: 0xe0000011  CR2: 0x00000000  CR3: 0x01001000
... [CPU2] 2: (0xff80bf2c) 0xff803fec
[CPU 3]: Trap: 0xd  Error: 0x18

... [CPU2] 3: (0xff80bf30) 0xff80aed8
... [CPU2] 4: (0xff80bf34) 0xff80bfa8
... [CPU2] 5: (0xff80bf38) 0x00000003
... [CPU2] 6: (0xff80bf3c) 0xff803fec
... [CPU2] 7: (0xff80bf40) 0xff80aed8
... [CPU2] 8: (0xff80bf44) 0x80801c61
... [CPU2] 9: (0xff80bf48) 0x8080703b
... [CPU2] 10: (0xff80bf4c) 0x00000003
... [CPU2] 11: (0xff80bf50) 0x0000000d
... [CPU2] 12: (0xff80bf54) 0x00000018
... [CPU2] 13: (0xff80bf58) 0x808088f9
... [CPU2] 14: (0xff80bf5c) 0xff80c000
... [CPU2] 15: (0xff80bf60) 0x808088f9
... [CPU2] 16: (0xff80bf64) 0x80801e71
... [CPU2] 17: (0xff80bf68) 0xff80bfa8
... [CPU2] 18: (0xff80bf6c) 0x00000003
... [CPU2] 19: (0xff80bf70) 0x00000000
... [CPU2] 20: (0xff80bf74) 0xff80c000
... [CPU2] 21: (0xff80bf78) 0x808088f9
... [CPU2] 22: (0xff80bf7c) 0xff804fec
... [CPU2] 23: (0xff80bf80) 0xff804f18
... [CPU2] 24: (0xff80bf84) 0x80801c8c
.. switching to the new task at 0x900002a8 on CPU2

Page Fault
Entered _LApicBroadcastIpi on CPU 2 for dev 0x81000080
.. Qualified on CPU 2
.. The ESR report 0x00000000
.. Delivery status reports 0x000c00f0
.. Completed on CPU 2
[CPU 2]: EAX: 0x01001000  EBX: 0xff80aed8  ECX: 0x01001000
[CPU 2]: EDX: 0x00000000  ESI: 0xff803fec  EDI: 0x00000003
[CPU 2]: EBP: 0xff80bfa8  ESP: 0xff80bf10  SS: 0x10
[CPU 2]: EIP: 0xff80bfa8  EFLAGS: 0x00010046
[CPU 2]: CS: 0x8  DS: 0x28  ES: 0x28  FS: 0x28  GS: 0x78
[CPU 2]: CR0: 0xe0000011  CR2: 0x01001000  CR3: 0x01001000
[CPU 2]: Trap: 0xe  Error: 0x2
```

And in particular, I am after the triggering `#GP` exception, cleaned up here:

```
[CPU3]: General Protection Fault
Entered _LApicBroadcastIpi on CPU 3 for dev 0x81000080
.. Qualified on CPU 3
.. The ESR report 0x00000000
.. Delivery status reports 0x000c00f0
.. Completed on CPU 3
[CPU 3]: EAX: 0x00000019  EBX: 0x90000250  ECX: 0xff80af24
[CPU 3]: EDX: 0x00000000  ESI: 0x808088f9  EDI: 0xff803fec
[CPU 3]: EBP: 0xff803f18  ESP: 0xff80af08  SS: 0x10
[CPU 3]: EIP: 0x80801b56  EFLAGS: 0x00010086
[CPU 3]: CS: 0x8  DS: 0x28  ES: 0x28  FS: 0x28  GS: 0x90
[CPU 3]: CR0: 0xe0000011  CR2: 0x00000000  CR3: 0x01001000
[CPU 3]: Trap: 0xd  Error: 0x18
```

Also, the stack on CPU3 looks like this:

```
Sanity Checking process 0x90000250 (CPU = 3; stack = 0xff803000; tos = 0xff80af24)
Stack Dump:
... [CPU3] 0: (0xff80af24) 0x00000000
... [CPU3] 1: (0xff80af28) 0x90000040
... [CPU3] 2: (0xff80af2c) 0x00000019
... [CPU3] 3: (0xff80af30) 0xff801004
... [CPU3] 4: (0xff80af34) 0x808037c5
... [CPU3] 5: (0xff80af38) 0x90000040
... [CPU3] 6: (0xff80af3c) 0x90000040
... [CPU3] 7: (0xff80af40) 0x00000002
... [CPU3] 8: (0xff80af44) 0x00000027
... [CPU3] 9: (0xff80af48) 0xff80b001
... [CPU3] 10: (0xff80af4c) 0x81000a2c
... [CPU3] 11: (0xff80af50) 0x00000000
... [CPU3] 12: (0xff80af54) 0x8080418e
... [CPU3] 13: (0xff80af58) 0x00000000
... [CPU3] 14: (0xff80af5c) 0x00000000
... [CPU3] 15: (0xff80af60) 0x00000000
... [CPU3] 16: (0xff80af64) 0x0033d1a0
... [CPU3] 17: (0xff80af68) 0x00000000
... [CPU3] 18: (0xff80af6c) 0x00000000
... [CPU3] 19: (0xff80af70) 0x00000000
... [CPU3] 20: (0xff80af74) 0xff80b000
... [CPU3] 21: (0xff80af78) 0x808088f9
... [CPU3] 22: (0xff80af7c) 0xff803fec
... [CPU3] 23: (0xff80af80) 0xff803f18
... [CPU3] 24: (0xff80af84) 0x80801c8c
.. switching to the new task at 0x90000250 on CPU3
```

and the log shows the following:

```
00191929393e[CPU3  ] load_seg_reg(FS, 0x0019): RPL & CPL must be <= DPL
00191929393i[      ] CPU 3: exception 0dh error_code=18
00191929393i[CPU3  ] CPU is in protected mode (active)
00191929393i[CPU3  ] CS.mode = 32 bit
00191929393i[CPU3  ] SS.mode = 32 bit
00191929393i[CPU3  ] EFER   = 0x00000000
00191929393i[CPU3  ] | EAX=00000019  EBX=90000250  ECX=ff80af24  EDX=00000000
00191929393i[CPU3  ] | ESP=ff80af30  EBP=ff803f18  ESI=808088f9  EDI=ff803fec
00191929393i[CPU3  ] | IOPL=0 id vip vif ac vm rf nt of df if tf SF zf af PF cf
00191929393i[CPU3  ] | SEG sltr(index|ti|rpl)     base    limit G D
00191929393i[CPU3  ] |  CS:0008( 0001| 0|  0) 00000000 ffffffff 1 1
00191929393i[CPU3  ] |  DS:0028( 0005| 0|  0) 00000000 ffffffff 1 1
00191929393i[CPU3  ] |  SS:0010( 0002| 0|  0) 00000000 ffffffff 1 1
00191929393i[CPU3  ] |  ES:0028( 0005| 0|  0) 00000000 ffffffff 1 1
00191929393i[CPU3  ] |  FS:0028( 0005| 0|  0) 00000000 ffffffff 1 1
00191929393i[CPU3  ] |  GS:0090( 0012| 0|  0) 810003dc 00000007 0 1
00191929393i[CPU3  ] | EIP=80801b58 (80801b56)
00191929393i[CPU3  ] | CR0=0xe0000011 CR2=0x00000000
00191929393i[CPU3  ] | CR3=0x01001000 CR4=0x00000000
00191929393i[      ] CPU 3: interrupt 0dh
```

Ok, this is interesting...  from the stack, the `fs` register is the third element on the stack (not something deeper as I would expect).  Unless it's deeper than 25 elements in, this is the only stack element it can be.

From the log entries above, the faulting instruction is `0x80801b58`.  This instruction is:

```s
80801b56:       8e e0                   mov    %eax,%fs
```

which is what I would expected to have been.

The instructions (from stack replacement to this point) is:

```s
80801b51:       89 cc                   mov    %ecx,%esp

80801b53 <IsrCommonStub.3>:
80801b53:       58                      pop    %eax
80801b54:       58                      pop    %eax
80801b55:       58                      pop    %eax
80801b56:       8e e0                   mov    %eax,%fs
```

So, this is genuinely and properly the third thing on the stack.  Now, on the other hand, I am seeing nothing that would contain the values of `ds`, `es` or `fs`.

I think the instruction I am interested in is:

```s
80801b3e:       65 8b 1d 04 00 00 00    mov    %gs:0x4,%ebx
```

And in the log, this instruction boils down to:

```
00191929383i[      ] CPU 3 at 0x80801b3e: mov ebx, dword ptr gs:0x00000004   (reg results):
00191929383i[      ] LEN 7	BYTES: 658b1d04000000
00191929383i[      ]   EAX: 0x00000000; EBX: 0x90000250; ECX 0x01001000; EDX: 0x00000000
00191929383i[      ]   ESP: 0xff80afa8; EBP: 0xff803f18; ESI 0x808088f9; EDI: 0xff803fec
00191929383i[      ]   EFLAGS: 0x00000002 (id vip vif ac vm rf nt IOPL=0 of df if tf sf zf af pf cf)
00191929383i[      ] MEM ACCESS[0]: 0x00000000810003e0 (linear) 0x0000001113e0 (physical) RD SIZE: 4
```

Looking even farther back, I want to see:

```s
80801ae0:       65 8b 1d 00 00 00 00    mov    %gs:0x0,%ebx
80801ae7:       8b 5b 04                mov    0x4(%ebx),%ebx
```

But for that, I need to get before the call to `IsrHandler()`.

So, I am able to identify that the last push onto the stack is:

```
00191929423i[      ] CPU 3 at 0x80801ac0: push eax   (reg results):
00191929423i[      ] LEN 1	BYTES: 50
00191929423i[      ]   EAX: 0x00000010; EBX: 0x90000250; ECX 0x01001000; EDX: 0x00000000
00191929423i[      ]   ESP: 0xff80aed8; EBP: 0xff803f18; ESI 0x808088f9; EDI: 0xff803fec
00191929423i[      ]   EFLAGS: 0x00000086 (id vip vif ac vm rf nt IOPL=0 of df if tf SF zf af PF cf)
00191929423i[      ] MEM ACCESS[0]: 0x00000000ff80aed8 (linear) 0x000001028ed8 (physical) WR SIZE: 4
```

This tells me that the stack is at `0xff80aed8`.  This is compared to `0xff80af24` from before the fault.  So, I do have a problem with the stack -- still trying to determine what.

---

### 2020-Mar-23

Picking back up where I left off....

Looking back through this, I notice that the value in `ebx` is repeating itself when I do not think it should: `0x90000250`.

From the Bochs debugger, this looks reasonable (after the crash):

```
<bochs:2> set $cpu=0
<bochs:3> x gs:4
[bochs]:
0x00000000810001f4 <bogus+       0>:    0x90000098
<bochs:4> set $cpu=1
<bochs:5> x gs:4
[bochs]:
0x0000000081000298 <bogus+       0>:    0x900001f8
<bochs:6> set $cpu=2
<bochs:7> x gs:4
[bochs]:
0x000000008100033c <bogus+       0>:    0x900002a8
<bochs:8> set $cpu=3
<bochs:9> x gs:4
[bochs]:
0x00000000810003e0 <bogus+       0>:    0x90000250
```

So, let's go back and see what value get pulled from those places (I am expecting several to be `0x00000000`)....

Well, I only find 1 location where `currentThread` is being updated, and that is for CPU2.  This may have some promise to research.

And, sure enough, the following `CurrentThreadAssign(next)` calls were missing:

```c++
    if (next != NULL) {
#if DEBUG_ENABLED(ProcessSchedule)
        kprintf(".. switching to the new task at %p on CPU%d\n", next, thisCpu->cpuNum);
#endif
        ProcessListRemove(next);
        CurrentThreadAssign(next);      // <-- here
        ProcessSwitch(next);
    } else if (currentThread->status == PROC_RUNNING) {
#if DEBUG_ENABLED(ProcessSchedule)
        kprintf(".. this is the only running taskin CPU%d\n", thisCpu->cpuNum);
#endif
        // -- Do nothing; the current process can continue; reset quantum
        AtomicAdd(&currentThread->quantumLeft, currentThread->priority);
        return;
    } else {
#if DEBUG_ENABLED(ProcessSchedule)
        kprintf(".. no task available to run on CPU%d; idling\n", thisCpu->cpuNum);
#endif
        next = thisCpu->idleProcess;
        CurrentThreadAssign(next);      // <-- and, here
        ProcessSwitch(next);
    }
```

So, now I need to re-run the test.  This takes about an hour given all the logs I am capturing.  That's if it fails!

Running this through QEMU first, this is working quite well now.  I am not getting any errors (after about 2 min, which never happened).  And same after about 10 minutes.  OK, so let me stop some of this debugging code and try again.

This is working better now (CPUs 1, 2, & 3 get something to do; CPU0 is bored).  But when I reinstate the `Sleep()` function in `kInitAp()`, the cores deadlock.  Well, I think...  did I wait long enough??

Well, maybe not solved...  back to a `#GP`!!!  Grrrr!!!

OK, so as long as only 1 of the 3 processes is sleeping, things work.  Also, I notice that CPU0 and CPU2 (in this last execution) were not getting interrupts.  So, this may be a consideration from `ProcessStart()`, although I am not starting any processes again at the moment.  So, that is not it... at least yet.  I bookmarked it with a TODO tag.

So, if I take out all the `Sleep()` calls, all 4 CPUs print properly.  When I add that back into the `kInit()` function, both CPU0 and CPU2 stop (in QEMU).

This time I got the same ol' `#GP` fault on CPU2 (and this time it stopped the other CPUs properly -- wondering if I am not halting the other CPUs properly yet...):

```
[CPU2]: General Protection Fault
Entered _LApicBroadcastIpi on CPU 2 for dev 0x81000080
.. Qualified on CPU 2
.. The ESR report 0x00000000
.. Delivery status reports 0x000c00f0
.. Completed on CPU 2
[CPU 2]: EAX: 0x00000019  EBX: 0x90000040  ECX: 0xff80af24
[CPU 2]: EDX: 0x00000000  ESI: 0x80808909  EDI: 0xff803fec
[CPU 2]: EBP: 0xff803f18  ESP: 0xff80af08  SS: 0x10
[CPU 2]: EIP: 0x80801b56  EFLAGS: 0x00000082
[CPU 2]: CS: 0x8  DS: 0x28  ES: 0x28  FS: 0x28  GS: 0x78
[CPU 2]: CR0: 0xe0000011  CR2: 0x00000000  CR3: 0x01001000
[CPU 2]: Trap: 0xd  Error: 0x18
```

OK, back to Bochs....  And..., it's working!!!  God I hate this!

This could be a problem!!:

```
Locking scheduler on CPU0
*(1);*(2);*(3);*(0);*(1);*(2);*(3);*(0);*(1);*(2);*(3);*(0);*(1);.(2);*(3);*(0);*(1);*(2);*(3);*(0);*(1);*(2);*(3);*(0);*(1);*(2);*(3);*(0);*(1);*(2);*(3);*(0);*(1);*(2);*(3);*(0);*(1);*(2);*(3);*(0);*(1);*(2);*(3);*(0);.(1);*(2);.(3);*(0);*(1);*(2);*(3);*(0);*(1);*(2);*(3);*(0);*(1);*(2);*(3);*(0);*(1);*(2);*(3);*(0);*(1);*(2);*(3);*(0);*(1);*(2);*(3);*(0);*(1);*(2);*(3);[CPU0]: Preparing to sleep....
*(1);*(2);*(3);Unlocking scheduler on CPU0
```

After locking the scheduler on CPU0, timer interrupts are still firing (the `*(0);` bits)!

---

### 2020-Mar-24

Yesterday, I traced all the code from `Locking scheduler on CPU0` to `Unlocking scheduler on CPU0` and I could not find where interrupts had be enabled again.  So, I am not certain from where the `*(0);` bits are being written.  Now, there is this small area in here:

```c++
EXPORT INLINE
void ProcessLockAndPostpone(void) {
    kprintf("Locking scheduler on CPU%d\n", thisCpu->cpuNum);
    ProcessLockScheduler();
    AtomicInc(&thisCpu->postponeCount);
}
```

... between the `kprintf()` and the actual lock in `ProcessLockScheduler()` where interrupts could really fire.  So, this might be a race.  So, with a quick change to report the successful lock, I am tring again....

And this looks better:

```
Locking scheduler on CPU0
*(1);*(2);*(3);*(0);*(1);*(2);*(3);*(0);*(1);*(2);*(3);*(0);*(1);.(2);*(3);*(0);*(1);*(2);*(3);*(0);*(1);*(2);*(3);*(0);*(1);*(2);*(3);*(0);*(1);*(2);*(3);*(0);*(1);*(2);*(3);*(0);*(1);*(2);*(3);*(0);*(1);*(2);*(3);*(0);.(1);*(2);*(3);*(0);*(1);*(2);.(3);*(0);*(1);*(2);*(3);*(0);*(1);*(2);*(3);*(0);*(1);*(2);*(3);*(0);*(1);*(2);*(3);*(0);*(1);*(2);*(3);*(0);*(1);*(2);*(3);... CPU0 locked
*(1);*(2);*(3);[CPU0]: Preparing to sleep....
*(1);*(2);*(3);Unlocking scheduler on CPU0
```

OK, several of the process addresses:
* `kInit()` process from `ProcessInit()`: `0x90000040`
* Idle process for CPU0: `0x90000098`
* Idle process for CPU1: `0x900000f0`
* Idle process for CPU2: `0x90000148`
* Idle process for CPU3: `0x900001a0`
* `kInitAp()` process for CPU1: `0x900001f8`
* `kInitAp()` process for CPU2: `0x90000250`
* `kInitAp()` process for CPU3: `0x900002a8`

Now, the thought occurs to me that I am actually trying to put the idle processes on the Ready queue.  Let me put some code in to check that.

---

I did figure out that I am replacing `currentThread` twice -- once before calling `ProcessSwitch()`, and once again within `ProcessSwitch()` after I save the rest of the state to that stack.  I commented the updates out in `ProcessSchedule()` and I did then get an assert failure trying to ready a CPU idle process.  But, I am still having other issues.

It looks like I do have a problem with voluntarily rescheduling....  I really need to work on a table of what happens on the way in and out.

OK, so I get a fault on CPU3:

```
[CPU3]: General Protection Fault
[CPU 3]: EAX: 0x00000019  EBX: 0x90000250  ECX: 0xff80af24
[CPU 3]: EDX: 0x00000000  ESI: 0x80807c65  EDI: 0xff803fec
[CPU 3]: EBP: 0xff803f18  ESP: 0xff80af08  SS: 0x10
[CPU 3]: EIP: 0x80801b46  EFLAGS: 0x00010086
[CPU 3]: CS: 0x8  DS: 0x28  ES: 0x28  FS: 0x28  GS: 0x90
[CPU 3]: CR0: 0xe0000011  CR2: 0x00000000  CR3: 0x01001000
[CPU 3]: Trap: 0xd  Error: 0x18
```

And, the steps leading to this point are:

```
Preparing to schedule a new task on CPU3...
... postponed!
Preparing to schedule a new task on CPU3...
..  getting a new process to run on CPU3...
..  resulting process address 0x90000250
Sanity Checking process 0x90000250 (CPU = 3; stack = 0xff803000; tos = 0xff80af24)
Stack Dump:
... [CPU3] 0: (0xff80af24) 0x00000000
... [CPU3] 1: (0xff80af28) 0x90000040
... [CPU3] 2: (0xff80af2c) 0x00000019
... [CPU3] 3: (0xff80af30) 0xff801004
... [CPU3] 4: (0xff80af34) 0x80803755
... [CPU3] 5: (0xff80af38) 0x90000040
... [CPU3] 6: (0xff80af3c) 0x90000040
... [CPU3] 7: (0xff80af40) 0x00000002
... [CPU3] 8: (0xff80af44) 0x00000027
... [CPU3] 9: (0xff80af48) 0x81000a94
... [CPU3] 10: (0xff80af4c) 0xffffffff
... [CPU3] 11: (0xff80af50) 0xffffffff
... [CPU3] 12: (0xff80af54) 0x8080416e
... [CPU3] 13: (0xff80af58) 0x00000000
... [CPU3] 14: (0xff80af5c) 0x00000000
... [CPU3] 15: (0xff80af60) 0x000c63e0
... [CPU3] 16: (0xff80af64) 0x000c8708
... [CPU3] 17: (0xff80af68) 0x00000000
... [CPU3] 18: (0xff80af6c) 0x00000000
... [CPU3] 19: (0xff80af70) 0x00000000
... [CPU3] 20: (0xff80af74) 0xff80b000
... [CPU3] 21: (0xff80af78) 0x80807c65
... [CPU3] 22: (0xff80af7c) 0xff803fec
... [CPU3] 23: (0xff80af80) 0xff803f18
... [CPU3] 24: (0xff80af84) 0x80801c7c
.. switching to the new task at 0x90000250 on CPU3
[CPU3] Attempting to ready process 0x900002a8
[CPU3] Process Ready
```

So, the code is switching from process `0x900002a8` (`kInitAp()` for CPU3) to process `0x90000250` (`kInitAp()` for CPU2).  I am not certain that this is correct.

Now, the faulting address is `0x80801b46`, which is in `IsrCommonStub()` and is after the call to `IsrHandler()`, assuming that the process change takes place as part of `TimerCallBack()`, which I would need to verify.  Now, the one thing I am starting to see as a pattern is that there is a problem when the process changes CPUs.

Ultimately, I need to trace the path of process `0x90000250`.  This process was preempted from CPU2.

Finally, before I call this a night, I want to capture the exception in the instrumentation logs.

```
00181798993e[CPU3  ] load_seg_reg(FS, 0x0019): RPL & CPL must be <= DPL
00181798993i[      ] CPU 3: exception 0dh error_code=18
00181798993i[CPU3  ] CPU is in protected mode (active)
00181798993i[CPU3  ] CS.mode = 32 bit
00181798993i[CPU3  ] SS.mode = 32 bit
00181798993i[CPU3  ] EFER   = 0x00000000
00181798993i[CPU3  ] | EAX=00000019  EBX=90000250  ECX=ff80af24  EDX=00000000
00181798993i[CPU3  ] | ESP=ff80af30  EBP=ff803f18  ESI=80807c65  EDI=ff803fec
00181798993i[CPU3  ] | IOPL=0 id vip vif ac vm rf nt of df if tf SF zf af PF cf
00181798993i[CPU3  ] | SEG sltr(index|ti|rpl)     base    limit G D
00181798993i[CPU3  ] |  CS:0008( 0001| 0|  0) 00000000 ffffffff 1 1
00181798993i[CPU3  ] |  DS:0028( 0005| 0|  0) 00000000 ffffffff 1 1
00181798993i[CPU3  ] |  SS:0010( 0002| 0|  0) 00000000 ffffffff 1 1
00181798993i[CPU3  ] |  ES:0028( 0005| 0|  0) 00000000 ffffffff 1 1
00181798993i[CPU3  ] |  FS:0028( 0005| 0|  0) 00000000 ffffffff 1 1
00181798993i[CPU3  ] |  GS:0090( 0012| 0|  0) 810003dc 00000007 0 1
00181798993i[CPU3  ] | EIP=80801b48 (80801b46)
00181798993i[CPU3  ] | CR0=0xe0000011 CR2=0x00000000
00181798993i[CPU3  ] | CR3=0x01001000 CR4=0x00000000
```

So, tomorrow I will trace the stack on the way in and on the way back out.

---

### 2020-Mar-25

Today I need to populate the following table, starting with the call to `ProcessSwitch()`:

| Stack Locn | Value      | Source Reg | Pgm Counter | Instruction | Retired By |
|:-----------|:-----------|:-----------|:------------|:------------|:-----------|
| 0xff80affc |
| 0xff80aff8 |
| 0xff80aff4 |
| 0xff80aff0 |
| 0xff80afec |
| 0xff80afe8 |
| 0xff80afe4 |
| 0xff80afe0 |
| 0xff80afdc |
| 0xff80afd8 |
| 0xff80afd4 |
| 0xff80afd0 |
| 0xff80afcc |
| 0xff80afc8 |
| 0xff80afc4 |
| 0xff80afc0 |
| 0xff80afbc |
| 0xff80afb8 |
| 0xff80afb4 |
| 0xff80afb0 |
| 0xff80afac |
| 0xff80afa8 |
| 0xff80afa4 | 0x80801b2e |            |             |          | ret @ 0x80801c7f |
| 0xff80afa0 |            |            |             |          | add esp, 0x0000001c @ 0x80801c7c |
| 0xff80af9c |            |            |             |          | add esp, 0x0000001c @ 0x80801c7c |
| 0xff80af98 |            |            |             |          | add esp, 0x0000001c @ 0x80801c7c |
| 0xff80af94 |            |            |             |          | add esp, 0x0000001c @ 0x80801c7c |
| 0xff80af90 |            |            |             |          | add esp, 0x0000001c @ 0x80801c7c |
| 0xff80af8c |            |            |             |          | add esp, 0x0000001c @ 0x80801c7c |
| 0xff80af88 |            |            |             |          | add esp, 0x0000001c @ 0x80801c7c |
| 0xff80af84 | 0x80801c7c |            |             |          | ret @ 0x80803d0f |
| 0xff80af80 | 0xff803f18 |            |             |          | pop ebp @ 0x808040bd |
| 0xff80af7c | 0xff803fec |            |             |          | pop edi @ 0x808040bc |
| 0xff80af78 | 0x80807c65 |            |             |          | pop esi @ 0x808040bb |
| 0xff80af74 | 0xff80b000 |            |             |          | pop ebx @ 0x808040ba |
| 0xff80af70 |            |            |             |          | add esp,0x1c @ 0x808040b7 |
| 0xff80af6c |            |            |             |          | add esp,0x1c @ 0x808040b7 |
| 0xff80af68 |            |            |             |          | add esp,0x1c @ 0x808040b7 |
| 0xff80af64 |            |            |             |          | add esp,0x1c @ 0x808040b7 |
| 0xff80af60 |            |            |             |          | add esp,0x1c @ 0x808040b7 |
| 0xff80af5c |            |            |             |          | add esp,0x1c @ 0x808040b7 |
| 0xff80af58 |            |            |             |          | add esp,0x1c @ 0x808040b7 |
| 0xff80af54 | 0x8080416e |            |             |          | ret @ 0x8080375b |
| 0xff80af50 | 0xffffffff |            |             |          | pop edi @ 0x8080375a |
| 0xff80af4c | 0xffffffff |            |             |          | pop esi @ 0x80803759 |
| 0xff80af48 | 0x81000a94 |            |             |          | pop ebx @ 0x80803758 |
| 0xff80af44 | 0x00000027 |    eax     | 0x80803730  | push eax | add esp,0x00000010 @ 0x80803755 |
| 0xff80af40 | 0x00000002 |    edx     | 0x80803739  | push edx | add esp,0x00000010 @ 0x80803755 |
| 0xff80af3c | 0x90000040 |    edi     | 0x8080373a  | push edi | add esp,0x00000010 @ 0x80803755 |
| 0xff80af38 | 0x90000040 |    edi     | 0x8080374d  | mov dword ptr ss:[esp], edi | add esp,0x00000010 @ 0x80803755 |
| 0xff80af34 | 0x80803755 |    eip     | 0x80803750  | call .+267 (call ProcessSwitch) | ret @ 0x808038c0 |
| 0xff80af30 | 0xff801004 |    ebx     | 0x80803874  | push ebx | pop ebx [CPU3] @ 0x808038bf |
| 0xff80af2c | 0x00000019 |    esi     | 0x80803875  | push esi | pop esi [CPU3] @ 0x808038be |
| 0xff80af28 | 0x90000040 |    edi     | 0x80803876  | push edi | pop edi [CPU3] @ 0x808038bd |
| 0xff80af24 | 0x00000000 |    ebp     | 0x80803877  | push ebp | pop ebp [CPU3] @ 0x808038bc |
| ~~0xff80af20~~ | ~~0x90000250~~ |    ~~esi~~     | ~~0x80803885~~  | ~~push esi~~ | add esp, 0x00000004 |
| ~~0xff80af1c~~ | ~~0x8080388b~~ |    ~~eip~~     | ~~0x80803886~~  | ~~call .-1019 (call ProcessDoReady)~~ | ret |
| ~~0xff80af18~~ | ~~0xff801004~~ |    ~~ebx~~     | ~~0x80803490~~  | ~~push ebx~~ | pop ebx |
| ~~0xff80af14~~ |            |    ~~esp~~     | ~~0x80803491~~  | ~~sub esp,8~~ | add esp, 0x00000008 |
| ~~0xff80af10~~ |            |    ~~esp~~     | ~~0x80803491~~  | ~~sub esp,8~~ | add esp, 0x00000008 |
| ~~0xff80af0c~~ | ~~0x81000a30~~ |    ~~edx~~     | ~~0x808034a7~~  | ~~push edx~~ | add esp, 0x00000010 |
| ~~0xff80af08~~ | ~~0x90000250~~ |    ~~ebx~~     | ~~0x808034a8~~  | ~~push ebx~~ | add esp, 0x00000010 |
| ~~0xff80af04~~ | ~~0x00000002~~ |    ~~ecx~~     | ~~0x808034ab~~  | ~~push ecx~~ | add esp, 0x00000010 |
| ~~0xff80af00~~ | ~~0x80807fa4~~ |            | ~~0x808034ac~~  | ~~push 0x80807fa4~~ | add esp, 0x00000010 |
| ~~0xff80aefc~~ | ~~0x808034b6~~ |    ~~eip~~     | ~~0x808034b1~~  | ~~call .+5002 (call kprintf)~~ | ret @ 0x80804a62 |
| ~~0xff80af0c~~ | ~~0x90000288~~ |    ~~ecx~~     | ~~0x8080355f~~ | ~~push ecx~~ | add esp, 0x00000010 |
| ~~0xff80af08~~ | ~~0x90000288~~ |    ~~ecx~~     | ~~0x80803560~~ | ~~push ecx~~ | add esp, 0x00000010 |
| ~~0xff80af04~~ | ~~0x00000002~~ |    ~~ebx~~     | ~~0x80803563~~ | ~~push ebx~~ | add esp, 0x00000010 |
| ~~0xff80af00~~ | ~~0x80808097~~ |            | ~~0x80803564~~ | ~~push 0x80808097~~ | add esp, 0x00000010 |
| ~~0xff80aefc~~ | ~~0x8080356e~~ |    ~~eip~~     | ~~0x80803569~~ | ~~call .+4818 (call kPrintf)~~ | ret @ 0x80804a62 |
| ~~0xff80af20~~ | ~~0x80803893~~ |    ~~eip~~     | ~~0x8080388e~~ | ~~call .+701 (call ProcessUpdateTimeUsed)~~ | ret @ 0x80803b8f |

OK, I have been able to determine that this is where the stack goes wrong:

```
00181798988i[      ] CPU 3 at 0x80801b41: mov esp, ecx   (reg results):
00181798988i[      ] LEN 2	BYTES: 89cc
00181798988i[      ]   EAX: 0x00000010; EBX: 0x90000250; ECX 0xff80af24; EDX: 0x00000000
00181798988i[      ]   ESP: 0xff80af24; EBP: 0xff803f18; ESI 0x80807c65; EDI: 0xff803fec
00181798988i[      ]   EFLAGS: 0x00000002 (id vip vif ac vm rf nt IOPL=0 of df if tf sf zf af pf cf)
```

Before this point, the stack is `0xff80afa8`.  This is obviously wrong.  And this is about what I expected.

Tracing backwards, the value in `ecx` is wrong.  `ecx` is loaded by the following:

```
00181798983i[      ] CPU 3 at 0x80801b3a: mov ecx, dword ptr ds:[ebx]   (reg results):
00181798983i[      ] LEN 2	BYTES: 8b0b
00181798983i[      ]   EAX: 0x00000000; EBX: 0x90000250; ECX 0xff80af24; EDX: 0x00000000
00181798983i[      ]   ESP: 0xff80afa8; EBP: 0xff803f18; ESI 0x80807c65; EDI: 0xff803fec
00181798983i[      ]   EFLAGS: 0x00000002 (id vip vif ac vm rf nt IOPL=0 of df if tf sf zf af pf cf)
00181798983i[      ] MEM ACCESS[0]: 0x0000000090000250 (linear) 0x000001011250 (physical) RD SIZE: 4
```

This is loaded from the top of the process block at `0x90000250`.  So where is that memory location set prior to this point in execution?

```
00181183843i[      ] CPU 2 at 0x80803893: mov dword ptr ds:[esi], esp   (reg results):
00181183843i[      ] LEN 2	BYTES: 8926
00181183843i[      ]   EAX: 0x90000250; EBX: 0xff801004; ECX 0x00017700; EDX: 0x00000000
00181183843i[      ]   ESP: 0xff80af24; EBP: 0x00000000; ESI 0x90000250; EDI: 0x90000040
00181183843i[      ]   EFLAGS: 0x00000002 (id vip vif ac vm rf nt IOPL=0 of df if tf sf zf af pf cf)
00181183843i[      ] MEM ACCESS[0]: 0x0000000090000250 (linear) 0x000001011250 (physical) WR SIZE: 4
```

This code is storing the value of the stack into the top of the process block at `0x90000250`.  `esp` is the value I am getting back, so nothing is corrupting this.

OK, so this is interesting!!:

```
00181183843i[      ] MEM ACCESS[0]: 0x0000000090000250 (linear) 0x000001011250 (physical) WR SIZE: 4
00181280143i[      ] MEM ACCESS[0]: 0x0000000090000250 (linear) 0x000001011250 (physical) RD SIZE: 4
00181320393i[      ] MEM ACCESS[0]: 0x0000000090000250 (linear) 0x000001011250 (physical) RD SIZE: 4
00181798928i[      ] MEM ACCESS[0]: 0x0000000090000250 (linear) 0x000001011250 (physical) RD SIZE: 4
00181798983i[      ] MEM ACCESS[0]: 0x0000000090000250 (linear) 0x000001011250 (physical) RD SIZE: 4
```

The top line is where the stack is stored.  This value is correct for the time.

Now, at timestamp `00181280143i` , the following is taking place:

```
00181280143i[      ] CPU 3 at 0x808036bf: mov eax, dword ptr ds:[edi]   (reg results):
00181280143i[      ] LEN 2	BYTES: 8b07
00181280143i[      ]   EAX: 0xff80af24; EBX: 0x80808199; ECX 0x00000029; EDX: 0x00000002
00181280143i[      ]   ESP: 0xff80bf3c; EBP: 0x00000000; ESI 0x00000000; EDI: 0x90000250
00181280143i[      ]   EFLAGS: 0x00000002 (id vip vif ac vm rf nt IOPL=0 of df if tf sf zf af pf cf)
00181280143i[      ] MEM ACCESS[0]: 0x0000000090000250 (linear) 0x000001011250 (physical) RD SIZE: 4
```

This seems safe enough because `eax` is pushed on the stack.

The next timestamp has the following:

```
00181320393i[      ] CPU 3 at 0x808036e7: mov ebx, dword ptr ds:[edi]   (reg results):
00181320393i[      ] LEN 2	BYTES: 8b1f
00181320393i[      ]   EAX: 0x0000000c; EBX: 0xff80af24; ECX 0x0000000c; EDX: 0x00000002
00181320393i[      ]   ESP: 0xff80bf38; EBP: 0x00000000; ESI 0x00000000; EDI: 0x90000250
00181320393i[      ]   EFLAGS: 0x00000002 (id vip vif ac vm rf nt IOPL=0 of df if tf sf zf af pf cf)
00181320393i[      ] MEM ACCESS[0]: 0x0000000090000250 (linear) 0x000001011250 (physical) RD SIZE: 4
```

Again, this seems safe enough since the stack is not updated after this read.

The final intermediate read has the following:

```
00181798928i[      ] CPU 3 at 0x808038a0: mov esp, dword ptr ds:[edi]   (reg results):
00181798928i[      ] LEN 2	BYTES: 8b27
00181798928i[      ]   EAX: 0x900002a8; EBX: 0xff80af88; ECX 0x0000fa00; EDX: 0x00000000
00181798928i[      ]   ESP: 0xff80af24; EBP: 0x00000000; ESI 0x900002a8; EDI: 0x90000250
00181798928i[      ]   EFLAGS: 0x00000002 (id vip vif ac vm rf nt IOPL=0 of df if tf sf zf af pf cf)
00181798928i[      ] MEM ACCESS[0]: 0x0000000090000250 (linear) 0x000001011250 (physical) RD SIZE: 4
```

Now, this is a mess.  It is restoring the stack.  But this is the correct restoring the stack (from `ProcessSwitch()`).

So, the last read is this one:

```
00181798983i[      ] CPU 3 at 0x80801b3a: mov ecx, dword ptr ds:[ebx]   (reg results):
00181798983i[      ] LEN 2	BYTES: 8b0b
00181798983i[      ]   EAX: 0x00000000; EBX: 0x90000250; ECX 0xff80af24; EDX: 0x00000000
00181798983i[      ]   ESP: 0xff80afa8; EBP: 0xff803f18; ESI 0x80807c65; EDI: 0xff803fec
00181798983i[      ]   EFLAGS: 0x00000002 (id vip vif ac vm rf nt IOPL=0 of df if tf sf zf af pf cf)
00181798983i[      ] MEM ACCESS[0]: 0x0000000090000250 (linear) 0x000001011250 (physical) RD SIZE: 4
```

And this is from `IsrCommonStub()`.  This is also reading and updating the stack from the same value.

I think the problem is quite simple now: `ProcessSwitch()` and `IsrCommonStub()` are both trying to restore the stack, and one was a preempted stack while the other was a voluntary stack yield.  Moreover, they are the same stack!

---

### 2020-Mar-26

So, the following on `freenode#osdev` helped:

```
[16:18:21] <eryjus> heat, are you still around?  and if you are, are you still interested in my problem?
[16:18:34] <heat> eryjus: yeah, sure :)
[16:19:12] <heat> geist: not really, I wasn't trolling
[16:20:28] <eryjus> let's see how well I can describe it...  i walked away after posting yesterday, so this is a good test of my own understanding..  i was having problems with what appeared to be a stack symmetry problem....
[16:20:53] <geist> heat: ok boomer
[16:21:33] <heat> ɹǝɯooq ʞo
[16:21:57] <eryjus> The what of the problem ended up being that a process would block on CPUx and resume on CPUy -- where each CPU had its own stack for handling interrupts.
[16:22:07] <kingoffrance> i just mean, c++ might take a little to wrap around, but if the alternative is special coccinelle "transformation patches" to achieve similar things by other means, which is more legible? :)
[16:23:32] <eryjus> well, when a process gives up the cpu voluntarily (such as sleep) on one CPU, and resume on another CPU the stack was being reset improperly
[16:23:49] <geist> eryjus: usually that sort of scheme the stack is per thread, not per cpu
[16:24:09] <geist> that way the irq can enter on one cpu, then get blocked, then resumed (and iretted) on another
[16:24:16] <geist> which on most architectures is totally legit
[16:24:23] <eryjus> geist, stack per thread for interrupts and for normal work?
[16:24:55] <geist> yes. same stack
[16:25:09] <geist> as in the irq just comes along and uses the kernel stack for the thread for irq work
[16:25:12] <eryjus> meaning when a process in interrupted by IRQ, each process has its own additional stack for that work?
[16:25:13] <geist> thats a fairly standard design
[16:25:47] <geist> for a standard design each thread in the system has a user stack and a kernel stack. the kernel stack is usually fairly small but fixed in size, allocated up front
[16:25:59] <geist> irqs happen in the contxt of whatever kernel stack happened to be active at the time
[16:26:09] <geist> ie, whatever thread was running. *including* the idle thread
[16:26:33] <geist> if the thread was in user space at the time then the cpu switches to the thread's corresponding kernel stack before issuing the irq
[16:26:55] <geist> if the thread was in kernel space at the time and the kernel is reentrant, then it just stacks right on top of the kernel stack
[16:27:22] <geist> which is incidentally why you must compile x86-64 kernel code with -mno-red-zone, because of this 'irq at any time blatting more state on the stack' issue
[16:27:45] <eryjus> i see
[16:28:25] <geist> what i described is a fairly standard multithreading, preemptive kernel stack system. works for SMP too
[16:28:48] <geist> also is pretty universal for all architectures, since pretty much any sane architecture can do this without much work
[16:28:49] <eryjus> i think must of that is much farther over the horizon than where I am at for the moment -- still working on getting SMP right
[16:29:06] <eryjus> it's only been about 8 or 9 months...  smh
[16:29:10] <geist> right, but this is why you should do SMP first. it basically immediately uncovers assumptions about exactly this stuff
[16:29:19] <geist> thats fine. took me a few years before i got my first os to that level
[16:30:42] <eryjus> OK, so each process has its own user-space stack (or process stack?) and a separate kernel stack...  that's easy enough to work on
[16:31:04] <eryjus> im not back to user mode again yet
[16:31:31] <heat> yes
[16:31:41] <heat> it also lets you preempt in kernel space
[16:31:54] <heat> which is Cool(tm)
[16:32:06] <eryjus> rephrase: is that a user stack in user-space or just a process stack?
[16:32:41] <heat> each user thread has a user-space stack and a separate kernel stack in the kernel's address space
[16:33:07] <heat> so above 0xC0000000 or 0xffff800000000000 or whatever
[16:33:23] * eryjus nods
[16:34:55] <eryjus> so, anyway heat, I had been looking for the actual reason for my stack symmetry problems for about 3 weeks.  At least I was able to explain the 'what' after yesterday.
[16:35:06] <eryjus> and now I have the 'why'
[16:35:11] <eryjus> so, thanks again
```

So, I have a decision to make: *Do I roll back my changes? or do I press forward with the current state?*

There are too many ancillary changes to lose.  I need to press forward with this version.

---

After creating a new branch (`R458-a`), I am going to start by eliminating the per-CPU stack.

Well, going back to the top of this journal, I can see that I may be better off abandoning my changes and reverting back to `v0.5.1a`.  I may be able to pick up several files that would makes sense to save.

---

OK, back on git branch `v0.5.1a`.  I will be able to delete branch `R458-a`.

I have the problem duplicated on both archs again.  But, it's not consistent.  So, this is starting to feel like a race condition.

I was able to narrow this down to the following line:

```c++
    scheduler.currentProcess->wakeAtMicros = when;
```

Where `scheduler.currentProcess` is NULL.  Adding an `assert`, I get the following:

```
!!! ASSERT FAILURE !!!
/home/adam/workspace/century-os/modules/kernel/src/process/ProcessMicroSleepUntil.cc(30) scheduler.currentProcess != NULL scheduler.currentProcess is NULL


Page Fault
Flushing TLB on CPU 3
CPU: 0
EAX: 0x00000000  EBX: 0xffffffff  ECX: 0xffffffff
EDX: 0x00000000  ESI: 0x0047bf30  EDI: 0x00000000
EBP: 0x00000003  ESP: 0xff800f2c  SS: 0x10
EIP: 0x808031fd  EFLAGS: 0x00200013
CS: 0x8  DS: 0x28  ES: 0x28  FS: 0x28  GS: 0x48
CR0: 0xe0000011  CR2: 0x0000002c  CR3: 0x01001000
Trap: 0xe  Error: 0x2
```

So, how the hell can that happen??  This function can only be called from a running process (as in not from the `TimerCallBack()` function or any IPI).  Another problem is that this only happens about 1 every 10 executions (qemu), so duplicating this and getting an execution log is going to be rough.

OK, I have it captured in the logs.  However, I think my problem is going to be that I have 1 central global `currentProcess` and I need to redo the work to have a `currentProcess` per CPU.

---

### 2020-Mar-27

Today I am starting by renaming the `scheduler.currentProcess` member and replacing it with `currentThread` member.

After cleaning that up, I am still getting a `#PF`.  I believe that this is related to not having an idle process (instead the CPU is on the previous process stack waiting for something to do when an interrupt comes in).  In particular, I think this was my problem before:
* Process A blocks on CPU0
* CPU0 goes into an idle state, enabling interrupts to wait for a timer tick to check for something to do
* While CPU0 is idle, Process A is allowed to schedule on some other CPUn
* With the same stack in place on CPU0, an interrupt fires

Ultimately, I need an idle process for each CPU, so that I have a good stack for each CPU when a process is scheduled on another CPU.  The easiest thing to do is to create `cpus.discoveredCpus` idle process and ready those processes.  I should be able to do that in `ProcessInit()`.

So, even with those changes, I am still getting a `#PF`:

```
Page Fault
CPU: 0
EAX: 0x90000094  EBX: 0xff800f8c  ECX: 0x810009c0
EDX: 0x9000011c  ESI: 0x00000000  EDI: 0xff800f8c
EBP: 0x80807f00  ESP: 0xff800e1c  SS: 0x10
EIP: 0x808036fe  EFLAGS: 0x00200046
CS: 0x8  DS: 0x28  ES: 0x28  FS: 0x28  GS: 0x48
CR0: 0xe0000011  CR2: 0x00000008  CR3: 0x01001000
Trap: 0xe  Error: 0x0
```

OK, this is from this section of `ProcessSwitch()`:

```s
808036f4 <ProcessSwitch.cont>:
808036f4:       53                      push   %ebx
808036f5:       56                      push   %esi
808036f6:       57                      push   %edi
808036f7:       55                      push   %ebp
808036f8:       8b 35 a0 09 00 81       mov    0x810009a0,%esi
808036fe:       83 7e 08 01             cmpl   $0x1,0x8(%esi)
```

And this means that I am not initializing `currentThread` properly.

Aw CRAP!:

```s
;;
;; -- Get the current task structure
;;    ------------------------------
        mov     esi,[scheduler+SCH_CURRENT_PROCESS]         ;; get the address of the current process

        cmp     dword [esi+PROC_STATUS],PROC_STS_RUNNING    ;; is this the current running process
        jne     .saveStack
```

Now, I am deadlocking!  However, I am deadlocking starting the other cores.

I have the same problem on rpi2b.  Time for the bochs instrumentation.

My first test shows that the CPU is halted with interrupts disabled:

```
eflags 0x00200086: ID vip vif ac vm rf nt IOPL=0 of df if tf SF zf af PF cf
```

Clearing that up, I now have a virtual memory problem:

```
<bochs:5> creg
CR0=0xe0000011: PG CD NW ac wp ne ET ts em mp PE
CR2=page fault laddr=0x00000000ff401040
CR3=0x000000000000
    PCD=page-level cache disable=0
    PWT=page-level write-through=0
CR4=0x00000000: cet pke smap smep osxsave pcid fsgsbase smx vmx osxmmexcpt umip osfxsr pce pge mce pae pse de tsd pvi vme
CR8: 0x0
EFER=0x00000000: ffxsr nxe lma lme sce
```

That was an easy fix.  And now, I have a problem in `TimerCallBack()`:

```
Page Fault
CPU: 0
EAX: 0x00000000  EBX: 0x00000000  ECX: 0x81000a30
EDX: 0x810009fc  ESI: 0x90000074  EDI: 0x00000000
EBP: 0x000445c0  ESP: 0xff801f18  SS: 0x10
EIP: 0x80803ddb  EFLAGS: 0x00000082
CS: 0x8  DS: 0x28  ES: 0x28  FS: 0x28  GS: 0x48
CR0: 0xe0000011  CR2: 0x00000004  CR3: 0x01001000
Trap: 0xe  Error: 0x2
```

Hmmm.....

```
<bochs:3> creg
CR0=0xe0000011: PG CD NW ac wp ne ET ts em mp PE
CR2=page fault laddr=0x00000000ff401040
CR3=0x0000810009e0
    PCD=page-level cache disable=0
    PWT=page-level write-through=0
CR4=0x00000000: cet pke smap smep osxsave pcid fsgsbase smx vmx osxmmexcpt umip osfxsr pce pge mce pae pse de tsd pvi vme
CR8: 0x0
EFER=0x00000000: ffxsr nxe lma lme sce
```

`cr3` is odd...

OK, turns out I had the `scheduler` initialization wrong.  When I added an idle queue, I forgot to initialize it properly.

And with that, it all works.  Processes are bouncing around all the CPUs:

> .(1);B(2);B(0);A(3);B(2);B(1);A(0);B(0);B(1);A(3);B(3);B(1);A(0);.(0);B(2);B(0);A(1);B(3);B(1);A(0);B(0);B(0);A(1);B(1);B(3);A(2);.(1);B(3);B(2);A(1);B(2);B(1);A(0);B(2);B(0);A(1);B(3);B(1);A(0);.(0);B(2);B(3);A(2);B(2);B(2);A(0);B(0);B(1);A(3);B(3);B(0);.(1);A(1);B(2);B(1);A(1);B(3);B(3);A(0);B(0);B(1);A(2);B(0);B(0);.(0);A(0);B(2);B(3);A(1);B(3);B(0);A(0);B(2);B(2);A(1);B(1);.(2);B(2);A(3);B(3);B(3);A(3);B(1);B(3);A(2);B(2);B(1);A(2);B(2);.(1);B(1);A(3);B(2);B(0);A(2);B(0);B(3);A(1);

---

After staging the changes, I am going to remove the unused member of `scheduler`.

And I am running out of things to do on rpi2b only:

```
FATAL: CPU0: Enabling interrupts on CPU 1

Nothing available to schedule!!

At address: 0xff800f18
 R0: 0x808066fc   R1: 0x00000000   R2: 0x808066fa
 R3: 0x00000020   R4: 0x81000498   R5: 0x00000001
 R6: 0x80804804   R7: 0x808047e0   R8: 0x001006e8
 R9: 0xf900009c  R10: 0x00000024  R11: 0x01007000
R12: 0x90000040   SP: 0x8080338c   LR_ret: 0x8080338c
SPSR_ret: 0x000001d3     type: 0x8080338c

Additional Data Points:
User LR: 0xfffb56bf  User SP: 0xdde5ead3
Svc LR: 0xff800f28
Caught signal 2; exiting
```

However, there should be several things to schedule.

---

### 2020-Mar-28

OK, I have to get to the bottom of why the scheduler is not able to find anything to run, when there is clearly something available to run (4 Idle processes).

---

Actually, with some debugging code, I see that I am "consuming" the ready processes:

```
Dumping the status of the scheduler
The scheduler is locked
..     OS Queue process count: 4
..   High Queue process count: 0
.. Normal Queue process count: 0
..    Low Queue process count: 0
..   Idle Queue process count: 0
Dumping the status of the scheduler
The scheduler is locked
..     OS Queue process count: 3
..   High Queue process count: 0
.. Normal Queue process count: 0
..    Low Queue process count: 0
..   Idle Queue process count: 0
Enabling interrupts on CPU 1
Preempt
From within ProcessNext():
Dumping the status of the scheduler
The scheduler is locked
..     OS Queue process count: 3
..   High Queue process count: 0
.. Normal Queue process count: 0
..    Low Queue process count: 0
..   Idle Queue process count: 0
Preempt
From within ProcessNext():
Dumping the status of the scheduler
The scheduler is locked
..     OS Queue process count: 2
..   High Queue process count: 0
.. Normal Queue process count: 0
..    Low Queue process count: 0
..   Idle Queue process count: 0
Preempt
From within ProcessNext():
Dumping the status of the scheduler
The scheduler is locked
..     OS Queue process count: 1
..   High Queue process count: 0
.. Normal Queue process count: 0
..    Low Queue process count: 0
..   Idle Queue process count: 0
Preempt
From within ProcessNext():
Dumping the status of the scheduler
The scheduler is locked
..     OS Queue process count: 0
..   High Queue process count: 0
.. Normal Queue process count: 0
..    Low Queue process count: 0
..   Idle Queue process count: 0
FATAL: CPU0:
Nothing available to schedule!!
```

So, the problem really is that the current process is not being placed back on the ready queue.  Interestingly, this works on x86 but not on rpi2b.  I suspect something in `ProcessSwitch()`, but have not found it yet.

So, I have identified that this is not calling `ProcessDoReady()`:

```s
@@
@@ -- Execute a process switch
@@    ------------------------
ProcessSwitch:
@@
@@ -- before we get too crazy, do we need to postpone?
@@    ------------------------------------------------
    ldr     r1,=(scheduler+SCH_POSTPONE_COUNT)  @@ get the locks held address (&scheduler.postponeCount)
    ldr     r1,[r1]                         @@ and the count (scheduler.postponeCount)
    cmp     r1,#0                           @@ is this zero?
    beq     .cont                           @@ if zero, contunue

    ldr     r1,=(scheduler+SCH_CHG_PENDING) @@ get addr of the proc chg pend (&scheduler.processChangePending)
    mov     r0,#1                           @@ load the value to store (value 1)
    str     r0,[r1]                         @@ set the flag
    mov     pc,lr                           @@ and return

.cont:
    push    {lr}                            @@ save the link register; will pop it later to pc
    push    {lr}                            @@ save this value twice
    push    {ip}                            @@ save the intra-procedure call register (r12)
    push    {r4-r11}                        @@ save the rest of the register (r0-r3 are caller saved)


@@
@@ -- Save the state by writing the current stack pointer to the current structure
@@    ----------------------------------------------------------------------------
    mrc     p15,0,r2,c13,c0,3               @@ get the current thread

    ldr     r4,[r2,#PROC_STATUS]            @@ get the status scheduler.currentProcess->status
    cmp     r4,#PROC_STS_RUNNING            @@ is the status running
    bne     .saveStack                      @@ if not, skip the next part

    push    {r0,r1,r2,r3}                   @@ save the clobber registers
    mov     r0,r2                           @@ get the current process (scheduler.currentProcess)
    bl      ProcessDoReady                  @@ go make the current process ready
    pop     {r0,r1,r2,r3}                   @@ restore the vlobber registers

.saveStack:
```

So, what are the reasons this will not be called?
1. The current process is not in the status `PROC_STS_RUNNING` -- I have this bound with an assert.
1. The Postpone Count is not 0 (which is likely)

In the latter case above, the process has been removed from the Ready Queue but is not scheduled.  I had a fix for that which I lost to check if we had a lock and could schedule.

OK, from what I can tell (and that is not much concretely), I have a problem with my spinlocks or atomics.  What I need to do is come up with a way to confirm that they are working properly.  I only have a problem with the rpi2b.  Most of the rest of the code is common to both archs.

hmmm....  so what happens when I don't enable cache???  No difference in behavior.  But, to be fair, I was not able to get the C code to compile when I disabled it.

I think I am going to try to architect a test of the atomics (since they are basically the same logic).

Yup!!  I have an Atomics problem (the value I am expecting is 0).

x86:
```
The restulting value is 180664
```

rpi2b:
```
The restulting value is 4294967294
```

That said, if I have an atomics problem, I have a spinlock problem.  And if I have a spinlock problem, my scheduler is fubar, and my scheduler is fubar.

---

### 2020-Mar-29

Today I worked on the atomics.  Now, when I run the test, the result is 0 as I expect on both archs.  This takes care of [Redmine #448](http://eryjus.ddns.net:3000/issues/448).

Now, I need to work on the spinlock implementation as well.

So, spinlocks are not coming together like I thought they should....  I duplicated the test from yesterday using a spinlock to get the lock before adding/subtracting.  I am not getting good results -- spinlocks are still broken....

x86-pc:
```
The restulting value is 769
```

rpi2b:
```
The restulting value is 4294964453
```

These are not 0.

---

### 2020-Mar-30

Atomics are not working properly yet: 1 good test does not mean problem solved:

```
The resulting int value is 587336
The resulting Atomic val is -1208914
```

I added in some cache management:

```c++
    for (int i = 0; i < 1000000; i ++) {
        InvalidateCache((archsize_t)&atomVal, sizeof(AtomicInt_t));
        if (thisCpu->cpuNum % 2 == 0) {
            AtomicAdd(&atomVal, 1);
        } else {
            AtomicSub(&atomVal, 1);
        }
    }
```

... and it worked for the first 4 or 5 tests..  Then it stopped working!

OK, so it works properly with interrupts disabled.  But not enabled.

```
[19:59:36] <geist> but riscv has about the same capability with the 16bit load instructions so it should be about as good as the arm64 codegen
[20:26:38] <eryjus> Hmmm... atomics still don't work with interrupts enabled....
[20:26:55] * eryjus swears loudly
[20:28:22] <heat> maybe your test's flawed?
[20:30:08] <eryjus> i considered that as I distilled it down to the most basic __atomic* and __sync* tests...
[20:30:28] <eryjus> then i disabled interrupts and bubbled it all back up again
[20:30:34] <Mutabah> what do you mean by "don't work" ?
[20:31:04] <eryjus> mutabah, let me pull a gist together....
[20:39:19] <eryjus> https://gist.github.com/eryjus/8c6cff01d716a0e501e1cb4c11536717
[20:39:56] <eryjus> So, a basic test: iterate 1M times..  even cpus inc; odd dec.  in the end the total should be 0.
[20:39:58] <eryjus> it is not
[20:40:25] <eryjus> well, it is  not with interrupts enabled
[20:42:52] <heat> register corruption?
[20:42:54] <Mutabah> Have you checked your IRQ hanlders
[20:42:55] <Mutabah> ^
[20:44:23] <eryjus> i explicitly push and pop each register in turn..  checking symmetry now
[20:45:19] <eryjus> symmetry looks good...
[20:45:25] <Mutabah> Linky?
[20:45:52] <eryjus> ok, "linky" is a new term...  a little help here.
[20:46:33] <Mutabah> it means "can you link to that?"
[20:47:49] <eryjus> https://github.com/eryjus/century-os/blob/master/arch/x86/IsrCommonStub.s#L177
[20:50:31] <eryjus> not to say that it is not my handler overall...  with interrupts enabled on only 1 core, it apepars to work
[20:50:55] <Mutabah> looks half-decent, hard to be sure with the format there
[20:51:09] <Mutabah> (I use macros to handle those push/pop blocks)
[20:51:45] <Mutabah> hmm... you don't restoer SS
[20:52:40] <eryjus> currently doesn't change.., but I will look at that.
[20:53:39] <eryjus> interrupts disabled: works; enabled on 1 core: works; enables on 2 cores: fails; enabled on 4 cores: fails.
[20:53:46] <eryjus> so, I think you are on to something
[20:55:02] <Mutabah> are the cores sharing stacks?
[20:55:11] <Mutabah> i.e. is SP different
[20:55:44] <eryjus> they are not.
[20:56:11] <Mutabah> Put a breakpoint at the start/end of your ISR and ensure no corruption?
[20:58:31] <eryjus> OK, eliminating the timer handler, it works.  I think I know what I need to look at tomorrow.
[20:58:32] <eryjus> thanks
```

---

### 2020-Mar-31

After my last update, I was able to drill into the Timer interrupt to confirm that it was when `ProcessSchedule()` was called during an interrupt that there was a problem with the atomics.  In fact, it was when there was a preemption.

This morning, I woke up to this message from geist on `freenode#osdev`:

```
[21:17:41] <geist> eryjus: is it possible for the irqs to force a reschedule that migrates the threads?
[21:18:05] <geist> such that 'thisCpu' becomes non true if it gets preempted in the middle of it?
```

So, the plan for today is to take more of an, "If you can't fix it, feature it," approach.  I am going to disable interrupts during each atomic operation.

It turns out that processes were moving between CPUs and therefore the direction of any given increment/decrement instruction was changing based on the CPU it was currently on.  Changing that to collect that information at the start of the process worked better.  At least on x86.  I still have a problem on rpi where I am consuming all my processes:

```
FATAL: CPU0:
Nothing available to schedule!!

At address: 0xff800f18
 R0: 0x80807248   R1: 0x00000000   R2: 0x81000618
 R3: 0x00000000   R4: 0x810004b4   R5: 0x00000000
 R6: 0x80805164   R7: 0x80805140   R8: 0x001006e8
 R9: 0xf900009c  R10: 0x00000001  R11: 0x01007000
R12: 0xf8215000   SP: 0x808038c4   LR_ret: 0x808038c4
SPSR_ret: 0x000001d3     type: 0x808038c4

Additional Data Points:
User LR: 0xafefcdfe  User SP: 0xa7ff6cfd
Svc LR: 0xff800f28
```

Hmmm...  do I have idle processes created for Rpi?  I did.

So, same test as on x86, I did not enable interrupts and got the proper results:

```
Starting processes
The resulting int value is 0
The resulting Atomic val is 0
```

So, something there is also making a mess of the rpi.  Which I believe might be cache coherency.

---

### 2020-Apr-01

So, yesterday, I was able to get the rpi to consistently report `2000000` for each value.  I dawned on me later that evening that I can get to 2M by doing 3M increments and 1M decrements.  A quick change today to have an instance number that is running and use that instead of the CPU number and it reports 0 all the time now.

With that, I am working on cleaning up some of the debugging code so I can commit this jumble.

I think I am getting close.  Both archs are scheduling (rpi about 90% of the runs, so I still have a problem there to find/solve -- but I may not be able to get there until I have a debugger operational).

At this point, I want to clean up a few things and then commit this code (and clean up the other branches that need to be cleaned up).

---

OK, this has been merged and I have started sub-branch `v0.5.1b`.  There are still 12 issues left to complete in Redmine.

I have decided to handle only 3 of them at this point (2 of the 3 are very similar).  I really want to get into the debugger.  So, with that, I am also debating whether I take on my file cleanup effort now or wait and take care of it as I go.  I really want to get into the debugger.  I really need something new to work on!  I think my last task will be to check if this will run on real x86 hardware.  And it does!




