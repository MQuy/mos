#!/bin/bash
set -Eeuo pipefail

if [ -e mos.bin ]
then
  rm mos.bin
fi
if [ -e mos.iso ]
then
  rm mos.iso
fi

cd kernel && make clean && make && cp kernel.bin ../mos.bin
cd ../

if grub-file --is-x86-multiboot mos.bin; then
  echo multiboot confirmed
else
  echo the file is not multiboot
fi

if [ "$2" == "iso" ]
then
  mkdir -p isodir/boot/grub
  cp mos.bin isodir/boot/mos.bin && rm mos.bin
  cp grub.cfg isodir/boot/grub/grub.cfg
  grub-mkrescue -o mos.iso isodir
else
  # https://wiki.osdev.org/GRUB#HDD_Image_Instructions_for_OS_X_users
  dd if=/dev/zero of=mos.img count=163840 bs=512
  (
  echo y
  echo disk
  echo n
  echo edit 1
  echo 0B
  echo n
  echo 2047
  echo
  echo write
  echo quit
  ) | fdisk -e mos.img
  dd if=mos.img of=mbr.img bs=512 count=2047
  dd if=mos.img of=fs.img bs=512 skip=2047
  VOLUME_NAME=mos
  DISK_NAME="$(hdiutil attach -nomount fs.img)"
  $(brew --prefix e2fsprogs)/sbin/mkfs.ext2 $DISK_NAME
  hdiutil detach $DISK_NAME
  cat mbr.img fs.img > mos.img
  rm mbr.img fs.img
  hdiutil attach mos.img -mountpoint /Volumes/$VOLUME_NAME
  /usr/local/sbin/i386-elf-grub-install --modules="part_msdos biosdisk ext2 multiboot configfile" --root-directory="/Volumes/${VOLUME_NAME}" mos.img
  cp grub.cfg "/Volumes/${VOLUME_NAME}/boot/grub/grub.cfg"
  cp mos.bin "/Volumes/${VOLUME_NAME}/boot/mos.bin"
  cp sample.txt "/Volumes/${VOLUME_NAME}/sample.txt"
  hdiutil detach $DISK_NAME
fi

if [ "$1" == "bochs" ]
then
  if [ "$2" == "iso" ]
  then
    bochs -f .bochsrc.cdrom -q -rc .debugrc
  else
    bochs -f .bochsrc.disk -q -rc .debugrc
  fi
else
  if [ "$2" == "iso" ]
  then
    qemu-system-i386 -s -S -boot c -cdrom mos.iso -hda hdd.img \
      -chardev stdio,id=char0,logfile=logs/uart1.log \
      -serial chardev:char0 -serial file:logs/uart2.log -serial file:logs/uart3.log -serial file:logs/uart4.log \
      -rtc driftfix=slew
  else
    qemu-system-i386 -s -drive format=raw,file=mos.img,index=0,media=disk -d guest_errors,int
  fi
fi
