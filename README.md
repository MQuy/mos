## mOS

![license MIT](https://img.shields.io/badge/license-MIT-blue>)
[![By Vietnamese](https://raw.githubusercontent.com/webuild-community/badge/master/svg/by.svg)](https://webuild.community)

mOS is the hobby operating system which is developed from scratch

[![](http://i3.ytimg.com/vi/-I3gCIqPkuU/maxresdefault.jpg)](https://www.youtube.com/watch?v=-I3gCIqPkuU "mOS demo")

### Work-in-process features

- [x] Filesystem
- [x] Program loading
- [x] UI (X11)
- [x] Log
- [x] Networking
- [x] Signal
- [ ] Terminal
- [ ] POSIX compliant
- [ ] Port GCC (the GNU Compiler Collection)
- [ ] Sound
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

#### Tutorials

- http://www.brokenthorn.com/Resources
- http://www.jamesmolloy.co.uk/tutorial_html/
- https://wiki.osdev.org

#### Ebooks

- [Understanding the Linux Kernel, 3rd Edition by Daniel P. Bovet; Marco Cesati](https://learning.oreilly.com/library/view/understanding-the-linux/0596005652/)
