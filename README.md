## mOS

mOS is the hobby operating system which is developed from scratch

[![](http://i3.ytimg.com/vi/-I3gCIqPkuU/maxresdefault.jpg)](https://www.youtube.com/watch?v=-I3gCIqPkuU "mOS demo")

### Work-in-process features

- [x] Filesystem
- [x] Program loading
- [x] UI (window server <-> userspace apps)
- [ ] Terminal
- [x] Networking
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
$ cd src && ./create_image.sh && ./build.sh qemu iso
```

✍🏻 If you get this error `hdiutil: attach failed - no mountable file systems`, installing might help [extFS for MAC](https://www.paragon-software.com/home/extfs-mac/)

**Debuging**

in `build.sh`, adding `-s -S` right after `qemu` to switch to debug mode. Currently, I use vscode + [native debuge](https://marketplace.visualstudio.com/items?itemName=webfreak.debug) -> click Run -> choose "Attach to QEMU"

**Monitoring**

```
tail -f serial.log | while read line ; do echo $line ; done
```

✍🏻 Using `tail` in pipe way to color the output (like above) causes a delay -> have to manually saving in ide to get latest changes

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
