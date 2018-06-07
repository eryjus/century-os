***loader Module***

The loader module's purpose is the perform the early initialization of the system and transfer control to the kernel.  This module will receive control from the multiboot second stage and will perform the early initialization to put the system into its native mode.  The following will be completed by the multiboot loader:

* Complete as much 1-time initialization as can reasonably be completed for the  kernel.  Device driver initialization will happen in the device drivers.

* The screen will be placed into graphics mode.  This mode can be consistent across all architectures and allow the actual OS driver take over the video responsibility.  The minimum supported video resolution will be 1024 X 768 X 16.

* The MMU will be initialized for the architecture.  However, there is no need to actually enable paging for the loader until the very end... perhaps right before the jump into the kernel.

* The loader will initialize the UART and have that available for optionally dumping debug information to that port.

* Memory Initialization is a bit more of a challenge.  Since one of the goals of the loader is 1-time initialization, it makes sense to complete a physical frame allocator initialization.  Brendan mentions that there is no need to map freed frames: http://wiki.osdev.org/Brendan%27s_Memory_Management_Guide#Physical_Memory_Management.  With this in mind, it makes sense to create a bitmap for the memory and pass that bitmap to the kernel.  Since I will need to perform some calculations anyway, I that's where I will start and report the size of the structures needed.  1 4096-byte frame is able to keep track of 32768 frames (or 128MB of memory).  This represents 0.003% overhead to store keep track of physical frames used.
  
The loader is inspired by the following wiki page (or at least as it was on 13-Apr-2017): http://wiki.osdev.org/Creating_a_64-bit_kernel_using_a_separate_loader


**rpi2b Architecture**

There is an open source multiboot project called rpi-boot.  This project reports to supply a multiboot-compliant second stage for ARM (and specifically for the RPi).  Now, this does not specifically mention RPi2 (which has a different hardware address space), but I'm willing to bet I can get it working.  The project is still active as of Jan-2017.  This project is at https://github.com/jncronin/rpi-boot


**Unicode Support**

Century will support Unicode?  It's silly not to.  However, the question is: to what level?  Here are some thoughts:

* UTF-8 is the default encoding.
* Certain portions (such as the loader) will only support code points 0 through 127 (ASCII).  This restriction is reasonable since the font used would be too big with support more than this.  For any code point >127, the code point will be ignored (not printed).  A '?' will be printed in place.
* When printing/displaying a code point, all representations will be converted into UTF-32 encoding.


**UEFI**

It makes sense to address UEFI versus this loader module.  I thought about it myself (right about the time I was getting into the physical memory manager) and wondered why I did not go that way.  The end result had a little to do with whether I would lose my code to-date.  Actually, while there appears to be several open source projects that might work, I was not sure the projects were mature enough to work reliably.  So, I have decided to let those mature a bit and let others sort out how to boot UEFI for both qemu-system-arm and the RPi2b hardware so that I can reap their benefits.
