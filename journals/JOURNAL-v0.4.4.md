# The Century OS Development Journal - v0.4.4

## Version 0.4.4

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

### 2019-May-14

Now, I move on to Messaging.  This will also be as close to Posix compliance as I can get.  First, a bunch of reading.

---

### 2019-May-15

More reading, a little more coding....

---

I did manage to write a `MessageGet()` implementation today, which will be the kernel implementation of `msgget()`.

---

### 2019-May-16

I am working on the `MessageControl()` function, and have reached a point where I need to manage blocked tasks.  I actually got these out of order a bit, as `MessageControl()` would be a bit easier if I already had the structure in place that manage the blocked tasks as a result of `MessageSend()` and `MessageReveive()`.  It's OK, I got this.

Ok, now to be able to send a message.  One thing here is that the call can block if there is not enough room for the message.

---

Linux departs from the POSIX standard for `msgsnd()` in the following way:
* POSIX measures the total number of system-wide messages and limits the number of messages on all the queues based on this system-wide limit.
* Linux measures the total number of messages on any given queue and compares that to the number of bytes allowed on that queue -- which basically treats any 0-length messages as having a minimum of one byte.

Since CenturyOS is not going to have a system-imposed total message count limit, the latter (Linux) method will work better for this implementation.

The next thing to consider is that `msgsnd()` may block.  When it unblocks, it may or may not have enough space for the desired message.  If it does not, we need to make sure we block it in a manner in which it will unblock next again.  This may be a problem with the semaphore implementation, which [I will need to review](http://eryjus.ddns.net:3000/issues/411).

---

### 2019-May-19

Well, I accidentally destroyed one of the physical volumes that was used for the `/home` directory structure.  As a result, I lost the entire `/home` mount point... meaning all my code.  Thank God I am backing up daily and thank God I have an off-site copy of my code at major milestones.  I only ended up losing about an hour's worth of work, and it was not on this project.

---

Hmmm....  I need to consider some things....  For example, suppose several processes block waiting for space in a message queue.  When say for example 100 bytes are freed from the queue, the first process unblocks and adds a 2-byte message.  Well, there is still room for more messages to be added and processes to unblock as well.  This will need to be thought through a bit more.

---

### 2019-Jun-06

Well, it has been a while....  I have been working on an [Ada compiler](https://github.com/eryjus/ada) during this time.

Well, I think it is time to come back to this and make some progress here as well.  I need to get back to the message queue management.

Where I left off was trying to understand what to do when a process unblocks after some room is made available.  If there is enough room, several processes should be allowed to add messages to the queue.  I think the best thing to do is to unblock everything and let the scheduler sort it all out, allowing processes to block again if needed.  This may end up being more of a shotgun approach, but the alternative would be to keep track of which messages are which size and release the process enough to be able to fill the queue again.  I really am not interested in that approach.  In addition, this also lets the system send messages in priority order (based on process priority) rather than on first come first served.  I think I like that solution better anyway.

So, with that, I need to figure out where I left off and what to do to pick up the pieces.

As a side note, I am also now working on 2 projects at the same time.  Not that it is my favorite thing to do, but I really want to keep them both moving forward.  To allow for this, I think I am going to work on one for no more than a week before I switch to the other one.  Hopefully, this is not too disruptive.

So, the task I am left at is waking up any processes that are waiting on a message and then letting them sort out of the message is relevant to them.

The overall question though is of order.  Does sending and receiving order matter?

My initial response is that at the OS level it does not...  there should not be multiple receivers of a particular message on the same queue.  And if there is, then that is an application problem.  Therefore, order and priority do not matter.

---

### 2019-Jun-07

I was able to get the `MessageSend()` function tidied up today.  Now, I move on to the `MessageReceive()` function and get that written.  This should be relatively easy by comparison since it will mimic `MessageSend()` closely in overall structure.

So, my implementation of messages assumes that there is a relatively small number of processes that are communicating back and forth using this queue, not hundreds and hundreds of processes all using the same queue with a pair of processes using a single message type.  In the case of the latter, I do expect performance to suffer.

While this code is not tested, I think it is time to commit.  I am losing interest in this and need to move on to something else.  I will have to take on the debugging effort once I get the APs booted -- a decision I am going to regret later.  However, I'm having some ADD problem with this line of development.  As a result, I'm task switching way too often.

