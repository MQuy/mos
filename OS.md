## OS Notes

### Boot sequence

- a computer is turned on, it runs through a series of diagnostics (POST - Power On Self Test).
- BIOS looks for a bootable devices which contains byte sequence 0x55 0xAA at offset 510 511 and loads (usually 1st sector) into memory at 0x0000:0x7c00 (segment 0). CPU is currently in Real Mode.
- assuming booting from a hard drive, we have only 446 bytes available
  - if we executes the kernel directly -> it is called Single Stage Bootloader (it is nearly impossible to do since a lot of things need to be done like setuping GDT, IDT, loading/jumping 32bit kernel ...)
  - consists a single 512 byte bootloader above, however, it only loads and executes another loader Second Stage Bootloader which is used to setup/load 32bit kernel

**Reference**

- [Wiki boot sequence](https://en.wikipedia.org/wiki/Booting#Boot_sequence)
- [Wiki MBR layout](https://en.wikipedia.org/wiki/Master_boot_record#Sector_layout)
- [OS Development - Bootloader](http://www.brokenthorn.com/Resources/OSDev5.html)
