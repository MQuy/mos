## mOS

![license MIT](https://img.shields.io/badge/license-MIT-blue>)
[![By Vietnamese](https://raw.githubusercontent.com/webuild-community/badge/master/svg/by.svg)](https://webuild.community)

mOS is the unix-like operating system developed from scratch and aims to POSIX compliant.

[![](https://i.imgur.com/DZB1mn9.png)](https://www.youtube.com/watch?v=y3h7E1nkaUs "mOS")

### Work-in-process features

- [x] Filesystem
- [x] Program loading
- [x] UI (X11)
- [x] Log
- [x] Networking
- [x] Signal
- [x] Terminal
- [x] mOS toolchain
- [x] Port figlet
- [ ] Unit testing
- [ ] Port GNU Bash
- [ ] Dynamic linker
- [ ] Port GCC (the GNU Compiler Collection)
- [ ] Browser
- [ ] Sound
- [ ] Symmetric multiprocessing

🍀 Optional features

- [ ] POSIX compliant
- [ ] Setup 2-level paging in boot.asm

### Get started

**MacOS**

- install packages

  ```
  $ brew install qemu nasm gdb i386-elf-gcc i386-elf-grub bochs e2fsprogs xorriso
  ```

- open your bash config and add lines below. Depends on your bash, config file might be different. I use `ohmyzsh`, so it is `.zshrc`

  ```
  # .zshrc
  alias grub-file=i386-elf-grub-file
  alias grub-mkrescue=i386-elf-grub-mkrescue
  ```

- run emulator

  ```
  $ cd src && ./create_image.sh && ./build.sh qemu iso
  ```

✍🏻 If you get this error `hdiutil: attach failed - no mountable file systems`, installing [extFS for MAC](https://www.paragon-software.com/home/extfs-mac/) might help

**Ubuntu**

- install gcc cross compilier via https://wiki.osdev.org/GCC_Cross-Compiler#The_Build
- install mos toolchain

  ```
  $ cd src/toolchain
  $ ./build
  ```

- run emulator
  ```
  $ cd src && ./create_image.sh && ./build.sh qemu iso
  ```

**Debuging**

in `build.sh`, adding `-s -S` right after `qemu` to switch to debug mode. Currently, I use vscode + [native debuge](https://marketplace.visualstudio.com/items?itemName=webfreak.debug) -> click Run -> choose "Attach to QEMU"

**Monitoring**

by default mOS log outputs to terminal. If you want to monitor via file, doing following steps

```
# src/build.sh#L71
-serial stdio
↓
-serial file:logs/uart1.log
```

```
$ tail -f serial.log | while read line ; do echo $line ; done
```

✍🏻 Using `tail` in pipe way to color the output (like above) causes a delay -> have to manually saving in ide to get latest changes

### References

#### Tutorials

- http://www.brokenthorn.com/Resources
- http://www.jamesmolloy.co.uk/tutorial_html/
- https://wiki.osdev.org

#### Ebooks

- [Understanding the Linux Kernel, 3rd Edition by Daniel P. Bovet; Marco Cesati](https://learning.oreilly.com/library/view/understanding-the-linux/0596005652/)
