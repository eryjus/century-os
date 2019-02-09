# The Century OS Development Journal - v0.3.0

## Version 0.3.0

This version will specifically separate the architecture and platform abstractions and reconcile the code better that it is organized.  There will be no new functionality added with this version, with two exception: 1) I will properly separate the kernel and user paging structures for the rpi2b architecture, and 2) I will eliminate the loader in favor of putting the initialization in the kernel where it can be reclaimed later.

At the same time, I may have an opportunity to clean up the virtual address space and if it is clean to do so, I will take that on.

The goals for this version are tracked here: http://eryjus.ddns.net:3000/versions/7

---

### 2019-Feb-07

Today I start reorganizing the source tree and taking better care of the arch and platform abstractions.  At the same time, I will be eliminating the loader and performing all the initialization in the kernel, organized in a way that I can better reclaim the space.

I want to talk about this second goal for a bit.  I specifically stated this as a goal in the beginning.  But I ran into trouble:

> However, I need to revisit the goal of having all the one-time initialization completed in the loader.  I'm nearly positive at this point that I am not going to be able to meet this goal.  For example, I will need to have kernel exception handlers that for things like divide by 0 that will be responsible for shutting down the offending process.  I am not able to initialize that in the loader (at least not without some really fancy data structure in the kernel that I find and read for addresses -- which might be a possibility at a later time) without having a symbol table to read and I am not really interested in forcing debugging information to be available in the kernel.  While it is a great goal, I think it should be set aside for a moment and revisited once I have a better working system (read: working system).

... and later:

> These are implemented in the `loader` and cannot be moved to the kernel.  They will have to be re-implemented.  However, there is an additional concern: When I re-implement these functions, I will need to have a working PMM again.  So, the solution is going to be to allocate more space in the pmm binary and to purpose build the MMU functions I need to get the PMM running.  All of the one-time initialization was part of what I wanted to avoid in the kernel by moving to the loader, but I do not believe that is going to every come to fruition at this point -- not with my little hobby kernel.

These both indicate that my goal was unrealistic -- at least physical separation.  However, if I take a step back I might be able to accommodate this goal by placing all the initialization code in its own segment and then reclaim that when the initialization is complete.  This way, I can share code properly (which is the underlying problem from both quotes above).

This, then, brings me to where to start....  The best place is going to be to develop out arch and platform and then redo the loader (the other way around has me making significant changes in this crappy code base).  At the same time, if I chose to defer the loader changes until v0.3.1, I am not committed to anything yet.

Tomorrow, I think I will take a hard look at how I am going to reorganize the source tree.  One of the things I have working for me is the concept of one function per source file....  It should make this process relatively easy to get through.  Ha!!

---

### 2019-Feb-08

I was thinking today about the organization of the source tree.  I am not sure, but I may end up combining the PMM and the loader and the kernel all into the same executable and then manage the access through the paging tables.  They key to this plan in my opinion would be to set up lots of sections and then abandon them when I am done (reclaiming the pages and frames when I do).  But, the thing that I am not certain over at this point is how to organize the modules I would create.  For example, I would like to end up with a GUI, but I would be happy porting `bash` as a first step.  Even before that, I will need a file system driver which will likely be an external module loaded by grub for me.  The point is where do I fit these into the source tree?

Taking some inspiration from [lk](https://github.com/littlekernel/lk), there is a folder for each of the following:
* kernel
* apps (or what I call modules)
* lib (well, lots of things here)
* tools (which I assume are related to building the OS)
* and the all-important arch and platform folders

`lk` also creates a `build` folder to build a project.  It is a fully integrated project where one build command yields everything for a particular target (arch + platform).

Well, for me, I will start with the kernel, arch, and platform folders.  These will be organizes in the following manner:

```
century-os
+-- arch
|   +-- arm
|   |   +-- armv7
|   |   +-- armv8
|   +-- x86
|       +-- i686
|       +-- x86_64
+-- kernel
|   +-- inc
|   +-- src
+-- modules
|   +-- module1
+-- platform
|   +-- bcm2835
|   +-- bcm2836
|   +-- pc-isa
|   +-- pc-pci
+-- targets
    +-- rpi2b
    +-- i686-isa
```

Well, I think anyway.  Here are some thoughts:
* I will keep common code in `arch/<arch>` while keeping things that are specific to the specific bitness of a cpu in the respective folders (a perfect example of this would be the MMU tables which are quite different between 32-bit and 64-bit systems).
* I will still keep `sysroot`, `iso` and the like -- just not included in the changes for this version.
* `bin` will be eliminated eventually (but I need it for now to keep things compiling properly).
* `module`s may end up with their own `platform`s folders as needed (likely omitting the `platform` parent folder); I cannot yet see a module needing its own `arch` folders.

Another thought here is which to work on first.  I was thinking last night that I should probably work on the arm stuff before the x86 stuff.  The thinking here is that arm is so foreign to me that I can keep the x86 requirements in the back of my mind while I am actively working on arm and still end up in far less of a mess when I change architectures to catch up than I would if I worked in the opposite order.

So, now, in the kernel folder, do I want to keep the component folders I have today?  I think I do -- even through it will create a bit more complicated build system and a slightly deeper organization structure.  Because I have a single file per function (generally), this will keep where to look a little easier find.

---

At this point I have been able to move the heap over.  This was rather trivial since it uses absolutely no architecture-specific or platform-specific code.  At the same time, I was able to identify 2 things that need to be cleaned up a bit, both of which are logged in Redmine.  These will be taken care of later, once I have completed `kernel.elf` for both targets compiling.

---

I was also able to bring in ipc and frame-buffer.  This then is the end of what works without needing platform- or architecture-specific code.

From here I am going to copy in `kprintf.cc` and `kInit.cc` (which will force all the remaining `*.h` files to be copied over).  And I am going to create the build rules to link the `kernel.elf` file which will drive all the missing functions.

---

### 2019-Feb-09

Well, I was able to get `kernel.elf` to compile today.  I was not required to move every source file from the old folder (several low-level functions are not used in the kernel).  I was also able to identify several things that need to be cleaned up once I get the i686 arch and pc-isa platform copied over as well -- each of these are in Redmine now.  Here is the living document for the [v0.3.0 roadmap](http://eryjus.ddns.net:3000/versions/7).  A quick test and this kernel still operates as it did.

So, I now have a decision to make: Do I move over the i686 stuff next or start to resolve the issues raised in Redmine?  After reviewing the goals and current Redmine issues, I think it best to sort out the i686 code before I start breaking the execution where I might have days of debugging to take on.

So, that leaves me with an easy decision to build out the same x86 stuff.

---

OK, so I have the kernel code reorganized -- think broad brush strokes.  I did find several things I want to clean up and I created several Redmines to go with them.  I think the next step here though is going to be to make a copy of the existing kernel module stuff and remove it from this project.  I will need to be able to make sure I can test properly when I do this, but it will be the only way I can make sure the kernel works properly at the moment.

I will do this and then test both architectures.

Everything still looks clean, so I am going to commit these changes.

---



