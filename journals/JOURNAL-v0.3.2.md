# The Century OS Development Journal - v0.3.2

## Version 0.3.2

This version will improve the scheduler and process swapping. While things are working at the moment, they are nowhere near correct. Even before getting into this tutorial, I had fears that I had created a monster. So this version will clean all that up.

Using Brandan's Multi-tasking Tutorial (https://wiki.osdev.org/Brendan%27s_Multi-tasking_Tutorial) as a roadmap, this version will complete steps 1-14 and step 16. Step 15 (IPC) will be left for a later release (even though I had that working poorly at one point).

The goals for this version are tracked here: http://eryjus.ddns.net:3000/versions/9

---

### 2019-Mar-14

Happy Pi Day!!

I start this version with the realization that I will be taking a few steps backward in order to take a massive leap forward.  So, with that, the first thing to review is the `Process_t` structure:

```C
typedef struct Process_t {
    archsize_t stackPointer;            // This is the process current esp value (when not executing)
    archsize_t ss;                      // This is the process ss value
    archsize_t pageTables;              // This is the process cr3 value
    PID_t pid;                          // This is the PID of this process
    archsize_t ssAddr;                  // This is the address of the process stack
    size_t ssLength;                    // This is the length of the process stack
    char command[MAX_CMD_LEN];          // The identifying command, includes the terminating null
    size_t totalQuantum;                // This is the total quantum used by this process
    ProcStatus_t status;                // This is the process status
    ProcPriority_t priority;            // This is the process priority
    int quantumLeft;                    // This is the quantum remaining for the process (may be more than priority)
    bool isHeld;                        // Is this process held and on the Held list?
    struct Spinlock_t lock;             // This is the lock needed to read/change values
    ListHead_t::List_t stsQueue;        // This is the location on the current status queue
    ListHead_t lockList;                // This is the list of currently held locks (for the butler to release)
    ListHead_t messages;                // This is the queue of messages for this process
    void *prevPayload;                  // This is the previous payload in case the process did not allocate enough
} Process_t;
```

This structure look very close to what I will end up with.  There are going to need to be a few tweaks to names, but those should be minor.

The first big thought here is about the `ss` field: Do I really need it?  Well, for arm, there is no such thing since the memory is flat.  However, with the x86 architectures, I will need to keep track of the user-space stack section (well, or do I?).  The assumption here is that all task changes will happen in kernel space, not from user space.  Therefore, the `ss` field should be able to go away -- any change back to user space will be stored on the stack.

The next 2 things I am going to do is rename `stackPointer` to be `topOfStack` and `pageTables` to be `virtAddrSpace`.  These terms more accurately describe what they are used for across the architectures.

`pid` I am torn over.  I am going to keep it for now as there may be things that want to find any process by its pid.

`ssAddr` is the allocated base of the stack.  While the tutorial does some dodgy calculation to free the stack, I prefer to store this in the `Process_t` structure.  However, `ssLength` can go away.  I will use a constant value for all stack sizes.  While I am at it, I will also need to replace the TSS `esp0` field on a task change as well for x86.  I will need to read this again in the Intel Manuals about what that is used for.  In the meantime, I will leave this off.

`command` will be changed from an array to a pointer, and I will allocate the memory from the heap -- since I have a working heap much earlier than I thought I would.

`totalQuantum` will be removed; I will add in something more appropriate when I get to step 2.

`status` and `priority` will remain as-is, but I will also be adding a `policy` field.  This field will then need to be backed by an enumeration.

`quantumLeft` will not us used for not and will be removed.  `isHeld` is no longer needed. `lock` is no longer needed.

`stsQueue` is actually going to be used as the `next` member as noted in the tutorial.

`lockList`, `messages`, and `prevPayload` are removed.

---

With `Process_t` cleaned up and the header file `process.h` cleaned of the function prototypes, I have also commented out and renamed files so that I can get the binary to compile.

So, I move on to rebuilding the `ProcessInit()` function.  This function must populate the `Process_t` structure for the current running task -- the kernel initialization process.  I am going to work on the arm version first since it will be more foreign to me.

---

### 2019-Mar-15

I am trying to get the `ProcessInit()` function written today....

---

### 2019-Mar-16

I was able to get the `ProcessInit()` function to be architecture independent.  I did not think I would be able to do that.

The next step is to rewrite the `ProcessSwitch()` function to align with my structure.  For now, I am still ignoring the TSS.  Again, I will take on the arm architecture first.

---

Working on `ProcessCreate()`, there are a couple of things that are arch-dependent.  I am also working on getting those addressed so I can test this function.

---

So, I have come across a challenge related to the stacks.  A new process needs a new stack.  However, the new process stack cannot be in kernel space.  This would allow the stack to be accessible between processes.  Therefore, the stack really needs to be in the user address space in order to protect it.

For the initial version, I think I am going to use stacks in kernel space to make it easier to test incrementally.  To do this, I will be using the `pid` field to create an offset to the next stack in kernel space.

I was able to get to a point where I could execute a basic test.  Page Fault:

```
Page Fault
EAX: 0x80002b90  EBX: 0x00000400  ECX: 0x00000001
EDX: 0x3ffff40a  ESI: 0x90000080  EDI: 0x003fd800
EBP: 0x00000000  ESP: 0xff800f7c  SS: 0x10
EIP: 0x80002912  EFLAGS: 0x00200296
CS: 0x8  DS: 0x10  ES: 0x10  FS: 0x10  GS: 0x10
CR0: 0x80000011  CR2: 0xff40b000  CR3: 0x003fe000
Trap: 0xe  Error: 0x2
```

Interestingly enough, the fault came from `ProcessNewStack()`.  I will try to sort that tomorrow.

---

### 2019-Mar-17

So that problem was a post-decrement problem where I should have been pre-decrementing the stack before adding values to the stack.

With that fixed, it is now triple faulting.

---

x86-pc is working now.  I'm trying to get arm to work.

---

### 2019-Mar-18

OK, I have this now!  The problem is that I am treating the arm architecture as if it is the x86 architecture.  In reality, the return address is not placed on the stack for arm.  If the function wants to preserve this return address, it is responsible for placing the register on the stack and preserving it itself.  So, for arm, to duplicate the call, I will need to simulate this call by populating `lr` with the actual starting address.

After building the stack in a way I can restore things in the proper order, I am finally successful!

Now, while I am not done with step 1, it is time to commit this code.

---

There are 3 things I left outstanding:
1. The stack really needs to be relocated to user space.  I am not yet in user space scheduling, so I will leave this for later.
1. There needs to be a stack for kernel calls.  Again, I am not yet in user space, so I will leave this for later.
1. I should have a top level paging table for each process.  Currently, I am sharing the kernel tables.  As I am not yet back un user space, I will save this for later.

So, this then moves me along to Step 2 in the tutorial.  Step 2 is to implement a trivial round-robin scheduler.  This is still a cooperative multi-tasker.

If I recall the tutorial correctly, a good majority of this code is removed in later steps.  I'm OK with that.

---

### 2019-Mar-19

I was able to get the Round Robin scheduler working yesterday at the end of the day.  This was with 2 processes.  I want to test with 3 or more processes today.  A simple test that worked well.

I am going to commit with step 2.

---

