rm *.bin
rm *.iso

cd kernel && make clean && make && cp kernel.bin ../mos.bin && cd ../

if i386-elf-grub-file --is-x86-multiboot mos.bin; then
  echo multiboot confirmed
else
  echo the file is not multiboot
fi

if [ "$2" == "iso" ]
then
  mkdir -p isodir/boot/grub
  cp mos.bin isodir/boot/mos.bin && rm mos.bin
  cp grub.cfg isodir/boot/grub/grub.cfg
  i386-elf-grub-mkrescue -o mos.iso isodir
else
  # https://wiki.osdev.org/GRUB#HDD_Image_Instructions_for_OS_X_users
  dd if=/dev/zero of=mos.img count=163840 bs=512
  (
  echo y
  echo mos
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
  mkfs.ext2 $DISK_NAME
  hdiutil detach $DISK_NAME
  cat mbr.img fs.img > mos.img
  rm mbr.img fs.img
  hdiutil attach mos.img -mountpoint /Volumes/$VOLUME_NAME
  /usr/local/sbin/i386-elf-grub-install --modules="part_msdos biosdisk fat multiboot configfile" --root-directory="/Volumes/${VOLUME_NAME}" mos.img
  cp grub.cfg "/Volumes/${VOLUME_NAME}/boot/grub/grub.cfg"
  cp mos.bin "/Volumes/${VOLUME_NAME}/boot/mos.bin"
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
    qemu-system-i386 -boot c -cdrom mos.iso -hda hdd.img
  else
    qemu-system-i386 -s -drive format=raw,file=mos.img,index=0,media=disk -d guest_errors,int
  fi
fi