## OS Notes

### Boot sequence

- a computer is turned on, it runs through a series of diagnostics (POST - Power On Self Test).
- BIOS looks for a bootable devices which contains byte sequence 0x55 0xAA at offset 510 511 and loads (usually 1st sector) into memory at 0x0000:0x7c00 (segment 0). CPU is currently in Real Mode.
- assuming booting from a hard drive, we have only 446 bytes available
  - if we execute the kernel directly -> it is called Single Stage Bootloader (it is nearly impossible to do since a lot of things need to be done like setting up GDT, IDT, loading/jumping 32bit kernel ...).
  - consists of a single 512 byte bootloader above, however, it only loads and executes another loader Second Stage Bootloader is used to setup/load 32bit kernel.

**Reference**

- [Wiki boot sequence](https://en.wikipedia.org/wiki/Booting#Boot_sequence)
- [Wiki MBR layout](https://en.wikipedia.org/wiki/Master_boot_record#Sector_layout)
- [OS Development - Bootloader](http://www.brokenthorn.com/Resources/OSDev5.html)

### Linux scheduler

- Linux uses a Completely Fair Scheduling (CFS) algorithm.
- each thread is allocated timeslice to run and is accumulated `runtime` when running.
- a runqueue (red black tree), is for each core, sorts threads by `vruntime` (`runtime/weight`, weight is affected by ["nice"](https://man7.org/linux/man-pages/man1/nice.1.html)) , Linux picks a thread with smallest `vruntime`.
  ![runqueue](https://i.imgur.com/nfh5njM.png)
- load balancing threads between runqueues based on "load" (combination between weight and average CPU utilization). It is expensive, therefore, running periodically or when there is an idle core.
  - migrating threads between cores could cause cache locality and NUMA issues, therefore, the load balancer uses a hierarchical strategy.
  - cores are logically organized in a hierarchy. At the bottom level is single core, cores are a group in higher levels depends on how they share physical resources like cache, NUMA ...
  - a core (either idle or first of scheduling domain) starts this algorithm at second-to-lowest level and balances the load one level below. For example, it begins at pair of cores and balances the load between two individual cores. Then, it will move to the next level (NUMA) with 4 scheduling groups and balance the load between pair of cores (not between individual cores of NUMA level). The core will find the busiest scheduling group other than its own and steal tasks from the busiest core in that group.

**Reference**

- [Linux scheduler: a Decade of Wasted Cores](https://blog.acolyer.org/2016/04/26/the-linux-scheduler-a-decade-of-wasted-cores/)
