## mOS

mOS is the hobby operating system which is developed from scratch

[![](http://i3.ytimg.com/vi/-I3gCIqPkuU/maxresdefault.jpg)](https://www.youtube.com/watch?v=-I3gCIqPkuU "mOS demo")

### Work-in-process features

- [x] Filesystem
- [x] Program loading
- [x] UI (window server <-> userspace apps)
- [ ] Terminal
- [ ] Networking
- [ ] Sound
- [ ] POSIX
- [ ] Port GCC (the GNU Compiler Collection)
- [ ] Symmetric multiprocessing

🍀 Optional features

- [ ] Setup 2-level paging in boot.asm
- [ ] Dynamic linker

### Get started

```
$ brew install qemu nasm gdb i386-elf-gcc i386-elf-grub bochs e2fsprogs xorriso
$ cd src && ./build.sh
```

✍🏻If you get this error `hdiutil: attach failed - no mountable file systems`, installing might help [extFS for MAC](https://www.paragon-software.com/home/extfs-mac/)

### References

#### Memory management

- [Virtual memory](https://www.youtube.com/watch?v=qcBIvnQt0Bw)
- [How malloc works](https://forum.osdev.org/viewtopic.php?p=66669&sid=6491dc94867786304d824e07844575c4#p66669)
- [Recursive page directory](http://www.rohitab.com/discuss/topic/31139-tutorial-paging-memory-mapping-with-a-recursive-page-directory/)

#### Device

- [FAT12 Overview](http://www.disc.ua.es/~gil/FAT12Description.pdf)

#### Tutorials

- http://www.brokenthorn.com/Resources
- http://www.jamesmolloy.co.uk/tutorial_html/
- https://wiki.osdev.org

#### Ebooks

- [Understanding the Linux Kernel, 3rd Edition by Daniel P. Bovet; Marco Cesati](https://learning.oreilly.com/library/view/understanding-the-linux/0596005652/)
