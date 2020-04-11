# The Century OS Development Journal - v0.6.1

This version will start to build the messaging foundations.

## Version 0.6.1a

This micro-version will implement the `msgget` system interface.

---

### 2020-Apr-08

The Posix XSI interfaces specification indicates that a filename must exist as part of the requirements to generate a `key_t` which is used to get to a unique message queue.  The function of building this key value resides in the user-space C-library -- this is not a kernel function.  But a kernel that uses these queues also needs to be able to build these `kKey_t` values.  I will use hard-coded values for now.  I may come back and revisit this later.

So, the first thing I need to do is to define the type `key_t`.  `glib` makes a mess of this by all kinds of `#defines` and intermediate `typedef`s.  I'm not going to get that fancy.

Posix says that this prototype is defined in `sys/msg.h`.  However, this is for user space.  I need a kernel space function to handle this work.  The function name will not be the same.  It's also clear that the actual implementation is up to me -- I only need to be able to provide the proper interface to user-space.

So, with that, I really only need to get started with the code.  The key things to keep in mind is that the interface functions:

```c++
int       msgctl(int, int, struct msqid_ds *);
int       msgget(key_t, int);
ssize_t   msgrcv(int, void *, size_t, long, int);
int       msgsnd(int, const void *, size_t, int);
```

OK, I think I have the header file all worked out.

---

### 2020-Apr-09

I was thinking last night about ownership of the message queue.  I am going to need a field to describe the owner at some point.  But I will add that when I get there.  For now, I will be just assuming that when the number of processes referencing a message queue, it is available to be deleted.

Now, one process can reference many message queues, and one message queue can be referenced by many processes.  This is a many-to-many relationship and that makes a mess of things.  To resolve this, I am going to need a structure to resolve these references.  Or...  can I just get away with a reference count?  Nope....  If something dies, I need to be able to clean up its mess.  I need specific references.

With that, I think I have all I need to start my `MsgqInit()` function, and then the `MsgqCreate()` function.

When releasing the reference to a resource, the trick is to find both the Process Reference and the Resource Reference and remove both.  For safety sake, they need to be checked.  The problem is the many-to-many relationship and the list implementation: I have to search both lists for a match.  This is ***ugly!!***  I'm thinking O(n<sup>2</sup>).  The only thing I can think of is to add the `Process_t` address and the resource address to the `Reference_t` structure to make checking easier.  With that, I should only need to check the process's list of references and if that does not exist I can just call it and move on.

---

I was able to get all the messaging kernel functionality written.  I do not yet have the Posix SYSCALL interface ready and that will not come for some time.  Everything compiles.  I will have to construct a couple of test processes to see this working.

And both processes block on a MSGW status.  So, this can be likely one of a few things:
* The message does not get queued properly
* The process is not added to the waiting queue properly
* A message is not being identified to be received properly
* A process is not getting readied properly when a message arrives

I cannot find anything rubber-ducking the code.  I'm going to have to build some debugging functions for message queues.

```
+---------------------------+--------+----------+----------+------------+-----------------------+
| Command                   | PID    | Priority | Status   | Address    | Time Used             |
+---------------------------+--------+----------+----------+------------+-----------------------+
| kInit                     | 0      | OS       | DLYW     | 0x90000040 | 0x00000000 0x00d12ec8 |
| Idle Process              | 1      | IDLE     | READY    | 0x900000e0 | 0x00000000 0x7bc03ca0 |
| Idle Process              | 2      | IDLE     | RUNNING  | 0x90000180 | 0x00000000 0x7bb87470 |
| Idle Process              | 3      | IDLE     | RUNNING  | 0x90000220 | 0x00000000 0x7bb04a98 |
| Idle Process              | 4      | IDLE     | RUNNING  | 0x900002c0 | 0x00000000 0x7bb288d0 |
| kInitAp(1)                | 5      | OS       | TERM     | 0x90000360 | 0x00000000 0x00009088 |
| kInitAp(2)                | 6      | OS       | TERM     | 0x90000400 | 0x00000000 0x000088b8 |
| kInitAp(3)                | 7      | OS       | TERM     | 0x900004a0 | 0x00000000 0x000088b8 |
| Process A                 | 8      | OS       | MSGW     | 0x90000648 | 0x00000000 0x000007d0 |
| Process B                 | 9      | OS       | MSGW     | 0x900006e8 | 0x00000000 0x00000bb8 |
| Kernel Debugger           | 10     | OS       | RUNNING  | 0x900007bc | 0x00000000 0xa4fd6958 |
+---------------------------+--------+----------+----------+------------+-----------------------+
sched :>
 (allowed: show,status,run,ready,list,exit)
```

As you can see, `Process A` and `Process B` are not getting any CPU once they block on the message queue for the first time.

What kinds of things do I need to see from a debugging function for message queues?  Well, I think I need to show a list of them, with the number of messages queued, the number of processes waiting, and the status of the queue overall.  I may have to take that on tomorrow.

---

### 2020-Apr-10

I am working today on getting some debugging code written to view the message queues.  I will have 2 commands so far:
1. `status` -- which will give the overall status of the queues
1. `show` -- which will give lots of details about one of the queues in question

Implementing the first one will be relatively easy.  The second one, on the other hand, is going to require either some name or ordinal queue number to work, which requires updating my structures a bit.  Let me start with some status code.

OK, interesting:

```
- :> msgq
msgq :> stat
+------------+--------+--------------+------------+--------------+------------+
| Queue Addr | Status | #Msgs Stated | #Msgs Calc | #Wait Stated | #Wait Calc |
+------------+--------+--------------+------------+--------------+------------+
| 0x90000540 | ALLOC  | 1            | 0          | 0            | 0          |
| 0x900005c4 | ALLOC  | 1            | 1          | 0            | 0          |
+------------+--------+--------------+------------+--------------+------------+
msgq :>
 (allowed: status,show,exit)
```

I have a message that was pulled out of the queue but the count was not updated properly.  Also, the number of waiting tasks is 0 for both queues yet both processes are waiting in a `MSGW` status.

And I found a bug when I block waiting for a message -- it was not queueing the process into the waiting queue.

So, after testing, I think it is time to commit and merge this micro-version.  The next step will end up being the post-boot cleanup, and turning the `kInit()` process into the `Butler()` process to maintain the system.

---




