***Prologue***

So as I am sitting down today to start laying out how my virtual memory manager will be used, 
I really should be documenting my progress and thoughts if this is really going to be 
something others can learn from.  The README.md file is not really suited for the type
of "conversation" I want to have here.  One of the goals of this project is, after all,
to have a result that others can learn from -- even if it is from my mistakes.

So, what has happened to this point that I really should take a moment to catch up on
before I get into the rest of the decision making.  Well....

I have decided on the initial architectures I would like to support:  i686, x86_64, and
Raspberry Pi 2b (or 'rpi2b' as you will come to recognize throughout).  I also have an
rpi3b sitting on my desk, but the ARM architecture if foreign enough at this point I want
to make sure I can get through that architecture before I take on a new one.

I have been able to set up an emulator for each of the architectures and I am able to use
make to invoke each architecture to test.  My memory is not as sharp as it used to be and
so I cannot always recall the exact command I want to type for each emulator -- there are 
slight differences.

I have built up a massive (and admittedly difficult to maintain) make system, and have torn
the core of that make system down in favor of tup.  I did talk about it a bit on osdev.org, 
here: http://forum.osdev.org/viewtopic.php?f=13&p=275461.

I also have taken on the rpi-boot second stage to boot the rpi2b architecture (compare that
to GRUB for x86-family).  I have a loader (which has much my focus to-date), and a kernel 
(which is loaded my the second stage but never gets control, yet).

And I have a bitmap physical memory allocator (at least initialization) started.  Everything
boots and the kernel module is loading.

I did start to parse the ACPI tables, and then decided to rip that code back out.  This 
deserves a bit of conversation.  I have conflicting goals and I had to figure out how to 
reconcile them.  The conflicts were: 1) all 1-time initialization was going to be performed
in the loader; and 2) the CPU was going to be placed into its native mode when jumping
from the loader to the kernel.  Well, the issue was that the ACPI tables were not all 
guaranteed to be locatable by 32-bit code.  This means the for 64-bit systems, I would need
to either read the table in the kernel module, or would need to get into the native CPU mode
prior to the jump to the kernel code in order to complete the initialization.  I chose to 
delay this decision by postponing the integration of the ACPI tables.

So, my priority is to get into the native CPU mode and then transfer control to the kernel
module.  This change of course makes my Virtual Memory Manager (VMM) the next on the list.

I wanted to incorporate all the thinking I had done with Century64 (the assembly-based 64-bit
version), but I just realized that the documentation was lost in the "Big SAN Crash of 2015!"

---

***2017-May-14***

I started to lay out the virtual memory today.  I intended to lay out the memory for all the
architectures but quickly realized the differences that are required.  So, I stepped back to
start with the 32-bit x86-family.  I stated a few of the base functions that will be used
to manage this memory.

At the same time, I had to add some additional functions for the Physical Memory Manager that
I was not expecting to need (check an allocation and allocate a new frame).

Also, the things to do list is getting to be a bit more than I can keep track of.  I need 
a separate tool to keep track of them, but I do not want to include them all in the code
as TODO: items and I do not want to create a GitHub bug for them.  I have a Redmine server
as a part of my home computer environment, so I am going to use that.  Forgive me for not
having a public to-do list. 

---

***2017-May-15***

So, the first thing to do today was to implement the missing function FrameNew(), which is
exceptionally trivial.  I will not be allocating too many frames in the loader and have no
desire to reuse this code in the kernel.  The algorithm used starts from the high address 
point and walks back down toward 0.  This is so that the frames we allocate here for the 
MMU structures are generally out of the way and not going to be used soon (and probably 
not ever released).

At the same time, it took on the task to properly set the function prototypes in proto.h.
This change enabled the elimination of the phys-mm.h file.

After that I spent the rest of the evening sorting out the physical memory manager and MMU
initialization.  As I call it a night, I think I have it all working.  I will test again 
tomorrow.

---

***2017-May-16***

Today, as I was considering the additional MMU mappings I needed, the frame buffer got in the 
way....  I have to figure out where I want to put this in the overall memory map.  This led 
me to scrap completely the map I had started with and develop a new one.  Of particular note
is the plan to adopt Linux's use of a "poison" memory space.  This space will generate a 
page fault with any reference that theoretically will lead to the *type* of object improperly
initialized.  More information is available in the linux source: 
http://elixir.free-electrons.com/linux/latest/source/include/linux/poison.h.

Now, when I go back to fix up the phys-mm.c initialization, I realize that there is a lot of
instances where I am looping over a number of frames to allocate or free.  It becomes a good 
opportunity for code cleaning.  So, I'm taking this on.

At the end of the day, I have been able to map the target frame buffer, but I am still having 
issues with the phys-mm.c initialization.  In particular, MB2 structures appear to be 
reporting all memory ranges as available.  I may have to go back to the MB2 parser to address 
this.

---

***2017-May-17***

What a difference a day makes...  I was able to pinpoint the MB2 issue within seconds with a 
fresh perspective.  It always amazes me how walking away from a problem can actually help you
solve it faster.  So, at this point, both the MB1 and MB2 boots of i686 match.  Good!

The MMU is not complete yet, but I will back out all my debugging code and commit an 
intermediate checkpoint.  This is a good point to be able to roll back to if need be.

I decided to take on determining why the kernel module was not presenting its name to GRUB
when it is loaded.  I decided to try to narrow down where the problem actually lay.  Since 
the loader is all 32-bit code for both x86_64 and i686, I can run both loader targets on 
both architectures and determine easily where the problem lay.  So:

| Loader version | qemu-i386 | qemu-x86_64 |
|----------------|-----------|-------------|
| iso/i686.iso   |   Works   |    Works    |
| iso/x86_64.iso |   Fails   |    Fails    |

This simple table clearly shows that my problem is with the x86_64 image.  I converted the 
x86_64 kernel to 32-bit and the problem remained -- leading me to believe that the problem
is not related to the elf64 format of kernel.elf.  I also copied the x86_64 loader to be
the x86_64 kernel and the i686 kernel to be the x86_64 kernel and the results were the same:
the kernel name did not show up.  Leading me to concluding that the kernel itself is not
the problem.

After more testing with the .iso images, I have been able to determine that there is something
with the loader.elf image that is causing the problem.  When I use the loader image from i686,
everything works as expected.  However, when I use the x86_64 loader image, it does not. I
have no conditional compiles based on the architecture, so it is likely in something related
to an architecture-specific file.

After still more testing, I decided to rename my x86_64 loader and copy the i686 loader in 
its place.  Then exhaustively, replace the files with the saved off versions, compiling and 
testing after each step.  I was finally able to determine that the file that caused the 
problem was arch-proto.h.  The difference between the 64-bit version and the 32-bit version
was the `arch_addr_t` type, defined as `uint32_t` in the i686 loader and `uint64_t` in the
x86_64 loader.

Some searching through the mb2.h file found 2 uses of the `arch_addr_t` type specifically 
in the modules sub-structure.  Well, the MB2 specification specifically calls out the sub-
structure to look like this:

> ```
>           +-------------------+
> u32       | type = 3          |
> u32       | size              |
> u_phys    | mod_start         |
> u_phys    | mod_end           |
> u8[n]     | string            |
>           +-------------------+
> ```

The specification's definition of the type specifically states:

> `u_phys` -- The type of unsigned data of the same size as target architecture physical
address size.

Well, I read that as 64-bits for the x86_64.  However, I now believe that even with the 
x86_64, the target architecture is still 32-bit since MB specifies a 32-bit state.  I am
reasonably convinced that changing the type to 32-bit makes that distinction as well.

By the way, I make a few other minor corrections.  What a mess.  I think the next task will 
be to tackle updating the phys-mm with the loaded modules and getting them mapped into the 
VMM.

---

***2017-May-18***

Taking care of the phys-mm modules was easy to address.  The VMM of the kernel was not mapped
yet (I want to take another look at the kernel.ld script for the kernel before I take that 
on).  However, the bulk of the evening was spent reading up on and digesting the ARM MMU 
structures and considering how I would implement them.

Some special requirements I need to come to terms with:
* The Level1 Translation Table needs to be allocated 16K contiguous and must be 16K aligned.
There will be a special need for a function to address this need.
* The Level2 Translation Tables are 1K in length.  If I am allocating chunks in 4K increments,
I need to be able to carve those 4K frames into 4 X 1K L2 tables and keep track of them and
be able to reuse them.  This sounds a lot like a stack, but not sure how to implement this
yet.
* Is there a method that will allow for recursive mapping???
* I think I need to clean up the phys-mm initialization functions for rpi2b (I think the MB1
mmap tables are empty).
* What is the purpose of the P bit [9] in the L1 Table?  Bits [1:0] == 0b00 indicate a fault,
so I'm not sure is this would mean "Present" or not.

---

***2017-May-19***

So having thought about the concerns overnight, I have some plans I can execute against.
Breaking this down a little bit, here is where I am at so far:

* **Align TTL1 to 16K** -- This is relatively easy to implement...  I will need a special
purpose function to allocate a 16K aligned 16K block of frames.  Since I may want this in 
the x86-family architecture, I will make this a common function.  I will probably also 
need to make this generic (looking ahead to future things).
* **Align TTL2 to 1K and each table is 1K long** -- Well, this one puzzled me a bit.  However,
it finally dawned on me to continue to handle 4K frames and just allocate 4 of them for the 
correct offsets.  Therefore, if I am looking for TTL2 table at 0x01, then I can allocate 4
X 1K tables intended for offset 0x00 and then set all 4 of them from 0x00 to 0x03.  In this
way, there is no need to keep track of partial frame allocations or stacks of free partial 
frames when I hand off the state to the kernel. 
* **Frame Granularity** -- This was not identified last night, but if I set this as a guiding
principle for the loader/kernel, then things will become simpler to keep together.  I am 
using 4K frames for the x86-family and 16K is a multiple of 4K, and 1K is a divisor of 4K, 
and 4K is one of the page sizes for the ARM architecture overall (including my rpi2b), so 
it makes sense to keep this as a principle.  Done.
* **TTL1[P]** -- In reviewing the ARM Architecture Reference Manual, I read the the P bit
(specifically bit [9] in the TTL1 table) is _implementation defined_.  The ARM ARM goes on
to define that _implementation defined_ (I'm not sure yet if this is referring to the rpi2b
or the software): 

> Means that the behavior is not architecturally defined, but must be defined and 
> documented by individual implementations. 

I have not yet had the opportunity to white-board **Recursive Mapping**, and the **phys-mm**
cleanup will be a to-do.

Well, it occurs to me that the beauty of the x86-family of recursive mapping is the fact 
that the Level 2 table looks exactly like the L1 table, and any L3 tables looks exactly like
the L1 and the L2 tables.  Well with the ARM, the L1 and L2 tables are completely different.
This leads me to the conclusion that recursive mapping for the rpi2b architecture is not going
to work.

However, we are able to do something similar that will take up a similar amount of memory as 
the x86-family.  The sum total of all the TTL2 tables is 4MB -- the same as the 32-bit 
recursive map loss.  I could easily define the same memory region for these level 2 tables.
This then only leaves the 16K TTL1 table.  It would be trivial to map a 16K region of 
memory to be used to map to this table.  I am going to take this approach.

So, with these decisions, it's time to start laying out the memory map for rpi2b and writing
the structure code for the implementation.

After reviewing my i686 memory map, I made a change so that the Temporary mapping range was 
immediately below the Recursive mapping space.  This allows me (on the rpi2b) to set the
additional 16K memory space right below the analogous recursive mapping space and achieve 
very similar results with the rpi2b architecture.

---

***2017-May-20***

I had several false starts to lay this code for the TTL1 and TTL2 tables in a readable format.

---

***2017-May-21***

More research and more thinking and more false starts.

I did manage to get the functions written today.  They turned out to be surprisingly simple 
considering the overall complexity of the ARM table differences.  I also ran the rpi2b loader
and it did not crash.  It doesn't mean they are correct.  I will test some tomorrow.

By the way, I was also responding to the following post in the forums on osdev.org today and 
I decided to take my own advice: http://forum.osdev.org/viewtopic.php?f=1&t=31934.  I wrote 
several functions to simplify the management.

---

***2017-May-22***

OK, I will need to revisit the i686 implementation of the paging tables.  I will do that once
I get the x86_64 tables mapped.  That way I can keep the i686 as a reference while I develop 
the x86_64 version in case I need it, and then when I have everything dialed in I can use the 
x86_64 version as a reference for the i686.

However, it all comes apart again!!!  I have just realized that I am mixing bit-ness (again) 
and may not be updating the structures I think I should be at the address I should be.  The 
structures and functions I have created will work perfectly for the kernel with paging 
already enabled.  However, for initialization, they are completely wrong.  

One of the lessons here is that the 64-bit initialization will be most telling if it is 
wrong.  The key symptom is that the addresses during the initialization are 32-bit but 
initialization is preparing for 64-bit memory addresses.  The frame allocator needs to only 
hand out 32-bit frames during initialization.  I also need to write an integrity checker so 
that I can trap errors in runtime quickly.  I will have to take on the following tomorrow:
* Change the FrameNew() function to only allocate 32-bit frames
* Move the mmu.c files to the kernel (surrounding code with `#if 0`|`#endif`)
* Start the integrity checker and build that out ahead of the next step
* Rewrite the initialization

---

***2017-May-23***

I started my day today by moving all 3 mmu.c files to the kernel module where they wil be 
picked up and polished at a later time.  I then did a global search an all references to 
the arch_addr_t type and verified that all usages were legitimate.  Several changes were 
required.

It's disheartening to have to admit to myself that the majority of the new programming
I have done over the last week has been unusable (at least for the task at hand).  I will 
be able to reclaim some of that work in the kernel, but it still stings a bit today.  At
least the debugging work will still sound.

Through the course of my debugging today, I found a stupid error in memset() and memmove().
Despite the expletives, I now have it working.  I have created an initialization for the 
64-bit 4-level paging structures that I can easily leverage for the i686 architecture (copy 
and modify, not reuse).  I will move on to dumping the paging structures.

At the end of the day, I feel I have a decent set of working functions.  More testing 
tomorrow.

---

***2017-05-25***

My day job got to me yesterday.  I was not able to do anything but sleep by the time my day
was done.

So tonight I was able to copy the x86_64 version to i686 and strip it down to be 32-bit.
This activity also exposed several small things in the x86_64 version which got fixed.

---

***2017-05-26***

Today's task is to take the i686 version of the mmu initialization and adapt and change it
for the rpi2b architecture.

---

***2017-05-27***

Before I started in on my day's other activities, I thought I would take a shot at some 
debugging.  It turns out that I was able to complete most of it last night and I only had
a couple of small things left this morning.  They were easily identified on a fresh night's 
sleep.

So as I write and commit this, I now have the MMU initialized for all 3 architectures, and
tested to the best of my ability until I am able to put the systems into their native modes.
While I am "out and about" today, I'm going to ponder a bit what is remaining to be able 
to pull that trigger.

So, with some thought today, it appears that my todo list is rather short now:
* Create an ELF loader to set up the kernel
* Create the permanent GDT appropriate to each x86 architecture
* Set up the IDT (some thinking required here; not sure what this will mean for rpi2b)
* Relocate the PhysMM bitmap (mapping the memory of course)
* Relocate the structure to communicate with the kernel (mapping the memory of course)

---

***2017-05-28***

Well my honey-do list got the better of me last night, and a good part of the day today
as well.  I'm not too sure how much I will be able to get through is a short sitting.  So,
I though I might start with some objective analysis of the to-do list from yesterday.

I nearly absolutely have to write an ELF loader to jump to my kernel.  The alternative would 
be to produce the kernel as a binary file (which has some merit), but that is not something 
I am wanting to do.  The key reason is that I would have to give up the ability to have
my symbol table loaded at the same time.  While I am not going to use it right away, it
it not really something I want to give up either short term or long term.  However, I 
will most likely tackle it last to postpone a final decision on whether to ELF or not
to ELF.

The GDT is not in its final form.  It does not exist on the rpi2b, so there is nothing
to address there.  The i686 is technically correct but not in the right spot.  Perhaps
I can take care of that in the startup and just hard-code the address there and "inform" 
the PhysMM what frame I am using.

The 64-bit version is a little more interesting and may drive some back-peddling on the
i686 setup.  Again, it is not in the correct place.  I may be able to address it in the 
same manner as I would for the i686.  But then I may also need to re-address it when 
changing to long mode.  And then I start to wonder.  Do I **need** to have another GDT?

So here is what I am thinking...  Maybe I have a 64-bit GDT already prepared where I 
have some 32-bit offsets already set up that I use for the loader and then when I am 
switching to long mode I can use an offset for a 64-bit GDT Entry.  This would mean
that my segment selector would be quite a bit higher than 0x08 and 0x10.  But that
does not appear to be a deal-breaker.  I'm going to have to research on this a little
bit more before I can call this one.

I really do not need an IDT before I change to the kernel.  I have been back and forth
on whether I want a common handler or have ability to hook straight into an IDT entry
without the jump tables.  A bit more thinking is set here, but this is technically not 
required before I jump into the native mode (in other words not having one will not 
prevent this jump), so it comes off the table.

The last 2 items are nearly trivial with memmove().  So they will be completed.

So, my list of 5 things has become 4.  And one really has my interest.  So, I start with
the GDT research.

This research confirms that I can do what I want.  Both the 32-bit and 64-bit modes use
64-bits to describe the GDT entries.  Therefore they can coexist.  However, the order
is quite important if we are to use SYSENTER and SYSEXIT for system calls.  I have not
made any decision on this either way, but I also want to make sure that any decision I
make in this next step will not limit my choices to a single option either.  Therefore,
wherever these selectors are, they must appear in this order: 
1.  Kernel Code
2.  Kernel Stack (this may be kernel data as well)
3.  User Code
4.  User Stack (and this may be data as well)

Not to mention I will also need space for a TSS, or more to the point one for each core
(and that can/may be extended to 1 GDT per core each with its own unique TSS).  I also
think it is important to note that xv6 uses a very short segment selector for CPU-specific
data (like 8 bytes, if I recall).  *However, that construct cannot be used easily in 64-bit
more since there are no limits on the selector entries* (**is this really true????**).  I
believe I will need to map that CPU-specific data to a single page and have separate cr3
values for each core.

But, before I go on, I need to standardize the GDT layout for both i686 and x86_64 
architectures.  The goal is to make them common and maintain the above 4 segment selectors
in their proper order.  So, my thoughts are:
* Separate data from stack?
* Need special loader segments
* TSS last to make sure I have room for growth (each TSS entry is 128 bits)
* More out of convention than need, I think I want the kernel code to be 0x08

---

***2017-05-29***

So, first to complete the outstanding item from yesterday:

> *However, that construct cannot be used easily in 64-bit more since there are no limits 
> on the selector entries* (**is this really true????**).

The Intel SDM, Vol3A, Section 5.3.1 addresses this:

> In 64-bit mode, the processor does not perform runtime limit checking on code or data 
> segments. However, the processor does check descriptor-table limits.

So my statement above is correct.  In 64-bit mode segment limits are irrelevant.  Likewise,
the rpi2b does not have limits (in fact, no segmentation -- it's all flat).  So, I will
treat the i686 in the same way: the limits will be from beginning to end.  It also means
that the special trick with the extremely short segment with CPU-specific data is not going
to happen.  It will be a page for each CPU, with each CPU having its own paging tables.

So, that leaves the GDT layout.  Here is the target layout:
* 0x00<<3: NULL descriptor
* 0x01<<3: Kernel Code
* 0x02<<3: Kernel Data (includes stacks)
* 0x03<<3: User Code
* 0x04<<3: User Data (includes stacks)
* 0x05<<3: Unused (left open for user data if I want it later)
* 0x06<<3: Unused (left open for kernel data if I want it later)
* 0x07<<3: Loader Code
* 0x08<<3: Loader Data
* 0x09<<3: TSS
* 0x0a<<3: TSS Second half (for 64-bit; NULL for 32-bit)
* 0x0b<<3: Future use call gate? (need to research the rpi2b capabilities)
* 0x0c<<3: Future use call gate?
* 0x0d<<3: Future use call gate?
* 0x0e<<3: Future use call gate?
* 0x0f<<3: Future use call gate?

For now, I will use only 16 GDT entries.  This will be frame aligned and have some room to 
grow.  The call gates may be able to be aligned with the SVC instruction for rpi2b.  That's
later.

One of the things to discuss is the TSS.  One of my goals for the loader is to complete all 
the 1-time initialization.  One such task is to execute the `ltr` instruction immediately 
after the jump to long mode.  On the other hand, the jump to long mode will happen at the 
same time as the jump to the kernel ELF file.  With with the choice to either complete the 
jump to long mode completely inside the loader or to execute this small bit of initialization
in the kernel, I am going to add this to the other things that need to be done in the kernel.
Another thing I simply cannot do in the loader is to release the page memory for the loader.

OK, back on topic: I should be able to complete the TSS initialization in the loader, but 
that will take a bit more work.  However, it is not critical for jumping to the kernel.

As for the GDT, I was able to get that moved to its final home at frame 0.  It will be 
permanent in that location.

---

***2017-May-30***

Today I get started on parsing the ELF kernel file....

I discovered today while investigating the kernel, I have lots more sections that I really
want.  I need to work on the loader.ld and kernel.ld files to reduce this from 15 sections
down to a handful.  I have also figured out that I am not linking my kernel for the correct
locations.  These 2 issues distracted me from parsing the ELF files.

---

***2017-May-31***

I received a response for jncronin (John, I think) about a change request I had put in.  The
change was completed and he needs a hardware test.  I'm more than happy to do it, but I need
to write a test to be booted on my hardware. 

Well, if I'm asking for a change I need to be able to help with its testing.  It's only fair.
With that said, I wll take a few days to write this change and execute some tests.  In the 
meantime, this project will be on hold.
