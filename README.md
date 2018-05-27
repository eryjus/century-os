***The Century OS***

Century OS is a hobby Operating System.  Its objective is to run a *relatively* "common" kernel across multiple architectures.  Currently, I am targeting i686, x86_64, and rpi2b.  I am developing for these architectures these in parallel.

        Copyright (c)  2017 -- Adam Clark (see LICENSE for the specifics)


I chose "Century" as a name because it described me and my project in 2 ways:
1) I am a road bicyclist.  In the US, the bicycling version of a marathon is called a century -- 100 miles at a time.  No, I have no aspirations to ride in the TDF.
2) I expect this OS to take at least 100 years to complete.

Since this OS will be available for others to review and possibly glean nuggets of information.  I do not represent my code as the ideal solution for anything.  If you have feedback for me, please feel free to e-mail me at hobbyos@eryjus.com.  Take a look at [my journal](./JOURNAL.md) if you haven't already.

As a result putting my own development stages in display in a public repository, I plan to provide smaller commits to my public repository.  The hope behind the large number of smaller commits is that other readers can observe the evolution of development.  OS development is a large project and many do not know what order to take tasks in.  The public repository documents my own development path and changes in direction.


***Why?***

I admit, I have had several partial starts, some on my own and some with existing OSes.  This one is no different and you may be familiar with some of them.  So, why this version of the OS and why now?  That's a fair question.

First, I am by no means an expert.  Each system I have started has had specific objectives in mind.  My first incarnation was a basic learning project.  It lacked focus and direction.  However, I was able to get multiple processes running properly.  I also managed to break it and fix it several times.  It was 32-bit.

Then, I embarked on a 64-bit assembly system.  I took it on in assembly because I was not able to figure out how to co-mingle 32-bit and 64-bit code in C.  Well, if you can't fix it, feature it!  So, I took it on in 100% assembly.  Again, I was able to get multiple processes running.

Then I looked at xv6, a teaching kernel which I refer back to even today.  I took that as a starting point since it had a working filesystem and user-mode applications.  I learn better when I have a working system to look review and dig into.  I was going to slowly replace portions with things of my own invention.

Well, after several years away, I have decided to pick up the OS again.  I had acquired a Raspberry Pi 2 that I wanted to start playing with and never did.  I got tired of it sitting on my desk staring me down.  So, I started over with this incarnation of Century.

The goals here are pretty simple:
1. Document the progress of this OS for those that learn through other's mistakes.  Use GitHub to keep track of my history.
2. Establish a common kernel base source that can be run on multiple architectures, with architecture-specific code handled properly.
3. Write this kernel as a micro-kernel.
4. Write the GUI desktop manager myself, but port as many other components as possible.  Therefore, take time to make the porting process as easy as possible.

But, again, I am no expert.  You might just want to follow this project just to watch the train wreck unfolding!  ;)


***The Build System***

`make` is out!  Well, mostly.

I have started to convert to `tup` for the bulk of the build system.

So, I have had a rather large shift in direction with my build system.  I have tarted to use `tup` to execute the core of my build, and then use make to do more scripting things.  The reason for the shift is simple: `tup` is FAR easier to maintain than make files.  One of the key reasons for this is that the Tupfile is located in the target directory (where makefiles are typically located in the source directories).  Therefore, if there is something that is needed to satisfy a dependency, `tup` only needs to look in the directory where that dependency should be and if it is not there, read the Tupfile in that directory on how to create that object.

However, there is also give and take with `tup`.  Since I do not want to clutter up my sysroot folders with a bunch of Tupfiles that would end up on the .iso or .img, I have implemented the parts that would copy these files into the sysroot folders in the only Makefile.  The result is MUCH simpler.  I have a default 'all' target that simply calls `tup` to refresh the build.  Any of the other specialized targets (.iso, or running QEMU) depend on the 'all' target and then run the steps needed to complete the script.

The only key function I am giving up is the ability to build an architecture or a module independently.  For the moment, I think I can live with that by creating stub functions when needed.

You can find `tup` at: http://gittup.org/gittup/.
